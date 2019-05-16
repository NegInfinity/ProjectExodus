#pragma once
#include "JsonTypes.h"
#include "JsonGameObject.h"

class JsonPrefabData{
public:
	FString name;
	FString path;
	FString guid;
	FString prefabType;

	TArray<JsonGameObject> objects;

	bool hasObjects() const{
		return objects.Num() > 0;
	}

	void load(JsonObjPtr jsonData);
	JsonPrefabData() = default;
	JsonPrefabData(JsonObjPtr jsonData);
};

