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
public:
	AActor* getActor() const{
		return actor;
	}

	USceneComponent* getComponent() const{
		return component;
	}

	void setCompatibleMobility(ImportedObject parentObject) const;
	bool hasActor() const{
		return actor != nullptr;//goddamn msvc warning...
	}

	bool hasComponent() const{
		return component != nullptr;
	}

	AActor *findRootActor() const;

	bool isValid() const{
		return actor || component;
	}

	bool operator==(const ImportedObject &other) const{
		return (actor == other.actor) && (component == other.component);
	}

	bool operator!=(const ImportedObject &other) const{
		return !(*this == other);
	}

	/*
	Fixes object ownership in case of complex reattachment scenarios
	*/
	void fixMismatchingOwner(bool applyToChidlren) const;//Erm... const modifier.

	/*
	* Component visibility will be adjusted only if adjustComponents is set to true.
	* This is added to deal with complex situations where component visibility does not match object visibility,
	* such as non-renderable mesh collider.
	*/
	void setActiveInHierarchy(bool active) const;
	void setFolderPath(const FString &folderPath, bool recursive = false) const;

	void attachTo(const ImportedObject &parent) const;

	void setNameOrLabel(const FString &newName, bool markDirty = true);

	void fixEditorVisibility() const;
	void convertToInstanceComponent() const;

	ImportedObject() = default;

	ImportedObject(AActor *actor_)
		:actor(actor_), component(nullptr){}

	ImportedObject(USceneComponent *component_)
		:actor(nullptr), component(component_){}
};

using ImportedObjectMap = TMap<JsonId, ImportedObject>;
using ImportedObjectArray = TArray<ImportedObject>;

