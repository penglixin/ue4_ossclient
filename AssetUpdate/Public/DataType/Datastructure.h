// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Datastructure.generated.h"


#define RESUME_UPLOAD_SIZE 1024*1024*5   //5MB

#define RESUME_DOWNLOAD_SIZE 1024*1024*10   //10MB

#define MAX_RETRY_COUNT 5 //请求失败重新请求最大次数


enum requestStatus
{
	ready,         //准备，还没开始发送请求
	downloading,   //正在下载
	succeed,       //下载完成
	failed,        //失败
	retry          //待重试
};

enum requestType
{
	versionlist, //下载资源版本列表
	version,     //下载某个版本的版本信息
	asset        //下载资源
};

USTRUCT(BlueprintType)
struct FOneAssetVersionInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString uniqueNumber;   //资源唯一编号

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString companycode;    //企业码

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString md5code;     //MD5

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString downloadlink; //下载链接

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int64 filesize;    //文件大小

	FORCEINLINE friend bool operator==(const FOneAssetVersionInfo& A, const FOneAssetVersionInfo& B)
	{
		return (A.md5code == B.md5code);
	}
};

USTRUCT(BlueprintType)
struct FAssetVersionInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString versionName;   //版本号
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString md5code; //校验码

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
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
	ASSETUPDATE_API bool Read(const FString& Json, FAssetVersionInfo& Infos);
	ASSETUPDATE_API void Write(const FAssetVersionInfo& Infos, FString& Json);

	ASSETUPDATE_API bool Read(const FString& Json, serverversion& Infos);
	ASSETUPDATE_API void Write(const serverversion& Infos, FString& Json);

	ASSETUPDATE_API bool Read(const FString& Json, transfercompleteinfo& Infos);
	ASSETUPDATE_API void Write(const transfercompleteinfo& Infos, FString& Json);
}