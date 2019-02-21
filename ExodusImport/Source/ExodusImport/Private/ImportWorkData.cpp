#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

void ImportWorkData::registerObject(const ImportedObject &object, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootObjects.Add(object);
	else
		childObjects.Add(object);
	allObjects.Add(object);
}

void ImportedObject::attachTo(AActor *parentActor, USceneComponent *parentComponent) const{
	check(actor || component);

	check(parentActor || parentComponent);

	if (component){
		UE_LOG(JsonLog, Log, TEXT("Attaching component %s"), *component->GetName());
		if (parentComponent){
			UE_LOG(JsonLog, Log, TEXT("Attaching to parent component %s"), *parentComponent->GetName());
			component->AttachToComponent(parentComponent, FAttachmentTransformRules::KeepWorldTransform);
		}		
		else{
			if (parentActor){
				UE_LOG(JsonLog, Log, TEXT("Attaching to parent actor %s"), *parentActor->GetName());
				component->AttachToComponent(parentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
			else{
				UE_LOG(JsonLog, Log, TEXT("Could not attach"));
				//check(false);
			}
		}
	}
	else if (actor){
		UE_LOG(JsonLog, Log, TEXT("Attaching an actor %s"), *actor->GetName());
		if (parentComponent){
			UE_LOG(JsonLog, Log, TEXT("Attaching to parent component %s"), *parentComponent->GetName());
			actor->AttachToComponent(parentComponent, FAttachmentTransformRules::KeepWorldTransform);
		}
		else {
			if (parentActor){
				UE_LOG(JsonLog, Log, TEXT("Attaching to parent actor %s"), *parentActor->GetName());
				actor->AttachToActor(parentActor, FAttachmentTransformRules::KeepWorldTransform);
			}
			else{
				UE_LOG(JsonLog, Log, TEXT("Could not attach"));
				//check(false);
			}
		}
	}
}

void ImportedObject::attachTo(ImportedObject *parent) const{
	check(parent);
	attachTo(parent->actor, parent->component);
}

void ImportedObject::setActiveInHierarchy(bool active) const{
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

void ImportedObject::setFolderPath(const FString &folderPath) const{
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

const JsonGameObject* ImportWorkData::findJsonObject(JsonId id) const{
	check(scene != nullptr);
	if ((id >= 0) && (id < scene->objects.Num()))
		return &scene->objects[id];

	return nullptr;
}

const JsonRigidbody* ImportWorkData::locateRigidbody(const JsonGameObject &srcGameObj) const{
	using namespace JsonObjects;

	const JsonGameObject *currentObject = &srcGameObj;
	while (currentObject){
		if (currentObject->rigidbodies.Num() > 0){
			return &currentObject->rigidbodies[0];
		}
		if (!isValidId(currentObject->parentId))
			break;

		if (!scene)
			break;

		currentObject = findJsonObject(currentObject->parentId);
	}
	return nullptr;
}

