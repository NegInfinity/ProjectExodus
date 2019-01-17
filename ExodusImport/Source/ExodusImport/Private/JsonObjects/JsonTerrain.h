#pragma once
#include "JsonTypes.h"

class JsonTerrain{
public:
	bool castShadows;
	float detailObjectDensity;
	float detailObjectDistance;
	bool drawHeightmap;
	bool drawTreesAndFoliage;
		
	bool renderHeightmap;
	bool renderTrees;
	bool renderDetails;
		
	float heightmapPixelError;
		
	float legacyShininess;
	//Color legacySpecular;
	FLinearColor legacySpecular;
	int lightmapIndex;
	FVector4 lightmapScaleOffet;

	int materialTemplateIndex = -1;
	FString materialType;		
		
	FVector patchBoundsMultiplier;
	bool preserveTreePrototypeLayers;
	int realtimeLightmapIndex;
	FVector4 realtimeLightmapScaleOffset;
		
	int terrainDataId = -1;
		
	float treeBillboardDistance;
	float treeCrossFadeLength;
		
	float treeDistance;
	float treeLodBiasMultiplier;
		
	int treeMaximumFullLODCount;

	JsonTerrain() = default;
	JsonTerrain(JsonObjPtr data);
	void load(JsonObjPtr data);
};

