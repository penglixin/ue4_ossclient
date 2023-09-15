#pragma once

#include "CoreMinimal.h"


#define RESUME_UPLOAD_SIZE 1024*1024*5   //5MB


enum uploadstatus
{
	beginupload = 0,
	uploading,
	succeed,
	failed
};

struct FOneAssetVersionInfo
{
	FString uniqueNumber;   //资源唯一编号

	FString companycode;    //企业码

	FString md5code;     //MD5

	FString downloadlink; //下载链接

	int64 filesize;    //文件大小

	FORCEINLINE friend bool operator==(const FOneAssetVersionInfo& A, const FOneAssetVersionInfo& B)
	{
		return (A.md5code == B.md5code);
	}
};

struct FAssetVersionInfo
{
	FString versionName;   //版本号

	FString md5code; //校验码

	TArray<FOneAssetVersionInfo> assets;    //资源

	void reset()
	{
		versionName.Empty();
		md5code.Empty();
		assets.Empty();
	}
};


struct serverversion
{
	FString latestversion; //oss上最新版本 格式为：userAccount_r_1.0.0
	FString md5code; //校验码
	TArray<TTuple<FString, FString>> historyversions; //历史版本 TTuple<版本号，md5>

	bool isempty()
	{
		return latestversion.IsEmpty();
	}

	void reset()
	{
		latestversion.Empty();
		md5code.Empty();
		historyversions.Empty();
	}
};

struct transfercompleteinfo
{
	TArray<FOneAssetVersionInfo> transfercompleteassets;

	void reset()
	{
		transfercompleteassets.Empty();
	}
};



namespace dataconversion
{
	bool Read(const FString& Json, FAssetVersionInfo& Infos);
	void Write(const FAssetVersionInfo& Infos, FString& Json);

	bool Read(const FString& Json, serverversion& Infos);
	void Write(const serverversion& Infos, FString& Json);

	bool Read(const FString& Json, transfercompleteinfo& Infos);
	void Write(const transfercompleteinfo& Infos, FString& Json);
}