#pragma once
#include "JsonTypes.h"

class JsonTerrainDetailPrototype{
public:
	int textureId;
	int detailPrefabId;
	int detailPrefabObjectId;

	int detailMeshId;
	IntArray detailMeshMaterials;

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

	bool billboardFlag = false;
	bool vertexLitFlag = false;
	bool grassFlag = false;

	bool isBillboard()const {return billboardFlag;}
	bool isVertexLit()const {return vertexLitFlag;}
	bool isGrassFlag()const {return grassFlag;}

	JsonTerrainDetailPrototype() = default;
	JsonTerrainDetailPrototype(JsonObjPtr data);
	void load(JsonObjPtr data);
};
