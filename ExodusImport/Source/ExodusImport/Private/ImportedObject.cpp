#include "JsonImportPrivatePCH.h"
#include "ImportedObject.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

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
