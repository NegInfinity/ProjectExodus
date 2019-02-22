#pragma once
#include "CoreMinimal.h"
#include "JsonObjects.h"
#include "Runtime/CoreUObject/Public/UObject/StrongObjectPtr.h"

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
This structure holds "spawned game object" information.
A game object can be spawned as a component or as an actor.  
(currently they're being spawned as actors in most cases)

This is represented in this structure. If "component" is set, "actor" is irrelevant or nullptr.
*/
class ImportedObject{
public:
	AActor *actor = nullptr;
	USceneComponent *component = nullptr;

	AActor *findRootActor() const{
		if (component){
			return component->GetAttachmentRootActor();
		}
		return actor;
	}

	bool isValid() const{
		return actor || component;
	}

	UObject* getPtrForOuter() const{
		if (component)
			return component;
		return actor;
	}

	void setActiveInHierarchy(bool active) const;
	void setFolderPath(const FString &folderPath) const;

	void attachTo(AActor *actor, USceneComponent *component) const;
	void attachTo(ImportedObject *parent) const;

	ImportedObject() = default;

	ImportedObject(AActor *actor_)
	:actor(actor_), component(nullptr){}

	ImportedObject(USceneComponent *component_)
	:actor(nullptr), component(component_){}

	ImportedObject(AActor *actor_, USceneComponent *component_)
	:actor(actor_), component(component_){}
};

using ImportedObjectMap = TMap<JsonId, ImportedObject>;
using ImportedObjectArray = TArray<ImportedObject>;

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

