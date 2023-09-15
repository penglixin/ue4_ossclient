
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "DataType/Datastructure.h"


/**
 * 将http请求再封装一层，断点续传时用于拆分任务
 */
class  FSubRequest :public TSharedFromThis<FSubRequest>
{
public:
	FSubRequest();
	virtual ~FSubRequest();


public:
	/** Returns reference to internal request object */
	TSharedRef<IHttpRequest> GetHttpRequest();

	/** 保存缓存到本地 */
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
	/** 任务ID */
	int32                    m_taskID = 0;
	/** 该请求URL */
	FString					 m_requesturl;
	/** 缓存文件存储文件夹 */
	FString					 m_cachefolder;
	/** md5 */
	FString					 m_md5code;
	/** 下载范围 : bytes=0-1024 */
	FString					 m_range;
	/** 下载任务状态 */
	requestStatus            m_status;
	/** 文件下载的字节位置 */
	int32					 m_startpos = 0;
	/** 该子任务的字节大小 */
	int32					 m_size = 0;
	/** 下载下来后的数据 */
	TArray<uint8>			 m_rawdata;
	/** 请求时间，用于判断超时处理 */
	//double					 m_requestTime = 0;
};