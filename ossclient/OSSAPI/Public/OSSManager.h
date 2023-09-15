
#pragma once

#include "CoreMinimal.h"
#include "OSSType.h"
#include "Misc/Paths.h"
#include <string>
#if PLATFORM_WINDOWS
#include "Windows/WindowsCriticalSection.h"
#else 
#if PLATFORM_LINUX
#include "Linux/LinuxCriticalSection.h"
#else
#if PLATFORM_MAC
#include "Mac/MacCriticalSection.h"
#endif
#endif
#endif


DECLARE_DELEGATE_TwoParams(FOneAssetUploadCompleted, const FString&, const bool&);


using namespace std;


class OSSAPI_API OSSManager
{
public:
	OSSManager();
	~OSSManager();

	static OSSManager* Get();
	static void Destroy();

public:
	FString get_current_exe_path();
	FString get_endpoint();
private:
	void initial_oss_account(); // eg: C:\mx_425\Engine\Binaries\Win64\ossclientconfig
	void init_options(void* inoptions);
	ANSICHAR* multibyte_to_utf8(const ANSICHAR* ch, ANSICHAR* str, int32 size);
public:
	FOneAssetUploadCompleted& OnOneAssetUploadCompleted();

	bool GetObjectToMemory(const FString& InBucketName, const FString& InObjectName, FString& ContextString, FRange Range = FRange(), ProgressCallback InProgressCallback = nullptr);

	bool PutObjectByMemory(const FString& InBucketName, const FString& Data, const FString& InObjectName = FString(), ProgressCallback InProgressCallback = nullptr, const TMap<FString, FString>& OSSMeta = TMap<FString, FString>());

	bool ResumableUploadObject(const FString& InBucketName, const FString& InObjectName, const FString& InUploadFilePath, int64 PartSize = 1024 * 1024 * 10, ProgressCallback InProgressCallback = nullptr, const TMap<FString, FString>& OSSMeta = TMap<FString, FString>());

	void AsyncResumableUploadObject(const FString& InBucketName, const FString& InObjectName, const FString& InUploadFilePath, int64 PartSize, ProgressCallback InProgressCallback = nullptr, const TMap<FString, FString>& OSSMeta = TMap<FString, FString>());
private:
	static OSSManager*                  m_oss;
	std::string                         AccessKeyId;
	std::string                         AccessKeySecret;
	std::string                         Endpoint;					                    
	FCriticalSection                    Mutex;
	FOneAssetUploadCompleted            m_completedDelegate;  //目前上传资源只用到了断点续传，所以只有断点续传调了

};