#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

/*
void ImportWorkData::addRootActor(AActor* actor){
	if (!storeActors)
		return;
	rootActors.Add(actor);
}
*/

void ImportWorkData::registerObject(const ImportedGameObject &object, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootObjects.Add(object);
	else
		childObjects.Add(object);
	allObjects.Add(object);
}

/*
void ImportWorkData::registerActor(AActor* actor, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootActors.Add(actor);
	else
		childActors.Add(actor);
	allActors.Add(actor);
}
*/

void ImportedGameObject::attachTo(AActor *parentActor, USceneComponent *parentComponent) const{
	check(actor || component);

	check(parentActor || parentComponent);

	if (component){
		if (parentComponent){
			component->AttachToComponent(parentComponent, FAttachmentTransformRules::KeepWorldTransform);
		}		
		else{
			if (parentActor){
				component->AttachToComponent(parentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
			else{
				//check(false);
			}
		}
	}
	else if (actor){
		if (parentComponent){
			actor->AttachToComponent(parentComponent, FAttachmentTransformRules::KeepWorldTransform);
		}
		else {
			if (parentActor){
				actor->AttachToActor(parentActor, FAttachmentTransformRules::KeepWorldTransform);
			}
			else{
				//check(false);
			}
		}
	}
}

void ImportedGameObject::attachTo(ImportedGameObject *parent) const{
	check(parent);
	attachTo(parent->actor, parent->component);
}

void ImportedGameObject::setActiveInHierarchy(bool active) const{
	if (!actor)
		return;
	if (component){
		component->bVisible = active;
		///Collisions?
	}
	else{
		auto root = actor->GetRootComponent();
		if (root)
			root->bVisible = active;
		actor->SetActorHiddenInGame(!active);
		actor->SetActorEnableCollision(active);
		actor->SetActorTickEnabled(active);
	}
}

void ImportedGameObject::setFolderPath(const FString &folderPath) const{
	if (!actor)
		return;
	actor->SetFolderPath(*folderPath);
}

void ImportWorkData::registerDelayedAnimController(JsonId skelId, JsonId controllerId){
	delayedAnimControllers.Add(AnimControllerIdKey(skelId, controllerId));
}

void ImportWorkData::registerAnimatorForPostProcessing(const JsonGameObject &jsonObj){
	if (!postProcessAnimatorObjects.Contains(jsonObj.id))
		postProcessAnimatorObjects.Add(jsonObj.id);
}
