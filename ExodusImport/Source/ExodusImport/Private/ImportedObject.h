#pragma once
#include "CoreMinimal.h"
#include "JsonObjects.h"
#include "Runtime/CoreUObject/Public/UObject/StrongObjectPtr.h"

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

	bool hasActor() const{
		return actor;
	}

	bool hasComponent() const{
		return component;
	}

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

	void setNameOrLabel(const FString &newName);

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

