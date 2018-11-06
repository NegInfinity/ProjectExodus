#pragma once
#include "JsonTypes.h"

#include "JsonBounds.h"
#include "JsonTerrainDetailPrototype.h"
#include "JsonTreePrototype.h"
#include "JsonTreeInstance.h"
#include "JsonSplatPrototype.h"

class JsonTerrainData{
public:
	FString name;
	FString path;
	FString exportPath;
	int32 alphaMapWidth = 0;
	int32 alphaMapHeight = 0;
	int32 alphaMapLayers = 0;
	int32 alphaMapResolution = 0;

	FString heightMapRawPath;
	TArray<FString> alphaMapRawPaths;
	TArray<FString> detailMapRawPaths;
		
	int32 baseMapResolution = 0;
	JsonBounds bounds;
	//writer.writeKeyVal("bounds", bounds);

	int32 detailWidth = 0;
	int32 detailHeight = 0;

	TArray<JsonTerrainDetailPrototype> detailPrototypes;//terrainDetails --> detailPrototypes
	int32 detailResolution = 0;

	int32 heightmapWidth = 0;
	int32 heightmapHeight = 0;
	int32 heightmapResolution = 0;
	FVector heightmapScale;
		
	FVector worldSize;
	float thickness = 0.0f;

	int32 treeInstanceCount = 0;

	//splatPrototypes --> splat prototypes
	TArray<JsonSplatPrototype> splatPrototypes;
	//treeInstances --> tree instances
	TArray<JsonTreeInstance> treeInstances;
	//treePrototypes --> treePrototypes);
	TArray<JsonTreePrototype> treePrototypes;

	float wavingGrassAmount = 0.0f;
	float wavingGrassSpeed = 0.0f;
	float wavingGrassStrength = 0.0f;

	FLinearColor wavingGrassTint;

	FVector getNormalizedPosAsWorld(const FVector &normalizedUnityCoord, const FVector &origin) const;

	FString getLayerName(int layerIndex) const;

	JsonTerrainData() = default;
	JsonTerrainData(JsonObjPtr data);
	void load(JsonObjPtr data);
};
