

#include "OSSManager.h"
#include "OSSMacro.h"
#include "oss_api.h"
#include "aos_http_io.h"
#include <memory>
#include <fstream>
#include "Async/AsyncWork.h"
#include "Misc/ScopeLock.h"
//#include "Engine/Engine.h"
#include "Async/TaskGraphInterfaces.h"
#include "aos_string.h"
#include "oss_define.h"
#include "CoreGlobals.h"
#include "Misc/ConfigCacheIni.h"

#include <direct.h>  
#include <stdio.h>
#include <Misc/FileHelper.h>
#include "Json.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"
#include <thread>

#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", off)
#endif

OSSManager* OSSManager::m_oss = nullptr;


class FOSSAsynTask :public FNonAbandonableTask
{
public:
	FOSSAsynTask(const TFunction<void()> InFuncton)
		:Functon(InFuncton)
	{
	}

	void DoWork()
	{
		Functon();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FOSSAsynTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	const TFunction<void()> Functon;
};

OSSManager::OSSManager()
{

}

OSSManager::~OSSManager()
{

}

OSSManager* OSSManager::Get()
{
	if (!m_oss)
	{
		m_oss = new OSSManager();

		if (aos_http_io_initialize(NULL, 0) != AOSE_OK)
		{
			exit(1);
		}

		m_oss->initial_oss_account();
	}
	return m_oss;
}

void OSSManager::Destroy()
{
	if (m_oss)
	{
		aos_http_io_deinitialize();

		delete m_oss;
	}
	m_oss = nullptr;
}

FString OSSManager::get_current_exe_path()
{
	char exeFullPath[MAX_PATH]; // Full path 

	getcwd(exeFullPath, MAX_PATH);

	std::string temp_str = exeFullPath;
	temp_str += "\\ossclientconfig";

	return temp_str.c_str();
}

FString OSSManager::get_endpoint()
{
	return Endpoint.c_str();
}

void OSSManager::initial_oss_account()
{
	FString full_configfile = get_current_exe_path();
	full_configfile += FString("\\oss_account.json");

	FString out_result;
	FFileHelper::LoadFileToString(out_result, *full_configfile);

	FString                         t_AccessKeyId;
	FString                         t_AccessKeySecret;
	FString                         t_Endpoint;

	TSharedPtr<FJsonValue> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(out_result);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		JsonParsed->AsObject()->TryGetStringField(TEXT("AccessKeyId"), t_AccessKeyId);
		JsonParsed->AsObject()->TryGetStringField(TEXT("AccessKeySecret"), t_AccessKeySecret);
		JsonParsed->AsObject()->TryGetStringField(TEXT("Endpoint"), t_Endpoint);

		AccessKeyId = TCHAR_TO_UTF8(*t_AccessKeyId);
		AccessKeySecret = TCHAR_TO_UTF8(*t_AccessKeySecret);
		Endpoint = TCHAR_TO_UTF8(*t_Endpoint);
	}
}

void OSSManager::init_options(void* inoptions)
{
	oss_request_options_t* options = (oss_request_options_t*)inoptions;
	options->config = oss_config_create(options->pool);

	/*Initializes the aos_string_t type with a string of type ansichar *. */
	aos_str_set(&options->config->endpoint, Endpoint.c_str());
	aos_str_set(&options->config->access_key_id, AccessKeyId.c_str());
	aos_str_set(&options->config->access_key_secret, AccessKeySecret.c_str());

	/*Whether to use CNAME. 0 means not used */
	//options->config->is_cname = Conf.bCname;
	options->config->is_cname = false;

	/*Set network related parameters, such as timeout. */
	options->ctl = aos_http_controller_create(options->pool, 0);

	/*Set the link timeout, which is 10 seconds by default. */
	//options->ctl->options->connect_timeout = Conf.ConnectTimeoutS;
	options->ctl->options->connect_timeout = 10;

	/*Set DNS timeout, default is 60 seconds. */
	//options->ctl->options->dns_cache_timeout = Conf.DNSTimeoutS;
	options->ctl->options->dns_cache_timeout = 60;

	/*
	Set request timeout:
	The minimum rate that can be tolerated is controlled by setting the value of speed limit, which is 1024 by default, i.e. 1KB / s.
	The maximum time that can be tolerated is controlled by setting the value of speed "time, which is 15 seconds by default.
	Indicates a timeout if the transfer rate is less than 1KB / s for 15 seconds.
	*/
	//options->ctl->options->speed_limit = Conf.MinimumRateOfTolerance;
	//options->ctl->options->speed_time = Conf.MinimumRateOfToleranceTime;

	options->ctl->options->speed_limit = 1024;
	options->ctl->options->speed_time = 15;
}

