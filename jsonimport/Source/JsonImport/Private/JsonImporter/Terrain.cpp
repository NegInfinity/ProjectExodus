#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "JsonObjects/JsonBinaryTerrain.h"

#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"

#include "JsonObjects/DataPlane2D.h"
#include "JsonObjects/DataPlane3D.h"
#include "JsonObjects/utilities.h"
#include "ImportWorkData.h"
#include "UnrealUtilities.h"

#include "TerrainBuilder.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

void JsonImporter::processTerrains(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedGameObject *parentObject, const FString& folderPath){
	UE_LOG(JsonLog, Log, TEXT("Processing terrains for object %d"), gameObj.id);
	for(const auto& cur: gameObj.terrains){
		//processTerrain(workData, gameObj, cur, parentActor, folderPath);
		processTerrain(workData, gameObj, cur, parentObject, folderPath);
	}
}

/*void JsonImporter::processTerrain(ImportWorkData &workData, const JsonGameObject &jsonGameObj, const JsonTerrain &jsonTerrain, 
		AActor *parentActor, const FString& folderPath){*/
void JsonImporter::processTerrain(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		const JsonTerrain &jsonTerrain, ImportedGameObject *parentObject, const FString& folderPath){
	auto dataId = jsonTerrain.terrainDataId;
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain data id found: %d"), dataId);

	auto terrainData = terrainDataMap.Find(dataId);
	if (!terrainData){
		UE_LOG(JsonLogTerrain, Warning, TEXT("Terrain data could not be found for id: %d"), dataId);
		return;
	}
	UE_LOG(JsonLogTerrain, Log, TEXT("Export path: \"%s\""), *(terrainData->exportPath));

	TerrainBuilder terrainBuilder(workData, this, jsonGameObj, jsonTerrain, *terrainData);
	auto builtTerrain = terrainBuilder.buildTerrain();

	if (!builtTerrain){
		UE_LOG(JsonLogTerrain, Error, TEXT("Failed to build terrain \"%s\""), *(terrainData->exportPath));
		return;
	}
	builtTerrain->PostEditChange();

	//setActorHierarchy(builtTerrain, parentActor, folderPath, workData, jsonGameObj);

	//setObjectHierarchy(builtTerrain, parentObject, folderPath, workData, jsonGameObj);
	setObjectHierarchy(ImportedGameObject(builtTerrain), parentObject, folderPath, workData, jsonGameObj);


	//return builtTerrain;
}
