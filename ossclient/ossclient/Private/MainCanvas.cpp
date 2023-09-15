#include "MainCanvas.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SEditableText.h"

#include "uploader.h"
#include "OSSManager.h"


#define LOCTEXT_NAMESPACE "SMainCanvas"


#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", off)
#endif


/**
 * 定义这几个全局变量是因为上传用的是异步，每次点击上传按钮的时候给这几个值存上
 * Slate can only be accessed from the GameThread or the SlateLoadingThread
 */
// FString g_userAccount;
// FString g_bucketName;
// FString g_osslink;
// FString g_filepath;
// FString g_folderpath;


void SMainCanvas::Construct(const FArguments& InArgs)
{
	SConstraintCanvas::Construct(InArgs);

	m_osslinkText.BindRaw(this, &SMainCanvas::GetosslinkText);
	m_progressText.BindRaw(this, &SMainCanvas::GetprogressText);
	m_btnEnable.BindRaw(this, &SMainCanvas::GetbttonEnable);

	const FLinearColor t_orange = FLinearColor(1.f, 0.144128f, 0.f);

	FTextBlockStyle buttonTextstyle = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText");
	buttonTextstyle.SetColorAndOpacity(FSlateColor(t_orange));
	buttonTextstyle.SetFont(FCoreStyle::GetDefaultFontStyle("Roboto", 10));

	//测试用的提示
	AddSlot()
		.Anchors(FAnchors(0.f))
		.Offset(FMargin(0.f))
		.Alignment(FVector2D::ZeroVector)	//同样的, 对应Alignment是个FVector2D
		.AutoSize(true)			//对应AutoSize
		.ZOrder(0)
		[
			SNew(STextBlock)
			.Text(m_progressText)
			.ColorAndOpacity(FSlateColor(t_orange))
			.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
		];

	//按钮
	AddSlot()
		.Anchors(FAnchors(0.5f, 1.f))
		.Offset(FMargin(0.f, -30.f, 110, 30))
		.Alignment(FVector2D(0.5f, 1.f))	//同样的, 对应Alignment是个FVector2D
		.AutoSize(false)			//对应AutoSize
		.ZOrder(0)
		[
			SAssignNew(m_commitbtn, SButton)
			.IsEnabled(m_btnEnable)
			.Text(LOCTEXT("commit resource", "commit resource"))
			.TextStyle(&buttonTextstyle)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			.OnClicked(this, &SMainCanvas::OnCommitResources)
		];


	//参数列
	AddSlot()
		.Anchors(FAnchors(0.5f))
		.Offset(FMargin(0.f, -20.f, 540.f, 200.f))
		.Alignment(FVector2D(0.5f, 0.5f))	//同样的, 对应Alignment是个FVector2D
		.AutoSize(false)			//对应AutoSize
		.ZOrder(0)
		[
			SNew(SVerticalBox)
			
			//bucketName
			+SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("bucketName", "bucketName:"))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
				+SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(3.f)
				[
					SAssignNew(m_ossBucketName, SEditableText)
					.Text(LOCTEXT("bucketName value", ""))
					.HintText(LOCTEXT("hint bucketName value", "input your bucket name, this cannot be empty."))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
					.OnTextChanged_Raw(this, &SMainCanvas::OnBucketNameChanged)
				]
			]

			//链接
			+SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("osslink", "osslink:"))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
				+SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(3.f)
				[
					SAssignNew(m_osslink,SEditableText)
					.Text(m_osslinkText)
					.IsEnabled(false)
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
			]

			//userAccount
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("userAccount", "userAccount:"))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(3.f)
				[
					SAssignNew(m_userAccount, SEditableText)
					.Text(LOCTEXT("userAccount value", ""))
					.HintText(LOCTEXT("hint userAccount value", "input your userAccount, this cannot be empty."))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
					.OnTextChanged_Raw(this, &SMainCanvas::OnUserAccountNameChanged)
				]
			]

			//fileNames
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("file path", "filepath:"))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(3.f)
				[
					SAssignNew(m_filePath, SEditableText)
					.Text(LOCTEXT("filepath value", ""))
					.HintText(LOCTEXT("hint filepath value", "input your full file path."))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
			]

			//folder
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("folder path", "folder path:"))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.FillWidth(3.f)
				[
					SAssignNew(m_folderPath, SEditableText)
					.Text(LOCTEXT("folder path value", ""))
					.HintText(LOCTEXT("hint folder value", "input your full folder path."))
					.ColorAndOpacity(FSlateColor(t_orange))
					.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
				]
			]
		
		];
		//Chinese is currently not supported
		//文字提示
		AddSlot()
			.Anchors(FAnchors(0.f, 1.f))
			.Offset(FMargin(0.f))
			.Alignment(FVector2D(0.f, 1.f))	//同样的, 对应Alignment是个FVector2D
			.AutoSize(true)			//对应AutoSize
			.ZOrder(0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("note", "note: chinese is currently not supported!!!"))
				.ColorAndOpacity(FSlateColor(t_orange))
				.Font(FCoreStyle::GetDefaultFontStyle("Roboto", 10))
			];
}

FReply SMainCanvas::OnCommitResources()
{
	FString g_userAccount = m_userAccount->GetText().ToString();
	FString g_bucketName = m_ossBucketName->GetText().ToString();
	FString g_osslink = m_osslink->GetText().ToString();
	FString g_filepath = m_filePath->GetText().ToString();
	FString g_folderpath = m_folderPath->GetText().ToString();

	uploader::get()->commit(g_userAccount, g_bucketName, g_osslink, g_filepath, g_folderpath);

	return FReply::Handled();
}

void SMainCanvas::OnBucketNameChanged(const FText& Text)
{
	//m_commitbtn->SetEnabled(GetbttonEnable());
}

void SMainCanvas::OnUserAccountNameChanged(const FText& Text)
{
	//m_commitbtn->SetEnabled(GetbttonEnable());
}

FText SMainCanvas::GetosslinkText() const
{
	if(m_ossBucketName.IsValid())
	{
		const FString t_osslink = FString("https://") + m_ossBucketName->GetText().ToString() + TEXT(".") + OSSManager::Get()->get_endpoint() + TEXT("/");
		return FText::FromString(t_osslink);
	}
	return FText();
}

FText SMainCanvas::GetprogressText() const
{
	return FText::FromString(uploader::get()->log_tips);
}

bool SMainCanvas::GetbttonEnable() const
{
	const bool textNotempty = (false == m_userAccount->GetText().IsEmpty() && false == m_ossBucketName->GetText().IsEmpty());
	const bool isnotuploading = (uploader::get()->get_status() != uploadstatus::uploading);

	return textNotempty && isnotuploading;
}


#if OSS_OPEN_OPTIMIZE
#pragma optimize( "", on)
#endif

#undef LOCTEXT_NAMESPACE