ANSICHAR* OSSManager::multibyte_to_utf8(const ANSICHAR* ch, ANSICHAR* str, int32 size)
{
	if (!ch || !str || size <= 0)
	{
		return NULL;
	}

	int32 chlen = strlen(ch);
	int32 multi_byte_cnt = 0;
	for (int32 i = 0; i < chlen - 1; i++)
	{
		if ((ch[i] & 0x80) && (ch[i + 1] & 0x80))
		{
			i++;
			multi_byte_cnt++;
		}
	}

	if (multi_byte_cnt == 0)
	{
		return const_cast<ANSICHAR*>(ch);
	}

	int32 len = MultiByteToWideChar(CP_ACP, 0, ch, -1, NULL, 0);
	if (len <= 0)
	{
		return NULL;
	}

	WIDECHAR* wch = (WIDECHAR*)malloc(sizeof(WIDECHAR) * (len + 1));
	if (!wch)
	{
		return NULL;
	}

	wmemset(wch, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, ch, -1, wch, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wch, -1, NULL, 0, NULL, NULL);
	if ((len <= 0) || (size < len + 1))
	{
		free(wch);

		return NULL;
	}
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wch, -1, str, len, NULL, NULL);
	free(wch);

	return str;
}

FOneAssetUploadCompleted& OSSManager::OnOneAssetUploadCompleted()
{
	return m_completedDelegate;
}

bool OSSManager::GetObjectToMemory(
	const FString& InBucketName, 
	const FString& InObjectName, 
	FString& ContextString, 
	FRange Range /*= FRange()*/,
	ProgressCallback InProgressCallback /*= nullptr*/)
{
	FScopeLock ScopeLock(&Mutex);

	CREATE_CONFIGURATION_AND_STORAGE

	STRING_TO_AOS_STRING_T(bucket, InBucketName);
	STRING_TO_AOS_STRING_T(object, InObjectName);

	aos_list_t buffer;
	aos_buf_t* content = NULL;
	aos_table_t* params = NULL;
	aos_table_t* headers = NULL;
	aos_table_t* resp_headers = NULL;
	aos_status_t* resp_status = NULL;
	ANSICHAR* buf = NULL;
	int64_t len = 0;
	int64_t size = 0;
	int64_t pos = 0;
	aos_list_init(&buffer);

	/* set range*/
	if (Range.IsValid())
	{
		headers = aos_table_make(pool, 0);
		apr_table_set(headers, "Range", TCHAR_TO_ANSI(*Range.ToString()));
	}

	/*Download files to local memory. */
	if (InProgressCallback)
	{
		resp_status = oss_do_get_object_to_buffer(oss_client_options, &bucket, &object,
			headers, params, &buffer, InProgressCallback, &resp_headers);
	}
	else
	{
		resp_status = oss_get_object_to_buffer(oss_client_options, &bucket, &object,
			headers, params, &buffer, &resp_headers);
	}

	auto DownLoadString = [&]()
	{
		/*Copy the download to the buffer. */
		len = aos_buf_list_len(&buffer);
		buf = (ANSICHAR*)aos_pcalloc(pool, len + 1);
		buf[len] = '\0';

		aos_list_for_each_entry(aos_buf_t, content, &buffer, node)
		{
			size = aos_buf_size(content);
			memcpy(buf + pos, content->pos, size);
			pos += size;
		}

		ContextString = ANSI_TO_TCHAR(buf);
	};

	/*Determine whether the upload is successful. */
	CHECK_OSS_RESULT(DownLoadString();
	OSS_SUCCESS_MSG("get object to buffer succeeded\n");,
		OSS_ERROR_MSG("get object to buffer failed\n"););

	/*Get response message of OSS server */
	OSS_SDK_RESPONSE_MSG

	return bVisitSuccess;
}

