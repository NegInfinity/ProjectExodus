#pragma once
#include "JsonObjects.h"

class JsonScene{
public:
	FString name;
	FString path;
	int buildIndex = -1;
	TArray<JsonGameObject> objects;

	bool containsTerrain() const;

	void load(JsonObjPtr data);
	JsonScene() = default;
	JsonScene(JsonObjPtr data){
		load(data);
	}
};
