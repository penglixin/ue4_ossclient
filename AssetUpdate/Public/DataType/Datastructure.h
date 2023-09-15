// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Datastructure.generated.h"


#define RESUME_UPLOAD_SIZE 1024*1024*5   //5MB

#define RESUME_DOWNLOAD_SIZE 1024*1024*10   //10MB

#define MAX_RETRY_COUNT 5 //����ʧ����������������


enum requestStatus
{
	ready,         //׼������û��ʼ��������
	downloading,   //��������
	succeed,       //�������
	failed,        //ʧ��
	retry          //������
};

enum requestType
{
	versionlist, //������Դ�汾�б�
	version,     //����ĳ���汾�İ汾��Ϣ
	asset        //������Դ
};

USTRUCT(BlueprintType)
struct FOneAssetVersionInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString uniqueNumber;   //��ԴΨһ���

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString companycode;    //��ҵ��

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString md5code;     //MD5

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString downloadlink; //��������

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int64 filesize;    //�ļ���С

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
		FString versionName;   //�汾��
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString md5code; //У����

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<FOneAssetVersionInfo> assets;    //��Դ

	void reset()
	{
		versionName.Empty();
		md5code.Empty();
		assets.Empty();
	}
};


struct serverversion 
{
	FString latestversion; //oss�����°汾 ��ʽΪ��userAccount_r_1.0.0
	FString md5code; //У����
	TArray<TTuple<FString, FString>> historyversions; //��ʷ�汾 TTuple<�汾�ţ�md5>
	
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