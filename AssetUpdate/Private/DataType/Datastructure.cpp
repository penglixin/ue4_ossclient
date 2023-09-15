
#include "DataType/Datastructure.h"
#include "Json.h"

bool dataconversion::Read(const FString& Json, FAssetVersionInfo& Infos)
{
	TSharedPtr<FJsonValue> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Json);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		JsonParsed->AsObject()->TryGetStringField(TEXT("versionName"), Infos.versionName);
		JsonParsed->AsObject()->TryGetStringField(TEXT("md5code"), Infos.md5code);

		TArray<TSharedPtr<FJsonValue>> JsonArr = JsonParsed->AsObject()->GetArrayField(TEXT("assets"));

		for (auto t_parsed : JsonArr)
		{
			const TSharedPtr<FJsonObject> JsonObjectArr = t_parsed->AsObject();
			FOneAssetVersionInfo oneAsset;

			JsonObjectArr->TryGetStringField(TEXT("uniqueNumber"), oneAsset.uniqueNumber);
			JsonObjectArr->TryGetStringField(TEXT("companycode"), oneAsset.companycode);
			JsonObjectArr->TryGetStringField(TEXT("md5code"), oneAsset.md5code);
			JsonObjectArr->TryGetStringField(TEXT("downloadlink"), oneAsset.downloadlink);
			JsonObjectArr->TryGetNumberField(TEXT("filesize"), oneAsset.filesize);
			Infos.assets.Add(oneAsset);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void dataconversion::Write(const FAssetVersionInfo& Infos, FString& Json)
{
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> t_JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Json);

	t_JsonWriter->WriteObjectStart();

	t_JsonWriter->WriteValue(TEXT("versionName"), Infos.versionName);
	t_JsonWriter->WriteValue(TEXT("md5code"), Infos.md5code);

	t_JsonWriter->WriteArrayStart(TEXT("assets"));
	{
		for (const auto& oneAsset : Infos.assets)
		{
			t_JsonWriter->WriteObjectStart();

			t_JsonWriter->WriteValue(TEXT("uniqueNumber"), oneAsset.uniqueNumber);
			t_JsonWriter->WriteValue(TEXT("companycode"), oneAsset.companycode);
			t_JsonWriter->WriteValue(TEXT("md5code"), oneAsset.md5code);
			t_JsonWriter->WriteValue(TEXT("downloadlink"), oneAsset.downloadlink);
			t_JsonWriter->WriteValue(TEXT("filesize"),oneAsset.filesize);

			t_JsonWriter->WriteObjectEnd();
		}
	}
	t_JsonWriter->WriteArrayEnd();

	t_JsonWriter->WriteObjectEnd();

	t_JsonWriter->Close();
}

bool dataconversion::Read(const FString& Json, serverversion& Infos)
{
	TSharedPtr<FJsonValue> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Json);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		JsonParsed->AsObject()->TryGetStringField(TEXT("latestversion"), Infos.latestversion);
		JsonParsed->AsObject()->TryGetStringField(TEXT("md5code"), Infos.md5code);

		TArray<TSharedPtr<FJsonValue>> JsonArr = JsonParsed->AsObject()->GetArrayField(TEXT("historyversions"));

		for (auto t_parsed : JsonArr)
		{
			const TSharedPtr<FJsonObject> JsonObjectArr = t_parsed->AsObject();
			
			FString versionName;
			FString md5code;
			JsonObjectArr->TryGetStringField(TEXT("versionName"), versionName);
			JsonObjectArr->TryGetStringField(TEXT("md5code"), md5code);
			Infos.historyversions.Add(MakeTuple(versionName, md5code));
		}
		return true;
	}
	else
	{
		return false;
	}
}

void dataconversion::Write(const serverversion& Infos, FString& Json)
{
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> t_JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Json);

	t_JsonWriter->WriteObjectStart();

	t_JsonWriter->WriteValue(TEXT("latestversion"), Infos.latestversion);
	t_JsonWriter->WriteValue(TEXT("md5code"), Infos.md5code);

	t_JsonWriter->WriteArrayStart(TEXT("historyversions"));
	{
		for (const auto& one_version : Infos.historyversions)
		{
			t_JsonWriter->WriteObjectStart();

			t_JsonWriter->WriteValue(TEXT("versionName"), one_version.Get<0>());
			t_JsonWriter->WriteValue(TEXT("md5code"), one_version.Get<1>());

			t_JsonWriter->WriteObjectEnd();
		}
	}
	t_JsonWriter->WriteArrayEnd();

	t_JsonWriter->WriteObjectEnd();

	t_JsonWriter->Close();
}

bool dataconversion::Read(const FString& Json, transfercompleteinfo& Infos)
{
	TArray<TSharedPtr<FJsonValue>> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Json);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		for (auto t_parsed : JsonParsed)
		{
			const TSharedPtr<FJsonObject> JsonObjectArr = t_parsed->AsObject();
			FOneAssetVersionInfo oneAsset;
			JsonObjectArr->TryGetStringField(TEXT("uniqueNumber"), oneAsset.uniqueNumber);
			JsonObjectArr->TryGetStringField(TEXT("companycode"), oneAsset.companycode);
			JsonObjectArr->TryGetStringField(TEXT("md5code"), oneAsset.md5code);
			JsonObjectArr->TryGetStringField(TEXT("downloadlink"), oneAsset.downloadlink);
			JsonObjectArr->TryGetNumberField(TEXT("filesize"), oneAsset.filesize);
			Infos.transfercompleteassets.Add(oneAsset);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void dataconversion::Write(const transfercompleteinfo& Infos, FString& Json)
{
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> t_JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Json);

	t_JsonWriter->WriteArrayStart();
	{
		for (const auto& oneAsset : Infos.transfercompleteassets)
		{
			t_JsonWriter->WriteObjectStart();

			t_JsonWriter->WriteValue(TEXT("uniqueNumber"), oneAsset.uniqueNumber);
			t_JsonWriter->WriteValue(TEXT("companycode"), oneAsset.companycode);
			t_JsonWriter->WriteValue(TEXT("md5code"), oneAsset.md5code);
			t_JsonWriter->WriteValue(TEXT("downloadlink"), oneAsset.downloadlink);
			t_JsonWriter->WriteValue(TEXT("filesize"), oneAsset.filesize);

			t_JsonWriter->WriteObjectEnd();
		}
	}
	t_JsonWriter->WriteArrayEnd();

	t_JsonWriter->Close();
}
