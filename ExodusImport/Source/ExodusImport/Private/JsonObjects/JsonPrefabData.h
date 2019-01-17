#pragma once
#include "JsonTypes.h"

class JsonPrefabData{
public:
	FString name;
	FString path;
	FString guid;
	FString prefabType;

	TArray<JsonGameObject> objects;

	void load(JsonObjPtr jsonData);
	JsonPrefabData() = default;
	JsonPrefabData(JsonObjPtr jsonData);
};

