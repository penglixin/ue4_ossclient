
#include "Assetoperation/Assetdownloader.h"
#include "Misc/FileHelper.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"


#if WITH_EDITOR
#pragma optimize( "", off)
#endif

UAssetdownloader* UAssetdownloader::ThisPtr = nullptr;

UAssetdownloader* UAssetdownloader::Get_Assetdownloader_Instance()
{
	if (!ThisPtr)
	{
		UObject* Outer = (UObject*)GetTransientPackage();
		ThisPtr = NewObject<UAssetdownloader>(Outer);

		ThisPtr->AddToRoot();
	}
	return ThisPtr;
}

void UAssetdownloader::DestroyInstance()
{
	if (ThisPtr)
	{
		ThisPtr->RemoveFromRoot();

		ThisPtr = nullptr;
	}
}

void UAssetdownloader::check_assets(const FString& accountID)
{
	if (accountID.IsEmpty()) return;

	refreshuseraccount(accountID);

	//一开始先拿到本地已经下载好了的文件信息
	m_localdowncomplete.reset();
	FString contentStr;
	FFileHelper::LoadFileToString(contentStr, *getlocaldowncompletedfile());
	dataconversion::Read(contentStr, m_localdowncomplete);

	const FString oss_versionlist_link = get_osslink() + get_userAccount() / get_userAccount() + m_versionlistName;
	downloadVersionInfo(oss_versionlist_link);
}

void UAssetdownloader::oneAssetdownloadcomplete(const FOneAssetVersionInfo& asset)
{
	m_localdowncomplete.transfercompleteassets.Add(asset);

	// 先暂时每次都写入一次，之后再加一个回调，只写入一次
	FString json;
	dataconversion::Write(m_localdowncomplete, json);
	FFileHelper::SaveStringToFile(json, *getlocaldowncompletedfile());

	//所有待下载的资源都下完了，再去更新资源信息文件 getSavefilename()
	bool downloadcompleted = true;
	for (const auto& t_download : m_downAssets)
	{
		if (!m_localdowncomplete.transfercompleteassets.Contains(t_download))
		{
			downloadcompleted = false;
			break;
		}
	}

	oneCompleted.Broadcast(asset.uniqueNumber);
	simple_oneCompleted.ExecuteIfBound(asset.uniqueNumber);
	if(downloadcompleted)
	{
		json.Empty();
		dataconversion::Write(m_latestversion, json);
		FFileHelper::SaveStringToFile(json, *getAssetversionFile());

		//删除本地记录已下载完成的临时文件
		IFileManager::Get().Delete(*getlocaldowncompletedfile());

		allCompleted.Broadcast();
		simple_allCompleted.ExecuteIfBound();
	}
}

void UAssetdownloader::createtasks()
{
	m_Tasks.Empty();
	if (m_downAssets.Num())
	{
		for (const auto& t_asset : m_downAssets)
		{
			TSharedPtr<FdownloadRequset> task = MakeShareable(new FdownloadRequset());
			task->createtask(t_asset);
			m_Tasks.Add(task);
		}
	}
}

void UAssetdownloader::executetasks()
{
	//每次执行一个任务，一个任务完成后再继续下一个
	for (const auto t_task : m_Tasks)
	{
		if (requestStatus::ready == t_task->get_status() || requestStatus::retry == t_task->get_status())
		{
			t_task->begindownload();
			break;
		}
	}
}

void UAssetdownloader::downloadVersionInfo(const FString& url)
{
	if (url.IsEmpty()) return;

	// Create the Http request and add to pending request list
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAssetdownloader::Handle_version_downloadcomplete);
	HttpRequest->OnRequestProgress().BindUObject(this, &UAssetdownloader::Handle_version_downloadprogress);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void UAssetdownloader::Handle_version_downloadcomplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (bSucceeded && HttpRequest.IsValid() && HttpResponse.IsValid() && EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()) && HttpResponse->GetContentLength() > 0)
	{
		if (requestType::versionlist == convertRequest(HttpResponse->GetURL()))
		{
			download_versionlist_complete(HttpResponse);
		}
		else if (requestType::version == convertRequest(HttpResponse->GetURL()))
		{
			download_userversion_complete(HttpResponse);
		}
	}
	else
	{
		reuestversionFailed.Broadcast();
		simple_reuestversionFailed.ExecuteIfBound();
	}
}

