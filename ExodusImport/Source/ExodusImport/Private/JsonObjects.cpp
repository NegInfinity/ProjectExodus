#include "JsonImportPrivatePCH.h"
#include "JsonObjects.h"
#include "JsonLog.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

using namespace JsonObjects;

JsonObjPtr JsonObjects::loadJsonFromFile(const FString &filename){
	FString jsonString;
	if (!FFileHelper::LoadFileToString(jsonString, *filename)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load json file \"%s\""), *filename);
		return 0;
	}

	UE_LOG(JsonLog, Log, TEXT("Loaded json file \"%s\""), *filename);
	JsonReaderRef reader = TJsonReaderFactory<>::Create(jsonString);

	JsonObjPtr jsonData = MakeShareable(new FJsonObject());
	if (!FJsonSerializer::Deserialize(reader, jsonData)){
		UE_LOG(JsonLog, Warning, TEXT("Could not parse json file \"%s\""), *filename);
		return 0;
	}
	return jsonData;
}

#undef LOCTEXT_NAMESPACE
