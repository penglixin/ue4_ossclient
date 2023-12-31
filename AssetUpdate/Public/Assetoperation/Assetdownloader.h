

#pragma once

#include "CoreMinimal.h"
#include "Assetoperation/AssetOperation.h"
#include "Assetoperation/HttpRequest/downloadRequest.h"
#include "Assetdownloader.generated.h"


 // 单个资源的单个子任务的进度条  for BP
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOneAssetdownloadProgress, const FString&, assetname, const float&, progress, const float&, totalprogress, const int32&, taskid, const int32&, totalTaskCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOneAssetCompleted, const FString&, assetname);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllAssetsCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAssetDownloadFailed, const FString&, assetname);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRequestVersionInfoFailed);
// for c++

DECLARE_DELEGATE_FiveParams(FSimpleOneAssetdownloadProgress, const FString&, const float&, const float&, const int32&, const int32&);
DECLARE_DELEGATE_OneParam(FSimpleOneAssetCompleted, const FString&);
DECLARE_DELEGATE(FSimpleAllAssetsCompleted);
DECLARE_DELEGATE_OneParam(FSimpleAssetDownloadFailed, const FString&);
DECLARE_DELEGATE(FSimpleRequestVersionInfoFailed);

/**
 * 资源下载，版本校验等 ，这步是在客户端执行的，只有读操作，所以就用http，不会使用到oss sdk
 */
UCLASS()
class ASSETUPDATE_API UAssetdownloader : public UAssetOperation
{
	GENERATED_BODY()

public:
	UFUNCTION(blueprintpure)
	static UAssetdownloader* Get_Assetdownloader_Instance();
	static void DestroyInstance();

public:
	//校验本地资源是不是服务器上的最新版本 接着下载 （入口）
	UFUNCTION(BlueprintCallable)
	void check_assets(const FString& accountID);


public:
	/** 下载完一个资源把这个资源写入到本地资源版本文件中 */
	void oneAssetdownloadcomplete(const FOneAssetVersionInfo& asset);

	/** 创建下载任务 */
	void createtasks();

	/** 执行下载任务 */
	void executetasks();
private:
	
	requestType convertRequest(const FString& url)
	{
		if (url.Contains(m_versionlistName))
		{
			return requestType::versionlist;
		}
		else if (url.Contains(m_userversionName))
		{
			return requestType::version;
		}
		return requestType::asset;
	}

	//下载版本信息暂时不用断点续传
	void downloadVersionInfo(const FString& url);

	void Handle_version_downloadcomplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void Handle_version_downloadprogress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesRecv);

	void download_versionlist_complete(FHttpResponsePtr HttpResponse);  //资源列表下载完成
	void download_userversion_complete(FHttpResponsePtr HttpResponse);  //资源版本下载完成

	/** 版本文件对比完，开始下载资源 */
	void versionfilecomparecompleted();

public:

	UPROPERTY(BlueprintAssignable)
		FOneAssetdownloadProgress downloading;

	UPROPERTY(BlueprintAssignable)
		FOneAssetCompleted oneCompleted;

	UPROPERTY(BlueprintAssignable)
		FAllAssetsCompleted allCompleted;

	UPROPERTY(BlueprintAssignable)
		FAssetDownloadFailed oneFailed;

	UPROPERTY(BlueprintAssignable)
		FRequestVersionInfoFailed reuestversionFailed;

	FSimpleOneAssetdownloadProgress& OnAssetProgress();
	FSimpleOneAssetCompleted& OnAssetDownloadCompleted();
	FSimpleAllAssetsCompleted& OnAllAssetsCompleted();
	FSimpleAssetDownloadFailed& OnAssetFailed();
	FSimpleRequestVersionInfoFailed& OnRequestVersionFailed();
private:
	static UAssetdownloader*                        ThisPtr;
	/** 服务器版本列表 */
	serverversion                                   m_sv;
	/** 服务器上最新版本，需要在所有待下载资源都下完之后再存到本地 */
	FAssetVersionInfo                               m_latestversion;
	/** 需要下载的pak链接 */
	TArray<FOneAssetVersionInfo>                    m_downAssets;
	/** 本地已经下载完成的资源信息，防止重复下载 */
	transfercompleteinfo                            m_localdowncomplete;
	/** 下载任务 */
	TArray<TSharedPtr<FdownloadRequset>>            m_Tasks;

	/** c++ 回调 */
	FSimpleOneAssetdownloadProgress                 simple_downloading;
	FSimpleOneAssetCompleted                        simple_oneCompleted;
	FSimpleAllAssetsCompleted                       simple_allCompleted;
	FSimpleAssetDownloadFailed                      simple_oneFailed;
	FSimpleRequestVersionInfoFailed                 simple_reuestversionFailed;
};
