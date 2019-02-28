#pragma once
#include "CoreMinimal.h"
#include "JsonObjects.h"
#include "Runtime/CoreUObject/Public/UObject/StrongObjectPtr.h"
#include "ImportedObject.h"

class AActor;
using IdActorMap = TMap<int, AActor*>;
using IdSet = TSet<int>;
using IdPair = TPair<JsonId, JsonId>;

using AnimClipIdKey = TPair<JsonId, JsonId>;
using AnimControllerIdKey = TPair<JsonId, JsonId>;

using AnimClipPathMap = TMap<AnimClipIdKey, FString>;
using AnimControllerPathMap = TMap<AnimControllerIdKey, FString>;

AnimClipIdKey makeAnimClipKey(JsonId skeletonId, JsonId clipId){
	return AnimClipIdKey(skeletonId, clipId);
}

AnimClipIdKey makeAnimControlKey(JsonId skeletonId, JsonId controllerId){
	return AnimControllerIdKey(skeletonId, controllerId);
}

class USceneComponent;

/*
This one exists mostly to deal with the fact that IDs are unique within SCENE, 
and within each scene they start from zero.

I kinda wonder if I should work towards ensureing ids being globally unique, but then again...
not much point when I can just use scoped dictionaries.
*/
class ImportWorkData{
public:
	const JsonScene *scene = nullptr;

	IdNameMap objectFolderPaths;

	ImportedObjectMap importedObjects;
	TStrongObjectPtr<UWorld> world;
	bool editorMode;
	bool storeActors;

	TArray<ImportedObject> rootObjects;
	TArray<ImportedObject> childObjects;
	TArray<ImportedObject> allObjects;

	TArray<AnimControllerIdKey> delayedAnimControllers;

	TArray<JsonId> postProcessAnimatorObjects;

	const JsonGameObject* findJsonObject(JsonId id) const;

	const JsonRigidbody* locateRigidbody(const JsonGameObject &gameObj) const;

	void registerAnimatorForPostProcessing(const JsonGameObject &jsonObj);
	void registerDelayedAnimController(JsonId skelId, JsonId controllerId);

	void registerObject(const ImportedObject &object, AActor *parent);

	ImportWorkData(UWorld *world_, bool editorMode_, const JsonScene *scene_, bool storeActors_ = false)
	:world(world_), editorMode(editorMode_), scene(scene_), storeActors(storeActors_){
	}

	void clear(){
		objectFolderPaths.Empty();
		importedObjects.Empty();

		delayedAnimControllers.Empty();
	}
};

