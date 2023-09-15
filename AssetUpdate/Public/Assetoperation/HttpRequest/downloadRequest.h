
#pragma once

#include "CoreMinimal.h"
#include "Assetoperation/HttpRequest/subRequset.h"

/**
 * 一个下载任务
 */
class  FdownloadRequset :public TSharedFromThis<FdownloadRequset>
{
public:
	FdownloadRequset();
	virtual ~FdownloadRequset();

public:
	/** 创建任务 */
	void createtask(const FOneAssetVersionInfo& in_info);

	/** 开始下载 */
	void begindownload();
private:
	FString getSavefilename(const FString& downlink);

	/** 分解一个下载任务 */
	void breakTask(const FOneAssetVersionInfo& asset);

	/** 更新任务 */
	void executesubtasks();

	/** 所有子任务下载完成 */
	void onsucceed();

	/** 失败 */
	void onfailed(const FString& errorMesg, const int32& taskID);

	void Handle_task_complete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 taskID);
	void Handle_task_progress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesRecv, int32 taskID);

public:
	FORCEINLINE requestStatus get_status() { return m_status; }

private:
	/** 需要下载的资源信息 */
	FOneAssetVersionInfo                            m_assetinfo;
	/** 缓存文件存储文件夹 */
	FString					                        m_cachefolder = FPaths::ProjectSavedDir() / TEXT("httpcache");
	/** 子任务 */
	TArray<TSharedPtr<FSubRequest>>                 m_subTasks;
	/** 已下载的字节数 */
	int64                                           m_hasdownloadsie = 0;
	/** 下载任务状态 */
	requestStatus                                   m_status;
	/** 重试次数 */
	int32                                           m_retrycount = 0;
};