bool OSSManager::PutObjectByMemory(
	const FString& InBucketName,
	const FString& Data, 
	const FString& InObjectName /*= FString()*/,
	ProgressCallback InProgressCallback /*= nullptr*/,
	const TMap<FString, FString>& OSSMeta /*= TMap<FString, FString>()*/)
{
	STRING_TO_AOS_STRING_T(bucket, InBucketName);
	STRING_TO_AOS_STRING_T(object, InObjectName);
	CREATE_CONFIGURATION_AND_STORAGE;

	string Object_Data = TCHAR_TO_UTF8(*Data);

	aos_list_t buffer;
	aos_buf_t* content = NULL;
	aos_table_t* params = nullptr;
	aos_table_t* headers = NULL;
	aos_table_t* resp_headers = NULL;
	aos_status_t* resp_status = NULL;
	aos_list_t* resp_body = nullptr;
	aos_list_init(&buffer);
	content = aos_buf_pack(oss_client_options->pool, Object_Data.c_str(), strlen(Object_Data.c_str()));
	aos_list_add_tail(&content->node, &buffer);

	if (InProgressCallback)
	{
		resp_status = oss_do_put_object_from_buffer(oss_client_options, &bucket, &object, &buffer, headers, params, InProgressCallback, &resp_headers, resp_body);
	}
	else
	{
		resp_status = oss_put_object_from_buffer(oss_client_options, &bucket, &object, &buffer, headers, &resp_headers);
	}

	/*Determine whether the upload is successful. */
	CHECK_OSS_RESULT(
		OSS_SUCCESS_MSG("put object from buffer succeeded\n."),
		OSS_ERROR_MSG("put object from buffer failed\n"))

		/*Get response message of OSS server */
		OSS_SDK_RESPONSE_MSG

		return bVisitSuccess;
}

FString NormalizePaths(const FString& InPaths, const FString& InFilename)
{
	FString Filename = FPaths::GetCleanFilename(InFilename);

	FString NewInlocalPaths = InPaths;
	if (!NewInlocalPaths.Contains(TEXT(".")))
	{
		int32 Position = NewInlocalPaths.Len() - 1;
		int32 i = 0;
		FPaths::NormalizeFilename(NewInlocalPaths);

		if (NewInlocalPaths.FindLastChar('/', i))
		{
			if (i == Position)
			{
				NewInlocalPaths += Filename;
			}
			else
			{
				NewInlocalPaths /= Filename;
			}
		}
		else
		{
			NewInlocalPaths /= Filename;
		}

		if (!InPaths.Contains(TEXT(":")))
		{
			checkf(NewInlocalPaths == InPaths, TEXT("This path is not valid. Please provide the correct path."));
		}
	}

	return NewInlocalPaths;
}

bool OSSManager::ResumableUploadObject(
	const FString& InBucketName, 
	const FString& InObjectName, 
	const FString& InUploadFilePath, 
	int64 PartSize /*= 1024 * 1024 * 10*/,
	ProgressCallback InProgressCallback /*= nullptr*/, 
	const TMap<FString, FString>& OSSMeta /*= TMap<FString, FString>()*/)
{
	FScopeLock ScopeLock(&Mutex);

	const FString NewLocalPaths = NormalizePaths(InUploadFilePath, InObjectName);

	CREATE_CONFIGURATION_AND_STORAGE;

	STRING_TO_AOS_STRING_T(bucket, InBucketName);
	STRING_TO_AOS_STRING_T(object, InObjectName);
	UTF8_STRING_TO_AOS_STRING_T(file, NewLocalPaths); //Avoid Chinese characters being unable to upload 

	aos_list_t resp_body;
	aos_table_t* headers = NULL;
	aos_table_t* resp_headers = NULL;
	aos_status_t* resp_status = NULL;
	oss_resumable_clt_params_t* clt_params;
	aos_list_init(&resp_body);

	/*Set user-defined meta information. */
	CLAIM_METADATA(OSSMeta);

	int32 ThreadNumber = std::thread::hardware_concurrency();

	/* 断点续传。*/
	clt_params = oss_create_resumable_clt_params_content(pool, PartSize, ThreadNumber, AOS_TRUE, NULL);
	resp_status = oss_resumable_upload_file(oss_client_options, &bucket, &object, &file, headers, NULL, clt_params, InProgressCallback, &resp_headers, &resp_body);

	/*Determine whether the upload is successful. */
	CHECK_OSS_RESULT(
		OSS_SUCCESS_MSG("Resumable upload %s success.", *InObjectName),
		OSS_ERROR_MSG("Resumable upload %s failed.", *InObjectName))

		/*Get response message of OSS server */
		OSS_SDK_RESPONSE_MSG

	m_completedDelegate.ExecuteIfBound(InUploadFilePath, bVisitSuccess);

	return bVisitSuccess;
}

void OSSManager::AsyncResumableUploadObject(
	const FString& InBucketName, 
	const FString& InObjectName,
	const FString& InUploadFilePath,
	int64 PartSize, 
	ProgressCallback InProgressCallback /*= nullptr*/, 
	const TMap<FString, FString>& OSSMeta /*= TMap<FString, FString>()*/)
{
	auto Lamada = [=]()
	{
		ResumableUploadObject(
			InBucketName,
			InObjectName,
			InUploadFilePath,
			PartSize,
			InProgressCallback,
			OSSMeta);
	};

	(new FAutoDeleteAsyncTask<FOSSAsynTask>(Lamada))->StartBackgroundTask();
}

#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", on)
#endif