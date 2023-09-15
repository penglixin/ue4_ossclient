// Copyright Epic Games, Inc. All Rights Reserved.


#include "ossclient.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Slate/Public/Framework/Application/SlateApplication.h"
#include "Developer/StandaloneRenderer/Public/StandaloneRenderer.h"
#include "MainCanvas.h"
#include "OSSManager.h"
#include "uploader.h"

#define LOCTEXT_NAMESPACE "ossclient"

DEFINE_LOG_CATEGORY_STATIC(Logossclient, Log, All);

IMPLEMENT_APPLICATION(ossclient, "ossclient");

// INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
// {
// 	GEngineLoop.PreInit(ArgC, ArgV);
// 	UE_LOG(Logossclient, Display, TEXT("Hello World"));
// 	return 0;
// }

int WINAPI WinMain(_In_ HINSTANCE hInInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int nCmdShow)
{
	GEngineLoop.PreInit(GetCommandLineW());

	// crank up a normal Slate application using the platform's standalone renderer
	FSlateApplication::InitializeAsStandaloneApplication(GetStandardStandaloneRenderer());
	//const FSlateBrush* icon = FCoreStyle::Get().GetBrush(TEXT("TrashCan"));

	//FString config = FPaths::ProjectDir(); //../../../Engine/Programs/ossclient/

	//const FText FileFilterType = NSLOCTEXT("GitSourceControl", "Executables", "Executables");
	//const FString FileFilterText = FString::Printf(TEXT("%s (*.exe)|*.exe"), *FileFilterType.ToString());

	// 建立窗口
	TSharedPtr<SWindow> MainWindow = SNew(SWindow)
		.Title(FText::FromString("OSS-Client"))
		.ClientSize(FVector2D(700, 400))
		[
			SNew(SMainCanvas)
		];
	FSlateApplication::Get().AddWindow(MainWindow.ToSharedRef());

	// 消息循环
	while (!IsEngineExitRequested())
	{
		FSlateApplication::Get().Tick();
		FSlateApplication::Get().PumpMessages();
	}

	OSSManager::Destroy();
	uploader::destroy();

	FSlateApplication::Shutdown();
	return 0;
}

#undef LOCTEXT_NAMESPACE