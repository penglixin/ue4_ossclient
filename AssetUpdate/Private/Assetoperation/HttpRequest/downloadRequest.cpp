#include "Assetoperation/HttpRequest/downloadRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Assetoperation/Assetdownloader.h"



#if WITH_EDITOR
#pragma optimize( "", off)
#endif

FdownloadRequset::FdownloadRequset()
{

}

FdownloadRequset::~FdownloadRequset()
{

}

void FdownloadRequset::createtask(const FOneAssetVersionInfo& in_info)
{
	m_status = requestStatus::ready;
	m_assetinfo = in_info;
	breakTask(m_assetinfo);
}

void FdownloadRequset::begindownload()
{
	m_retrycount++;
	m_status = requestStatus::downloading;
	executesubtasks();
}

FString FdownloadRequset::getSavefilename(const FString& downlink)
{
	const FString fullfilename = FPaths::ProjectDir() + TEXT("MiaoXiang/Cache/Patch") / FPaths::GetCleanFilename(downlink);
	return fullfilename;
}

void FdownloadRequset::breakTask(const FOneAssetVersionInfo& asset)
{
	m_subTasks.Empty();

	IPlatformFile& PlatFileModule = FPlatformFileManager::Get().GetPlatformFile();

	//判断有没有缓存
	TArray<int32> taskcache;
	if (PlatFileModule.DirectoryExists(*(m_cachefolder / asset.md5code)))
	{
		TArray<FString> CacheFiles;
		PlatFileModule.FindFiles(CacheFiles, *(m_cachefolder / asset.md5code), TEXT("hcf"));
		for (int32 i = 0; i < CacheFiles.Num(); ++i)
		{
			FString LStr, RStr;
			CacheFiles[i].Split(TEXT("_"), &LStr, &RStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			FString LStr1, RStr1;
			RStr.Split(TEXT("."), &LStr1, &RStr1, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			taskcache.Add(FCString::Atoi(*LStr1));
		}
	}

	//拆分任务
	int32 startpos = 0;
	int32 taskID = 0;

	while (startpos < asset.filesize)
	{
		TSharedPtr<FSubRequest> subTask = MakeShareable(new FSubRequest());
		int32 temp_size = 0;
		FString temp_range;

		subTask->set_taskID(taskID);
		subTask->set_requesturl(asset.downloadlink);
		subTask->set_md5(asset.md5code);
		subTask->set_startpos(startpos);
		if (startpos + RESUME_DOWNLOAD_SIZE < asset.filesize)
		{
			temp_size = RESUME_DOWNLOAD_SIZE;
			temp_range = FString::Printf(TEXT("bytes=%d-%d"), startpos, startpos + temp_size - 1);
		}
		else
		{
			temp_size = asset.filesize - startpos;
			temp_range = FString::Printf(TEXT("bytes=%d-"), startpos);
		}
		subTask->set_size(temp_size);
		subTask->set_range(temp_range);

		if (taskcache.Contains(taskID))
		{
			subTask->set_status(requestStatus::succeed);
			subTask->loadcacheData();
			m_hasdownloadsie += subTask->get_rawdata().Num();
		}

		m_subTasks.Add(subTask);

		startpos += temp_size;
		taskID++;
	}

	bool notfinished = m_subTasks.ContainsByPredicate([](const TSharedPtr<FSubRequest> A) {
		return requestStatus::succeed != A->get_status();
		});
	if (!notfinished)
	{
		//所有子任务下载完了，把所有数组组成完整的文件
		onsucceed();
	}
}

void FdownloadRequset::executesubtasks()
{
	bool hastaskexecuted = false;  //没有可执行的任务了，但是文件没下完，报错
	for (const auto t_task : m_subTasks)
	{
		if (requestStatus::ready == t_task->get_status() || requestStatus::retry == t_task->get_status()) //子任务不会有重试次数，失败了就直接重新调用downloadRequest,右端点续传，不用担心重复请求
		{
			hastaskexecuted = true;
			TSharedRef<IHttpRequest> subRequest = t_task->GetHttpRequest();
			subRequest->OnProcessRequestComplete() = FHttpRequestCompleteDelegate::CreateRaw(this, &FdownloadRequset::Handle_task_complete, t_task->get_taskID());
			subRequest->OnRequestProgress() = FHttpRequestProgressDelegate::CreateRaw(this, &FdownloadRequset::Handle_task_progress, t_task->get_taskID());
			subRequest->ProcessRequest();
			break;
		}
	}

	if (!hastaskexecuted && m_status != requestStatus::succeed)
	{
		onfailed(FString::Printf(TEXT("has some subtask failed , filename [%s]"), *(FPaths::GetCleanFilename(m_assetinfo.downloadlink))), INDEX_NONE);
	}
}

void FdownloadRequset::onsucceed()
{
	const FString localsavefile = getSavefilename(m_assetinfo.downloadlink);
	FArchive* Wirter = IFileManager::Get().CreateFileWriter(*localsavefile);
	if (Wirter)
	{
		for (const auto t_task : m_subTasks)
		{
			Wirter->Serialize(t_task->get_rawdata().GetData(), t_task->get_rawdata().Num());
		}
		Wirter->Close();
	}
	m_status = requestStatus::succeed;

	//清空缓存
	FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*(m_cachefolder / m_assetinfo.md5code));

	//通知 UAssetdownloader
	UAssetdownloader::Get_Assetdownloader_Instance()->oneAssetdownloadcomplete(m_assetinfo);

	//继续下一个任务
	UAssetdownloader::Get_Assetdownloader_Instance()->executetasks();
}

void FdownloadRequset::onfailed(const FString& errorMesg, const int32& taskID)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, errorMesg);
	if (m_retrycount < MAX_RETRY_COUNT)
	{
		m_retrycount++;
		m_status = requestStatus::retry;

	}
	else
	{
		m_status = requestStatus::failed;

		UAssetdownloader::Get_Assetdownloader_Instance()->oneFailed.Broadcast(m_assetinfo.uniqueNumber);
		UAssetdownloader::Get_Assetdownloader_Instance()->OnAssetFailed().ExecuteIfBound(m_assetinfo.uniqueNumber);
	}
	
	//继续下一个任务
	UAssetdownloader::Get_Assetdownloader_Instance()->executetasks();
}

