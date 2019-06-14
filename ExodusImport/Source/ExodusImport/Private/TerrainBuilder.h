#pragma once

#include "JsonTypes.h"
#include "JsonObjects/JsonGameObject.h"
#include "JsonObjects/JsonTerrain.h"
#include "JsonObjects/JsonTerrainData.h"
#include "ImportContext.h"

class JsonImporter;
class ALandscape;
class ULandscapeGrassType;
class ULandscapeLayerInfoObject;
class UStaticMesh;
class UMaterialInstanceConstant;

class TerrainBuilder{
protected:
	JsonImporter *importer = nullptr;
	ImportContext &workData;
	//FString terrainDataPath;
public:
	TArray<ULandscapeGrassType*> grassTypes;
	const JsonGameObject &jsonGameObj;
	const JsonTerrain &jsonTerrain;
	const JsonTerrainData &terrainData;

	ALandscape* buildTerrain();
	TerrainBuilder(ImportContext &workData, JsonImporter *importer, const JsonGameObject &gameObj, const JsonTerrain &jsonTerrain, const JsonTerrainData &terrainData);

	TerrainBuilder() = delete;
	TerrainBuilder(const TerrainBuilder&) = delete;

	JsonImporter* getImporter() const{
			return importer;
	}
	/*
	const FString& getTerrainDataPath() const{
		return terrainDataPath;
	}
	*/
	//TerrainBuilder& operator=(const TerrainBuileder& other) = delete;
protected:
	UStaticMesh* createClonedMesh(const JsonMesh &jsonMesh, const FString &baseName, const FString &terrainDataPath, 
		const IntArray &matIds, 
		std::function<void(UMaterialInstanceConstant *matInst, FStaticParameterSet &statParams)> matInstCallback
	);

	ULandscapeGrassType* createGrassType(int layerIndex, const FString &terrainDataPth);
	ULandscapeLayerInfoObject* createTerrainLayerInfo(int layerIndex, bool grassLayer, 
		const FString &terrainDataPath);
	void processFoliageTreeActors(ALandscape *landscape);

	UStaticMesh* createBillboardMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, int layerIndex, const FString &terrainDataPath);
	UStaticMesh* createGrassMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, int layerIndex, const FString &terrainDataPath);
	//UStaticMesh* TerrainBuilder::createTreeMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, int layerIndex, const FString &terrainDataPath);
};
