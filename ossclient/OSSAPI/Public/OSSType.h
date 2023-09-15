#pragma once

#include "CoreMinimal.h"

struct OSSAPI_API FRange
{

	int64 Start;
	int64 End;

	FRange()
		:Start(0)
		, End(0)
	{}

	FRange(int64 InStart, int64 InEnd)
		:Start(InStart)
		, End(InEnd)
	{}

	FORCEINLINE bool IsValid() { return Start < End&& End != 0; }

	FString ToString()
	{
		return FString::Printf(TEXT("bytes=%d-%i"), Start, End);
	}
};


typedef void(*ProgressCallback)(int64_t, int64_t);//consumed_bytes total_bytes
typedef void(*CallbackUploadPart)(FString, int64_t, int64_t, int64_t, bool, bool);