void FdownloadRequset::Handle_task_complete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 taskID)
{
	if (bSucceeded && HttpResponse.IsValid() && EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
	{
		TSharedPtr<FSubRequest> task = m_subTasks[taskID];
		if (task->get_size() == HttpResponse->GetContent().Num())
		{
			task->set_rawdata(HttpResponse->GetContent());
			task->savecacheData();
			task->set_status(requestStatus::succeed);
			m_hasdownloadsie += task->get_size();

			if (m_hasdownloadsie == m_assetinfo.filesize)
			{
				onsucceed();
			}
			else
			{
				//继续下一个子任务
				executesubtasks();
			}
		}
		else
		{
			onfailed(FString::Printf(TEXT(", subtask size error , taskID [%d]"), taskID), taskID);
		}
	}
	else
	{
		onfailed(FString::Printf(TEXT(", rquest error , taskID [%d]"), taskID), taskID);
	}
}

void FdownloadRequset::Handle_task_progress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesRecv, int32 taskID)
{
	if (Request.IsValid())
	{
		const int32 contenglen = Request->GetResponse()->GetContentLength();
		const float ratio = ((float)BytesRecv / (float)contenglen) * 100.f;
		const float totalratio = ((float)(BytesRecv + m_hasdownloadsie) / (float)m_assetinfo.filesize) * 100.f;
		//GEngine->AddOnScreenDebugMessage(-1, 1000.f, FColor::Green, FString::Printf(TEXT(" %s , taskID : [%d / %d] , task ratio : [%.2f%%] , total ratio : [%.2f%%]"), *m_assetinfo.uniqueNumber, taskID, m_subTasks.Num() - 1, ratio, totalratio));

		UAssetdownloader::Get_Assetdownloader_Instance()->downloading.Broadcast(m_assetinfo.uniqueNumber, ratio, totalratio, taskID, m_subTasks.Num() - 1);
		UAssetdownloader::Get_Assetdownloader_Instance()->OnAssetProgress().ExecuteIfBound(m_assetinfo.uniqueNumber, ratio, totalratio, taskID, m_subTasks.Num() - 1);
	}
}



#if WITH_EDITOR
#pragma optimize( "", on)
#endif