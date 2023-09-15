
#pragma once

#include "CoreMinimal.h"
#include "Assetoperation/HttpRequest/subRequset.h"

/**
 * һ����������
 */
class  FdownloadRequset :public TSharedFromThis<FdownloadRequset>
{
public:
	FdownloadRequset();
	virtual ~FdownloadRequset();

public:
	/** �������� */
	void createtask(const FOneAssetVersionInfo& in_info);

	/** ��ʼ���� */
	void begindownload();
private:
	FString getSavefilename(const FString& downlink);

	/** �ֽ�һ���������� */
	void breakTask(const FOneAssetVersionInfo& asset);

	/** �������� */
	void executesubtasks();

	/** ����������������� */
	void onsucceed();

	/** ʧ�� */
	void onfailed(const FString& errorMesg, const int32& taskID);

	void Handle_task_complete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 taskID);
	void Handle_task_progress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesRecv, int32 taskID);

public:
	FORCEINLINE requestStatus get_status() { return m_status; }

private:
	/** ��Ҫ���ص���Դ��Ϣ */
	FOneAssetVersionInfo                            m_assetinfo;
	/** �����ļ��洢�ļ��� */
	FString					                        m_cachefolder = FPaths::ProjectSavedDir() / TEXT("httpcache");
	/** ������ */
	TArray<TSharedPtr<FSubRequest>>                 m_subTasks;
	/** �����ص��ֽ��� */
	int64                                           m_hasdownloadsie = 0;
	/** ��������״̬ */
	requestStatus                                   m_status;
	/** ���Դ��� */
	int32                                           m_retrycount = 0;
};