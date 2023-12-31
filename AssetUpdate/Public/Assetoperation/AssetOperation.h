#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/Datastructure.h"
#include "AssetOperation.generated.h"

/**
 * 资源操作基类，上传下载各自从此继承
 */
UCLASS()
class ASSETUPDATE_API UAssetOperation : public UObject
{
	GENERATED_BODY()
	
public:
	//void refreshcompanycode(const FString& in_val) { m_companycode = in_val; }
	void refreshosslink(const FString& in_val) { m_osslink = in_val; }
	void refreshossbucketname(const FString& in_val) { m_ossBucketName = in_val; }
	void refreshuseraccount(const FString& in_val) { m_userAccount = in_val; }
	//void refreshselectfiles(const TArray<FFilePath>& in_val) { m_fileNames = in_val; }
	//void refreshfolder(const FDirectoryPath& in_val) { m_pakFolder = in_val; }

	//FORCEINLINE FString get_companycode() const { return m_companycode; }
	FORCEINLINE FString get_osslink() const { return m_osslink; }
	FORCEINLINE FString get_ossBucketName() const { return m_ossBucketName; }
	FORCEINLINE FString get_userAccount() const { return m_userAccount; }
	//FORCEINLINE TArray<FFilePath> get_fileNames() const { return m_fileNames; }
	//FORCEINLINE FDirectoryPath get_pakFolder() const { return m_pakFolder; }

public:
	/** 本地资源版本记录文件，用于对比oss上的资源版本 */
	FString getAssetversionFile() {
		const FString fullfilename = FPaths::ProjectDir() + TEXT("MiaoXiang/Cache/Compare") / get_userAccount() / FString::Printf(TEXT("%s_assets.json"), *get_userAccount());
		return fullfilename;
	}

	/** 本地已经下载完成的资源json */
	FString getlocaldowncompletedfile() {
		const FString fullfilename = FPaths::ProjectDir() + TEXT("MiaoXiang/Cache/Compare") / get_userAccount() / FString::Printf(TEXT("%s_downcompleted.json"), *get_userAccount());
		return fullfilename;
	}

	/** pak存放路径 */
	FString getSavefilename(const FString& downlink) {
		const FString fullfilename = FPaths::ProjectDir() + TEXT("MiaoXiang/Cache/Patch") / FPaths::GetCleanFilename(downlink);
		return fullfilename;
	}

public:
	const FString m_versionlistName = FString("_versionlist.json");
	const FString m_userversionName = FString("_version.json");
	const FString m_defaultversion = FString("_v_1.0.0");
	const FString m_pakExtesion = FString(".pak");

private:
	//为了不使 AssetUpdate 模块跟 MXEditor 互相引用，把需要从配置里获取到的字段在每次修改的时候赋值一遍
	//FString                 m_companycode;
	FString                 m_osslink;
	FString                 m_ossBucketName;
	FString                 m_userAccount; //用户账号，因为资源跟账户挂钩
	//TArray<FFilePath>       m_fileNames;
	//FDirectoryPath          m_pakFolder;
};
