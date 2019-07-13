#include "JsonImportPrivatePCH.h"
#include "ImportedObject.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "UnrealUtilities.h"

void ImportedObject::setCompatibleMobility(ImportedObject parentObject) const{
	if (!actor && !component)
		return;
	if (!parentObject.isValid())
		return;

	USceneComponent *myComponent = actor ? actor->GetRootComponent(): component;
	USceneComponent *parentComponent  = parentObject.actor ?parentObject.actor->GetRootComponent(): parentObject.component;

	check(myComponent && parentComponent);

	if (parentComponent->Mobility != EComponentMobility::Movable)
		return;

	UE_LOG(JsonLog, Warning, TEXT("Fixing mobility issues:\ncomponent\n%s\nparent component:\n%s\nMobility will be recursively set to moveable."), 
		*myComponent->GetFullName(), *parentComponent->GetFullName());

	auto newMobility = EComponentMobility::Movable;
	UnrealUtilities::processComponentsRecursively(myComponent, 
		nullptr, 
		[&](USceneComponent *component){
			component->SetMobility(newMobility);
		}, false
	);
}

void ImportedObject::attachTo(const ImportedObject &parent) const{
	auto parentActor = parent.actor;
	auto parentComponent = parent.component;
	check(actor || component);

	check(parentActor || parentComponent);

	setCompatibleMobility(parent);

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

		fixMismatchingOwner(true);//erm... this actually might be dangerous
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

void ImportedObject::fixMismatchingOwner(bool applyToChildren) const{
	if (!component)
		return;//nothing to do, an AActor is supposed to have a proper container by default.
	auto root = component->GetAttachmentRoot();
	if (!root)
		return;//how? should we fail in this case?
	auto rootOuter = root->GetOuter();
	auto curOuter = component->GetOuter();
	if (curOuter == rootOuter)
		return;
	if (!applyToChildren){
		component->Rename(0, rootOuter);
	}
	else{
		UnrealUtilities::changeOwnerRecursively(component, rootOuter);
	}
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

AActor* ImportedObject::findRootActor() const{
	if (component){
		return component->GetAttachmentRootActor();
	}
	return actor;
}