void UAssetdownloader::Handle_version_downloadprogress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesRecv)
{
	if (Request.IsValid())
	{
		if(FPaths::GetExtension(Request->GetURL(), true).Equals(m_pakExtesion))
		{
			int32 contenglen = Request->GetResponse()->GetContentLength();
			const float ratio = ((float)BytesRecv / (float)contenglen) * 100.f;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT(" %s , ratio : [%.2f%%] ....."), *(FPaths::GetCleanFilename(Request->GetURL())), ratio));
		}
	}
}

void UAssetdownloader::download_versionlist_complete(FHttpResponsePtr HttpResponse)
{
	m_sv.reset();
	m_downAssets.Reset();
	dataconversion::Read(HttpResponse->GetContentAsString(), m_sv);

	//校验资源版本
	FString contentStr;
	FFileHelper::LoadFileToString(contentStr, *getAssetversionFile());
	FAssetVersionInfo local_assetversion;
	dataconversion::Read(contentStr, local_assetversion);

	if (m_sv.latestversion == local_assetversion.versionName && m_sv.md5code == local_assetversion.md5code) return;

	//把中间的版本都下载下来
	TArray<TTuple<FString, FString>> historyversions = m_sv.historyversions;
	historyversions.Add(MakeTuple(m_sv.latestversion, m_sv.md5code));
	
	const bool bcontainlocal = historyversions.ContainsByPredicate([local_assetversion](const TTuple<FString, FString>& A) {
		return local_assetversion.versionName == A.Get<0>() && local_assetversion.md5code == A.Get<1>();
		});

	bool beginAdd = false;
	for (const auto& t_history : historyversions)
	{
		if (!bcontainlocal)
		{
			const FString oss_version_link = get_osslink() + get_userAccount() / t_history.Get<0>() / t_history.Get<0>() + m_userversionName;
			downloadVersionInfo(oss_version_link);
		}
		else
		{
			if(beginAdd)
			{
				const FString oss_version_link = get_osslink() + get_userAccount() / t_history.Get<0>() / t_history.Get<0>() + m_userversionName;
				downloadVersionInfo(oss_version_link);
			}
			else
			{
				beginAdd = (local_assetversion.versionName == t_history.Get<0>() && local_assetversion.md5code == t_history.Get<1>());
			}
		}
	}
}

void UAssetdownloader::download_userversion_complete(FHttpResponsePtr HttpResponse)
{
	FAssetVersionInfo assetversion;
	dataconversion::Read(HttpResponse->GetContentAsString(), assetversion);

	for (const auto& tempasset : assetversion.assets)
	{
		bool alreadydownload = m_localdowncomplete.transfercompleteassets.Contains(tempasset);

		if (!alreadydownload) m_downAssets.Add(tempasset);
	}

	if (assetversion.versionName == m_sv.latestversion)  //把最新版本存在本地
	{
		//临时存，所有下载完再写入本地
		m_latestversion = assetversion;
		/** 保存最新的资源版本文件到本地 */
		//FFileHelper::SaveArrayToFile(HttpResponse->GetContent(), *getAssetversionFile());

		/** 版本文件对比完，开始下载资源 */
		versionfilecomparecompleted();
	}
}

void UAssetdownloader::versionfilecomparecompleted()
{
	createtasks();

	executetasks();
}

FSimpleOneAssetdownloadProgress& UAssetdownloader::OnAssetProgress()
{
	return simple_downloading;
}

FSimpleOneAssetCompleted& UAssetdownloader::OnAssetDownloadCompleted()
{
	return simple_oneCompleted;
}

FSimpleAllAssetsCompleted& UAssetdownloader::OnAllAssetsCompleted()
{
	return simple_allCompleted;
}

FSimpleAssetDownloadFailed& UAssetdownloader::OnAssetFailed()
{
	return simple_oneFailed;
}

FSimpleRequestVersionInfoFailed& UAssetdownloader::OnRequestVersionFailed()
{
	return simple_reuestversionFailed;
}

#if WITH_EDITOR
#pragma optimize( "", on)
#endif