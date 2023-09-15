#include "AssetUpdate.h"
#include "Assetoperation/Assetdownloader.h"

#define LOCTEXT_NAMESPACE "FAssetUpdateModule"

void FAssetUpdateModule::StartupModule()
{
}

void FAssetUpdateModule::ShutdownModule()
{
    UAssetdownloader::DestroyInstance();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FAssetUpdateModule, AssetUpdate)