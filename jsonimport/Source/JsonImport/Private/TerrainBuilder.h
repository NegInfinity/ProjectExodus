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
class UStaticMesh;

class TerrainBuilder{
protected:
	JsonImporter *importer = nullptr;
	ImportWorkData &workData;
	FString terrainDataPath;
public:
	TArray<ULandscapeGrassType*> grassTypes;
	const JsonGameObject &jsonGameObj;
	const JsonTerrain &jsonTerrain;
	const JsonTerrainData &terrainData;

	ALandscape* buildTerrain();
	TerrainBuilder(ImportWorkData &workData, JsonImporter *importer, const JsonGameObject &gameObj, const JsonTerrain &jsonTerrain, const JsonTerrainData &terrainData);

	TerrainBuilder() = delete;
	TerrainBuilder(const TerrainBuilder&) = delete;

	JsonImporter* getImporter() const{
			return importer;
	}
	const FString& getTerrainDataPath() const{
		return terrainDataPath;
	}
	//TerrainBuilder& operator=(const TerrainBuileder& other) = delete;
protected:
	ULandscapeGrassType* TerrainBuilder::createGrassType(int layerIndex, const FString &terrainDataPth);
	ULandscapeLayerInfoObject* TerrainBuilder::createTerrainLayerInfo(int layerIndex, bool grassLayer, 
		const FString &terrainDataPath);
	void processFoliageTreeActors(ALandscape *landscape);

	UStaticMesh* createBillboardMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, const FString &terrainDataPath);
};
