#pragma once
#include "JsonTypes.h"
#include "JsonObjects/JsonGameObject.h"
#include "ImportWorkData.h"

class JsonImporter;

class ReflectionProbeBuilder{
public:
	static ImportedObject processReflectionProbe(ImportWorkData &workData, const JsonGameObject &gameObj,
		const JsonReflectionProbe &probe, ImportedObject *parentObject, const FString &folderPath, JsonImporter *importer);
	static void processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString &folderPath,
		ImportedObjectArray *createdObjects, JsonImporter *importer);
};