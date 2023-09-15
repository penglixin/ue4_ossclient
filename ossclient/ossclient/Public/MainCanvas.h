#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCanvas.h"
#include "Widgets/Layout/SConstraintCanvas.h"


class SMainCanvas : public SConstraintCanvas
{
public:
	/*SLATE_BEGIN_ARGS(SMainCanvas)
	{
	}
	SLATE_END_ARGS()*/


	void Construct(const FArguments& InArgs);

	FReply OnCommitResources();

	void OnBucketNameChanged(const FText& Text);
	void OnUserAccountNameChanged(const FText& Text);

	FText GetosslinkText() const;
	FText GetprogressText() const;
	bool GetbttonEnable() const;
private:
	TSharedPtr<SButton> m_commitbtn;
	TSharedPtr<SEditableText> m_osslink;
	TSharedPtr<SEditableText> m_ossBucketName;
	TSharedPtr<SEditableText> m_userAccount;
	TSharedPtr<SEditableText> m_filePath;
	TSharedPtr<SEditableText> m_folderPath;

private:
	TAttribute<FText> m_osslinkText;
	TAttribute<FText> m_progressText;
	TAttribute<bool> m_btnEnable;
};

