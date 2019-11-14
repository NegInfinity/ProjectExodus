#pragma once
#include "JsonTypes.h"
#include "JsonObjects/JsonGameObject.h"
#include "ImportContext.h"

#include <functional>

class JsonImporter;

class ReflectionProbeBuilder{
public:
	static ImportedObject processReflectionProbe(ImportContext &workData, const JsonGameObject &gameObj,
		const JsonReflectionProbe &probe, ImportedObject *parentObject, const FString &folderPath, JsonImporter *importer, std::function<UObject*()> outerCallback);
	static void processReflectionProbes(ImportContext &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString &folderPath,
		ImportedObjectArray *createdObjects, JsonImporter *importer, std::function<UObject*()> outerCallback);
};