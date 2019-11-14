#include "JsonImportPrivatePCH.h"
#include "TerrainComponentBuilder.h"
#include "UnrealUtilities.h"
#include "TerrainBuilder.h"
#include "JsonImporter.h"
#include "Landscape.h"

void TerrainComponentBuilder::processTerrains(ImportContext &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, 
		const FString& folderPath, ImportedObjectArray *createdObjects, JsonImporter *importer, std::function<UObject*()> outerCreator){
	using namespace UnrealUtilities;
	check(importer != nullptr);

	UE_LOG(JsonLog, Log, TEXT("Processing terrains for object %d"), gameObj.id);
	for(const auto& cur: gameObj.terrains){
		auto obj = processTerrain(workData, gameObj, cur, parentObject, folderPath, importer, outerCreator);
		registerImportedObject(createdObjects, obj);
	}
}

ImportedObject TerrainComponentBuilder::processTerrain(ImportContext &workData, const JsonGameObject &jsonGameObj, 
	const JsonTerrain &jsonTerrain, ImportedObject *parentObject, const FString& folderPath,
	JsonImporter *importer, std::function<UObject*()> outerCreator){
	using namespace UnrealUtilities;
	check(importer != nullptr);

	auto dataId = jsonTerrain.terrainDataId;
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain data id found: %d"), dataId);

	auto terrainData = importer->getTerrainDataMap().Find(dataId);//terrainDataMap.Find(dataId);
	if (!terrainData){
		UE_LOG(JsonLogTerrain, Warning, TEXT("Terrain data could not be found for id: %d"), dataId);
		return ImportedObject();
	}
	UE_LOG(JsonLogTerrain, Log, TEXT("Export path: \"%s\""), *(terrainData->exportPath));

	TerrainBuilder terrainBuilder(workData, importer, jsonGameObj, jsonTerrain, *terrainData);
	auto builtTerrain = terrainBuilder.buildTerrain();

	if (!builtTerrain){
		UE_LOG(JsonLogTerrain, Error, TEXT("Failed to build terrain \"%s\""), *(terrainData->exportPath));
		return ImportedObject();
	}
	builtTerrain->PostEditChange();

	//setActorHierarchy(builtTerrain, parentActor, folderPath, workData, jsonGameObj);

	//setObjectHierarchy(builtTerrain, parentObject, folderPath, workData, jsonGameObj);
	ImportedObject result(builtTerrain);
	setObjectHierarchy(result, parentObject, folderPath, workData, jsonGameObj);

	return result;
}
