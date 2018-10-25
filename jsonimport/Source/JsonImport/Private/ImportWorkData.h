#pragma once
#include "CoreMinimal.h"

class AActor;
using IdActorMap = TMap<int, AActor*>;
using IdSet = TSet<int>;

/*
This one exists mostly to deal with the fact htat IDs are unique within SCENE, 
and within each scene they start from zero.

I kinda wonder if I should work towards ensureing ids being globally unique, but then again...
not much point when I can just use scoped dictionaries.
*/
class ImportWorkData{
public:
	IdNameMap objectFolderPaths;
	IdActorMap objectActors;
	UWorld *world;
	bool editorMode;
	bool storeActors;

	TArray<AActor*> rootActors;
	TArray<AActor*> childActors;
	TArray<AActor*> allActors;

	void registerActor(AActor* actor, AActor *parent);
	//void addRootActor(AActor* actor);

	ImportWorkData(UWorld *world_, bool editorMode_, bool storeActors_ = false)
	:world(world_), editorMode(editorMode_), storeActors(storeActors_){
	}

	void clear(){
		objectFolderPaths.Empty();
		objectActors.Empty();
	}
};

