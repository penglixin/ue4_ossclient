
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "DataType/Datastructure.h"


/**
 * ��http�����ٷ�װһ�㣬�ϵ�����ʱ���ڲ������
 */
class  FSubRequest :public TSharedFromThis<FSubRequest>
{
public:
	FSubRequest();
	virtual ~FSubRequest();


public:
	/** Returns reference to internal request object */
	TSharedRef<IHttpRequest> GetHttpRequest();

	/** ���滺�浽���� */
	void savecacheData();
	void loadcacheData();

public:
	void set_taskID(const int32& in_val) { m_taskID = in_val; }
	int32 get_taskID() { return m_taskID; }

	void set_requesturl(const FString& in_val) { m_requesturl = in_val; }
	FString get_requesturl() { return m_requesturl; }

	void set_range(const FString& in_val) { m_range = in_val; }
	FString get_range() { return m_range; }

	void set_md5(const FString& in_val) { m_md5code = in_val; }

	void set_startpos(const int32& in_val) { m_startpos = in_val; }
	int32 get_startpos() { return m_startpos; }

	void set_size(const int32& in_val) { m_size = in_val; }
	int32 get_size() { return m_size; }

	void set_rawdata(const TArray<uint8>& in_val) { m_rawdata = in_val; }
	TArray<uint8> get_rawdata() { return m_rawdata; }

	void set_status(const requestStatus& in_val) { m_status = in_val; }
	requestStatus get_status() { return m_status; }
private:

	//TSharedRef<IHttpRequest> m_HttpRequest;
	/** ����ID */
	int32                    m_taskID = 0;
	/** ������URL */
	FString					 m_requesturl;
	/** �����ļ��洢�ļ��� */
	FString					 m_cachefolder;
	/** md5 */
	FString					 m_md5code;
	/** ���ط�Χ : bytes=0-1024 */
	FString					 m_range;
	/** ��������״̬ */
	requestStatus            m_status;
	/** �ļ����ص��ֽ�λ�� */
	int32					 m_startpos = 0;
	/** ����������ֽڴ�С */
	int32					 m_size = 0;
	/** ��������������� */
	TArray<uint8>			 m_rawdata;
	/** ����ʱ�䣬�����жϳ�ʱ���� */
	//double					 m_requestTime = 0;
};