#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"

/*
void ImportWorkData::addRootActor(AActor* actor){
	if (!storeActors)
		return;
	rootActors.Add(actor);
}
*/

void ImportWorkData::registerActor(AActor* actor, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootActors.Add(actor);
	else
		childActors.Add(actor);
	allActors.Add(actor);
}

