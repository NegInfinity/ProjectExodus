#pragma once
#include "JsonTypes.h"
#include "JsonObjects/JsonLight.h"
#include "ImportedObject.h"
#include "ImportWorkData.h"

class UPointLightComponent;
class USpotLightComponent;
class ULightComponent;
class USceneComponent;

class LightBuilder{
public:
	static void setupPointLightComponent(UPointLightComponent *pointLight, const JsonLight &jsonLight);
	static void setupSpotLightComponent(USpotLightComponent *spotLight, const JsonLight &jsonLight);
	static void setupDirLightComponent(ULightComponent *dirLight, const JsonLight &jsonLight);

	static ImportedObject processLight(ImportContext &workData, const JsonGameObject &gameObj, const JsonLight &light,
		ImportedObject *parentObject, const FString& folderPath, bool createActors);
	static void processLights(ImportContext &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath,
		ImportedObjectArray *createdObjects, bool createActors);
};
