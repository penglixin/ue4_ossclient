
#include "uploader.h"
#include "OSSManager.h"
#include "Misc/FileHelper.h"


#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", off)
#endif


DEFINE_LOG_CATEGORY_STATIC(UploadOSSLog, Log, All);

uploader* uploader::m_uploader = nullptr;

FCriticalSection uploadAssetMutex;
void oss_progress_callback_local(int64 ConsumedBytes, int64 TotalBytes)
{
	FScopeLock ScopeLock(&uploadAssetMutex);
	/*auto EventRef = FFunctionGraphTask::CreateAndDispatchWhenReady([ConsumedBytes, TotalBytes]()
		{
			AsyncPutObject_Delegate.ExecuteIfBound(ConsumedBytes, TotalBytes);
		}, TStatId(), nullptr, ENamedThreads::GameThread);*/
		/*FTaskGraphInterface::Get().WaitUntilTaskCompletes(EventRef);*/
	uploader::get()->assets_upload_progress(ConsumedBytes, TotalBytes);
}

uploader::uploader()
{

}

uploader::~uploader()
{

}

uploader* uploader::get()
{
	if (!m_uploader)
	{
		m_uploader = new uploader();
	}
	return m_uploader;
}

void uploader::destroy()
{
	if (m_uploader)
	{
		delete m_uploader;
	}
	m_uploader = nullptr;
}

void uploader::commit(const FString& in_userAccount, const FString& in_bucketName, const FString& in_osslink, const FString& in_filepath, const FString& in_folderpath)
{
	m_userAccount = in_userAccount;
	m_bucketName  = in_bucketName;
	m_osslink     = in_osslink;
	m_filepath    = in_filepath;
	m_folderpath  = in_folderpath;
	m_status = uploadstatus::beginupload;
	log_tips = "begin";

	const TArray<FString>& uploadfiles = get_alluploadAssets();

	if (uploadfiles.Num() > 0)
	{
		if (false == OSSManager::Get()->OnOneAssetUploadCompleted().IsBound())
			OSSManager::Get()->OnOneAssetUploadCompleted() = FOneAssetUploadCompleted::CreateRaw(this, &uploader::on_one_asset_upload_completed);

		initial_localinfo();
		m_allassets.versionName = m_sv.latestversion;
		m_allassets.md5code = m_sv.md5code;

		TArray<FString> t_needupload_fullpath;
		for (const auto& t_file : uploadfiles)
		{
			FString objName = m_userAccount / m_sv.latestversion / FPaths::GetCleanFilename(t_file);
			FOneAssetVersionInfo oneasset;
			constructAssetversioninfo(t_file, oneasset);
			oneasset.downloadlink = m_osslink + objName;
			m_allassets.assets.Add(oneasset);
			if (!m_localuploadcomplete.transfercompleteassets.Contains(oneasset))
			{
				m_needuploadAssets.Add(oneasset.filesize, oneasset);
				t_needupload_fullpath.Add(t_file);
			}
		}

		//上传资源
		for (const auto& t_file : t_needupload_fullpath)
		{
			m_status = uploadstatus::uploading;
			FString objName = m_userAccount / m_sv.latestversion / FPaths::GetCleanFilename(t_file);
			//SIMPLE_OSS.AsyncPutObject(GetDefault<UMXSettings>()->ossBucketName, t_file, objName, oss_progress_callback_local);
			OSSManager::Get()->AsyncResumableUploadObject(m_bucketName, objName, t_file, RESUME_UPLOAD_SIZE, oss_progress_callback_local);
		}
	}
}

TArray<FString> uploader::parasePath(const FString& in_str)
{
	TArray<FString> retval;
	in_str.ParseIntoArray(retval, TEXT(";"));
	return retval;
}

void uploader::initial_localinfo()
{
	m_needuploadAssets.Empty();
	m_succeedcount = 0;
	m_failedcount = 0;
	m_allassets.reset();
	m_localuploadcomplete.reset();
	FString contentStr;
	FFileHelper::LoadFileToString(contentStr, *getlocaluploadcompletedfile());
	dataconversion::Read(contentStr, m_localuploadcomplete);

	m_sv = get_newversion();
	m_succeedcount = m_localuploadcomplete.transfercompleteassets.Num();
}

TArray<FString> uploader::get_alluploadAssets()
{
	TArray<FString> uploadfiles;

	const TArray<FString> files = parasePath(m_filepath);
	for (const auto& t_file : files)
	{
		if (FPaths::FileExists(t_file) && FPaths::GetExtension(t_file, true).Equals(FString(".pak")))
			uploadfiles.AddUnique(t_file);
	}

	const TArray<FString> folders = parasePath(m_folderpath);
	for(const auto& t_folder : folders)
	{
		if (FPaths::DirectoryExists(t_folder))
		{
			TArray<FString> FoundFiles;
			IFileManager::Get().FindFilesRecursive(FoundFiles, *t_folder, TEXT("*.pak"), true, false);
			uploadfiles.Append(FoundFiles);
		}
	}
	return uploadfiles;
}

