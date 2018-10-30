#pragma once
#include "JsonTypes.h"

class JsonTerrainDetailPrototype{
public:
	int textureId;
	int detailPrefabId;
	int detailPrefabObjectId;

	float bendFactor;
	FLinearColor dryColor;
	FLinearColor healthyColor;
	float minWidth;
	float minHeight;
	float maxWidth;
	float maxHeight;
	float noiseSpread;
	FString renderMode;
	bool usePrototypeMesh;

	JsonTerrainDetailPrototype() = default;
	JsonTerrainDetailPrototype(JsonObjPtr data);
	void load(JsonObjPtr data);
};
