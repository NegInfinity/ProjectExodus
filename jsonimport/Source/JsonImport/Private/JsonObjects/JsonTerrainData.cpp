#include "JsonImportPrivatePCH.h"
#include "JsonTerrainData.h"
#include "macros.h"
#include "utilities.h"

using namespace JsonObjects;

JsonTerrainData::JsonTerrainData(JsonObjPtr data){
	load(data);
}

void JsonTerrainData::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, exportPath);
	JSON_GET_VAR(data, alphaMapWidth);
	JSON_GET_VAR(data, alphaMapHeight);
	JSON_GET_VAR(data, alphaMapLayers);
	JSON_GET_VAR(data, alphaMapResolution);
		
	JSON_GET_VAR(data, baseMapResolution);
	//JSON_GET_VAR(data, bounds;
	//writer.writeKeyVal("bounds", bounds);
	getJsonObj(data, bounds, "bounds");

	JSON_GET_VAR(data, detailWidth);
	JSON_GET_VAR(data, detailHeight);

	JSON_GET_VAR(data, heightMapRawPath);
	JSON_GET_VAR(data, alphaMapRawPaths);
	JSON_GET_VAR(data, detailMapRawPaths);


	//TArray<JsonTerrainDextailPrototype> detailPrototypes;//terrainDetails --> detailPrototypes
	getJsonObjArray(data, detailPrototypes, "detailPrototypes");
	JSON_GET_VAR(data, detailResolution);

	JSON_GET_VAR(data, heightmapWidth);
	JSON_GET_VAR(data, heightmapHeight);
	JSON_GET_VAR(data, heightmapResolution);
	JSON_GET_VAR(data, heightmapScale);
		
	JSON_GET_VAR(data, worldSize);
	JSON_GET_VAR(data, thickness);

	JSON_GET_VAR(data, treeInstanceCount);

	//splatPrototypes --> splat prototypes
	getJsonObjArray(data, splatPrototypes, "splatPrototypes");
	//treeInstances --> tree instances
	getJsonObjArray(data, treeInstances , "treeInstances");
	//treePrototypes --> treePrototypes);
	getJsonObjArray(data, treePrototypes, "treePrototypes");

	JSON_GET_VAR(data, wavingGrassAmount);
	JSON_GET_VAR(data, wavingGrassSpeed);
	JSON_GET_VAR(data, wavingGrassStrength);

	JSON_GET_VAR(data, wavingGrassTint);
}

FString JsonTerrainData::getLayerName(int layerIndex) const{
	return FString::Printf(TEXT("Layer%d"), layerIndex);
}

FString JsonTerrainData::getGrassLayerName(int layerIndex) const{
	return FString::Printf(TEXT("GrassLayer%d"), layerIndex);
}

FString JsonTerrainData::getGrassTypeName(int layerIndex) const{
	return FString::Printf(TEXT("GrassType%d"), layerIndex);
}

FVector JsonTerrainData::getNormalizedPosAsWorld(const FVector& normalizedUnityCoord, const FVector &origin) const{
	auto ueSize = unitySizeToUe(worldSize);
	auto ueCoord = unityVecToUe(normalizedUnityCoord);
	//ueCoord.X = ueCoord.X * 0.5f - 0.5f;
	//ueCoord.Y = ueCoord.Y * 0.5f - 0.5f;

	auto result = origin + 
		FVector(
			ueCoord.X * ueSize.X,
			ueCoord.Y * ueSize.Y,
			ueCoord.Z * ueSize.Z
		);
	return result;
}
