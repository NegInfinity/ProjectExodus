#pragma once
#include "JsonTypes.h"
#include "ImportWorkData.h"
#include "ImportedObject.h"
#include "JsonObjects/JsonGameObject.h"

class JsonImporter;

class SkeletalMeshComponentBuilder{
public:
	static void processSkinMeshes(ImportWorkData &workData, const JsonGameObject &gameObj, 
		ImportedObject *parentObject, const FString &folderPath, ImportedObjectArray *createdObjects, JsonImporter *importer);
	static ImportedObject processSkinRenderer(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		const JsonSkinRenderer &skinRend, ImportedObject *parentObject, const FString &folderPath, JsonImporter *importer);
};