serverversion uploader::get_newversion()
{
	FString contextStr;
	const FString& objname = m_userAccount + TEXT("/") + m_userAccount + FString("_versionlist.json");
	if (OSSManager::Get()->GetObjectToMemory(m_bucketName, objname, contextStr))
	{
		serverversion sv;
		if (dataconversion::Read(contextStr, sv))
		{
			FString l, r;
			sv.latestversion.Split(TEXT("."), &l, &r, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			int32 currentnum = FCString::Atoi(*r);
			currentnum++;
			FString newversion = l + TEXT(".") + FString::FromInt(currentnum);

			TTuple<FString, FString> history = MakeTuple(sv.latestversion, sv.md5code);
			sv.historyversions.Add(history);
			sv.latestversion = newversion;
			sv.md5code = FGuid::NewGuid().ToString();
			return sv;
		}
	}
	serverversion sv;
	sv.latestversion = m_userAccount + FString("_v_1.0.0");
	sv.md5code = FGuid::NewGuid().ToString();
	return sv;
}

FString uploader::construct_md5(const FString& filefullpath)
{
	TArray<uint8> content;
	FFileHelper::LoadFileToArray(content, *filefullpath);
	return FMD5::HashBytes(content.GetData(), content.Num());
}

void uploader::constructAssetversioninfo(const FString& assetfile, FOneAssetVersionInfo& oneasset)
{
	oneasset.md5code = construct_md5(assetfile);
	oneasset.uniqueNumber = FPaths::GetCleanFilename(assetfile);
	oneasset.companycode = "non-companycode";

	FFileStatData filestatus = IFileManager::Get().GetStatData(*assetfile);
	oneasset.filesize = filestatus.FileSize;
}

FString uploader::getlocaluploadcompletedfile()
{
	FString userAccount = m_userAccount;
	const FString fullfilename = OSSManager::Get()->get_current_exe_path() + TEXT("\\") + userAccount + TEXT("\\") + FString::Printf(TEXT("%s_uploadcompleted.json"), *userAccount);
	return fullfilename;
}

void uploader::assets_upload_progress(const int64& ConsumedBytes, const int64& TotalBytes)
{
	if (m_needuploadAssets.Contains(TotalBytes))
	{
		FOneAssetVersionInfo* asset = m_needuploadAssets.Find(TotalBytes);
		float ratio = ((float)ConsumedBytes / (float)TotalBytes) * 100.f;

		UE_LOG(UploadOSSLog, Warning, TEXT("[%s] ratio : [%.2f%%] ,total file : [%d/%d]"), *(asset->uniqueNumber), ratio, m_succeedcount + 1, m_allassets.assets.Num());

		log_tips = FString::Printf(TEXT("[%s] ratio : [%.2f%%] ,total file : [%d/%d]"), *(asset->uniqueNumber), ratio, m_succeedcount + 1, m_allassets.assets.Num());
	}
}

void uploader::on_one_asset_upload_completed(const FString& filefullpath, const bool& bsucceed)
{
	if (!bsucceed)
	{
		m_failedcount++;
		UE_LOG(UploadOSSLog, Error, TEXT("[%s] upload failed."), *filefullpath);
		return;
	}

	UE_LOG(UploadOSSLog, Warning, TEXT("[%s] upload complete, total file : [%d/%d] ."), *filefullpath, m_succeedcount + 1, m_allassets.assets.Num());

	log_tips = FString::Printf(TEXT("[%s] upload complete, total file : [%d/%d] ."), *filefullpath, m_succeedcount + 1, m_allassets.assets.Num());

	const FString& md5 = construct_md5(filefullpath);

	bool bhasfound = false;
	FOneAssetVersionInfo found_result;
	for (const auto& t_asset : m_needuploadAssets)
	{
		if (t_asset.Value.md5code == md5)
		{
			bhasfound = true;
			found_result = t_asset.Value;
			break;
		}
	}

	if (!bhasfound)
	{
		m_failedcount++;
		UE_LOG(UploadOSSLog, Error, TEXT("[%s] construct_md5 error."), *filefullpath);
		return;
	}

	m_succeedcount++;
	m_localuploadcomplete.transfercompleteassets.Add(found_result);
	// 先暂时每次都写入一次，之后再加一个回调，只写入一次
	FString json;
	dataconversion::Write(m_localuploadcomplete, json);
	FFileHelper::SaveStringToFile(json, *getlocaluploadcompletedfile());

	if (m_succeedcount == m_allassets.assets.Num())
	{
		on_all_assets_upload_completed();
	}
	else if (m_succeedcount + m_failedcount == m_allassets.assets.Num())
	{
		log_tips = FString("some file upload failed.");

		m_status = uploadstatus::failed;
	}
}

void uploader::on_all_assets_upload_completed()
{
	//上传本次版本的资源信息
	FString contentStr;
	dataconversion::Write(m_allassets, contentStr);
	const FString& assetVersionobjname = m_userAccount / m_sv.latestversion / m_sv.latestversion + FString("_version.json");
	OSSManager::Get()->PutObjectByMemory(m_bucketName, contentStr, assetVersionobjname);

	//上传当前账号的版本list
	contentStr.Empty();
	dataconversion::Write(m_sv, contentStr);
	const FString& versionlistname = m_userAccount / m_userAccount + FString("_versionlist.json");
	OSSManager::Get()->PutObjectByMemory(m_bucketName, contentStr, versionlistname);

	//删除本地记录已上传完成的临时文件
	IFileManager::Get().Delete(*getlocaluploadcompletedfile());

	log_tips = FString("all upload completed.");

	m_status = uploadstatus::succeed;

	UE_LOG(UploadOSSLog, Warning, TEXT("all upload completed."));
}


#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", on)
#endif
