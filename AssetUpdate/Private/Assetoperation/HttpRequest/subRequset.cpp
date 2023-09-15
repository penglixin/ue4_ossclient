
#include "Assetoperation/HttpRequest/subRequset.h"
#include "HttpModule.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"

#if WITH_EDITOR
#pragma optimize( "", off)
#endif

FSubRequest::FSubRequest()
	//:m_HttpRequest(FHttpModule::Get().CreateRequest())
{
	m_cachefolder = FPaths::ProjectSavedDir() / TEXT("httpcache");
	m_status = requestStatus::ready;
}

FSubRequest::~FSubRequest()
{
}

TSharedRef<IHttpRequest> FSubRequest::GetHttpRequest()
{
	TSharedRef<IHttpRequest> m_HttpRequest = FHttpModule::Get().CreateRequest();
	m_HttpRequest->SetURL(m_requesturl);
	m_HttpRequest->SetVerb(TEXT("GET"));
	m_HttpRequest->SetHeader(TEXT("Range"), m_range);
	//m_requestTime = FApp::GetCurrentTime();
	return m_HttpRequest;
}

void FSubRequest::savecacheData()
{
	FString subcachefilename = m_md5code / m_md5code + TEXT("_") + FString::FromInt(m_taskID) + TEXT(".hcf");
	FFileHelper::SaveArrayToFile(m_rawdata, *(m_cachefolder / subcachefilename));
}

void FSubRequest::loadcacheData()
{
	FString subcachefilename = m_md5code / m_md5code + TEXT("_") + FString::FromInt(m_taskID) + TEXT(".hcf");
	FFileHelper::LoadFileToArray(m_rawdata, *(m_cachefolder / subcachefilename));
}

#if WITH_EDITOR
#pragma optimize( "", on)
#endif