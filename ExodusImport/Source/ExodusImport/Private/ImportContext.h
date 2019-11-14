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

class USceneComponent;

/*
This one exists mostly to deal with the fact that IDs are unique within SCENE, 
and within each scene they start from zero.

I kinda wonder if I should work towards ensureing ids being globally unique, but then again...
not much point when I can just use scoped dictionaries.
*/
class ImportContext{
protected:
	mutable uint64 uniqueInt = 0;
public:
	uint64 getUniqueUint() const;
	//const JsonScene *srcScene = nullptr;
	const TArray<JsonGameObject> *srcObjects = nullptr;

	IdNameMap objectFolderPaths;

	ImportedObjectMap importedObjects;
	TStrongObjectPtr<UWorld> world;
	bool editorMode;

	TArray<AnimControllerIdKey> delayedAnimControllers;
	TArray<JsonId> postProcessAnimatorObjects;

	UObject* findSuitableOuter(const JsonGameObject &jsonObj) const;

	//void changeOwnerRecursively(USceneComponent *rootComponent, UObject *newOwner) const;

	const ImportedObject* findImportedObject(JsonId id) const;
	ImportedObject* findImportedObject(JsonId id);
	FString processFolderPath(const JsonGameObject &jsonObj);

	const FString* findFolderPath(JsonId id) const;
	void registerGameObject(const JsonGameObject &gameObj, ImportedObject imported);

	const JsonGameObject* findJsonObject(JsonId id) const;

	bool isCompoundRigidbodyRootCollider(const JsonGameObject &gameObj) const;

	const JsonRigidbody* locateRigidbody(const JsonGameObject &gameObj) const;

	void registerAnimatorForPostProcessing(const JsonGameObject &jsonObj);
	void registerDelayedAnimController(JsonId skelId, JsonId controllerId);

	void clear();

	AActor* createBlankActor(const JsonGameObject &gameObj, bool createMissingRootComponent) const;
	ImportedObject createBlankActor(const JsonGameObject &gameObj, USceneComponent *rootComponent = nullptr, 
		bool changeOwnership = false, bool createMissingRootComponeint = true) const;
	ImportedObject createBlankNode(const JsonGameObject &gameObj, bool createActor, bool createMissingRootComponent, std::function<UObject*()> outerGetter) const;

	ImportContext(UWorld *world_, bool editorMode_, const JsonScene *scene_);
	ImportContext(UWorld *world_, bool editorMode_, const TArray<JsonGameObject> *objects_);
};
