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

		fixMismatchingOwner();
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

void ImportedObject::setFolderPath(const FString &folderPath, bool recursive) const{
	if (!actor)
		return;
	if (recursive)
		actor->SetFolderPath_Recursively(*folderPath);
	else
		actor->SetFolderPath(*folderPath);
}

void ImportedObject::setNameOrLabel(const FString &newName, bool markDirty){
	if (actor)
		actor->SetActorLabel(newName, markDirty);
	if (component)
		component->Rename(*newName);
}

void ImportedObject::fixMismatchingOwner() const{
	if (!component)
		return;//nothing to do, an AActor is supposed to have a proper container by default.
	auto root = component->GetAttachmentRoot();
	if (!root)
		return;//how? should we fail in this case?
	auto rootOuter = root->GetOuter();
	auto curOuter = component->GetOuter();
	if (curOuter != rootOuter)
		component->Rename(0, rootOuter);
}

void ImportedObject::fixEditorVisibility() const{
	if (!component)
		return;
	auto rootActor = component->GetAttachmentRootActor();
	check(rootActor);
	rootActor->AddInstanceComponent(component);
}

void ImportedObject::convertToInstanceComponent() const{
	if (!component)
		return;
	auto rootActor = component->GetAttachmentRootActor();
	check(rootActor);
	component->bEditableWhenInherited = true;
	component->RegisterComponent();
}
