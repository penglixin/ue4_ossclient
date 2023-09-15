
#pragma once

#include "CoreMinimal.h"
#include "dataconversion.h"

class uploader
{
public:
	uploader();
	~uploader();

	static uploader* get();
	static void destroy();


	FString log_tips = "tips";

	/** 上传文件以及记录版本信息 */
	void commit(const FString& in_userAccount, const FString& in_bucketName, const FString& in_osslink, const FString& in_filepath, const FString& in_folderpath);

	uploadstatus get_status() const {
		return m_status;
	}
private:
	//;分割
	TArray<FString> parasePath(const FString& in_str);

	/** 检查本地已经上传完的资源 */
	void initial_localinfo();

	//获取所有要上传的资源
	TArray<FString> get_alluploadAssets();

	//获取当前版本号，先拉取oss上的版本列表,oss不存在，则返回 userAccount_r_1.0.0 并且构造一个 serverversion 结构
	serverversion get_newversion();

	FString construct_md5(const FString& filefullpath);

	//构建出此次上传资源的版本信息
	void constructAssetversioninfo(const FString& assetfile, FOneAssetVersionInfo& oneasset);

	/** 本地已经上传完成的资源json */
	FString getlocaluploadcompletedfile();

public:
	void assets_upload_progress(const int64& ConsumedBytes, const int64& TotalBytes);
	void on_one_asset_upload_completed(const FString& filefullpath, const bool& bsucceed);
	void on_all_assets_upload_completed();

private:
	/** 新的版本信息 */
	serverversion                             m_sv;
	/** 所有选择的文件，注意所有选择的文件不一定全部要上传，因为有断点续传，里面可能有些文件已经上传完成了 */
	FAssetVersionInfo                         m_allassets;
	/** 阿里云提供的进度条回调接口没有文件名，只有总字节数跟已发送字节数 */
	/** 需要上传的文件 */
	TMultiMap<int64, FOneAssetVersionInfo>    m_needuploadAssets;
	/** 已经上传完毕的文件数 */
	int32                                     m_succeedcount = 0;
	/** 上传失败的文件数 */
	int32                                     m_failedcount = 0;
	/** 本地已经上传完成的资源信息，防止重复上传 */
	transfercompleteinfo                      m_localuploadcomplete;

private:
	static uploader* m_uploader;
	FString m_userAccount;
	FString m_bucketName;
	FString m_osslink;
	FString m_filepath;
	FString m_folderpath;
	uploadstatus m_status = uploadstatus::beginupload;
};