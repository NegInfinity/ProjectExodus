#pragma once

#include "JsonTypes.h"
#include "JsonObjects/JsonGameObject.h"
#include "JsonObjects/JsonTerrain.h"
#include "JsonObjects/JsonTerrainData.h"
#include "ImportWorkData.h"

class JsonImporter;
class ALandscape;
class ULandscapeGrassType;
class ULandscapeLayerInfoObject;

class TerrainBuilder{
public:
	ALandscape* buildTerrain();
	TerrainBuilder(ImportWorkData &workData, JsonImporter *importer, const JsonGameObject &gameObj, const JsonTerrain &jsonTerrain, const JsonTerrainData &terrainData);

	TerrainBuilder() = delete;
	TerrainBuilder(const TerrainBuilder&) = delete;
	//TerrainBuilder& operator=(const TerrainBuileder& other) = delete;
protected:
	JsonImporter *importer = nullptr;
	ImportWorkData &workData;
	const JsonGameObject &jsonGameObj;
	const JsonTerrain &jsonTerrain;
	const JsonTerrainData &terrainData;
	FString terrainDataPath;

	ULandscapeGrassType* TerrainBuilder::createGrassType(int layerIndex, const FString &terrainDataPth);
	ULandscapeLayerInfoObject* TerrainBuilder::createTerrainLayerInfo(int layerIndex, bool grassLayer, 
		const FString &terrainDataPath);
	void processFoliageTreeActors(ALandscape *landscape);
};
