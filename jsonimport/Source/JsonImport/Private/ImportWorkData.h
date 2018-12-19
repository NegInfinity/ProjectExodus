#pragma once
#include "CoreMinimal.h"

class AActor;
using IdActorMap = TMap<int, AActor*>;
using IdSet = TSet<int>;

class USceneComponent;

/*
This structure holds "spawned game object" information.
A game object can be spawned as a component or as an actor.  
(currently they're being spawned as actors in most cases)

This is represented in this structure. If "component" is set, "actor" is irrelevant or nullptr.
*/
class ImportedGameObject{
public:
	AActor *actor = nullptr;
	USceneComponent *component = nullptr;

	void setActiveInHierarchy(bool active) const;
	void setFolderPath(const FString &folderPath) const;

	void attachTo(AActor *actor, USceneComponent *component) const;
	void attachTo(ImportedGameObject *parent) const;

	ImportedGameObject(AActor *actor_)
	:actor(actor_), component(nullptr){}

	ImportedGameObject(USceneComponent *component_)
	:actor(nullptr), component(component_){}

	ImportedGameObject(AActor *actor_, USceneComponent *component_)
	:actor(actor_), component(component_){}
};

using ImportedObjectMap = TMap<JsonId, ImportedGameObject>;

/*
This one exists mostly to deal with the fact htat IDs are unique within SCENE, 
and within each scene they start from zero.

I kinda wonder if I should work towards ensureing ids being globally unique, but then again...
not much point when I can just use scoped dictionaries.
*/
class ImportWorkData{
public:
	IdNameMap objectFolderPaths;

	ImportedObjectMap importedObjects;
	//IdActorMap objectActors;
	UWorld *world;
	bool editorMode;
	bool storeActors;

	TArray<ImportedGameObject> rootObjects;
	TArray<ImportedGameObject> childObjects;
	TArray<ImportedGameObject> allObjects;

	void registerObject(const ImportedGameObject &object, AActor *parent);
	/*
	TArray<AActor*> rootActors;
	TArray<AActor*> childActors;
	TArray<AActor*> allActors;

	void registerActor(AActor* actor, AActor *parent);
	*/

	ImportWorkData(UWorld *world_, bool editorMode_, bool storeActors_ = false)
	:world(world_), editorMode(editorMode_), storeActors(storeActors_){
	}

	void clear(){
		objectFolderPaths.Empty();
		//objectActors.Empty();
		importedObjects.Empty();
	}
};

