#pragma once
#include "JsonTypes.h"
#include "ImportContext.h"
#include "ImportedObject.h"
#include "JsonObjects/JsonGameObject.h"

class JsonImporter;

class TerrainComponentBuilder{
public:
	static void processTerrains(
		ImportContext &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath, ImportedObjectArray *createdObjects,
		JsonImporter *importer, std::function<UObject*()> outerCreator);
	static ImportedObject processTerrain(ImportContext &workData, const JsonGameObject &jsonGameObj, 
		const JsonTerrain &jsonTerrain, ImportedObject *parentObject, const FString& folderPath, 
		JsonImporter *importer, std::function<UObject*()> outerCreator);
};