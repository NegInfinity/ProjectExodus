#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#include "JsonObjects.h"
#include "UnrealUtilities.h"

#include "Classes/Engine/SkeletalMesh.h"

#include "builders/LightBuilder.h"
#include "builders/ReflectionProbeBuilder.h"
#include "builders/TerrainComponentBuilder.h"
#include "builders/SkeletalMeshComponentBuilder.h"
#include "builders/GeometryComponentBuilder.h"

#include "DesktopPlatformModule.h"

ImportedObject JsonImporter::importObject(const JsonGameObject &jsonGameObj, ImportWorkData &workData, bool createEmptyTransforms){
	using namespace UnrealUtilities;

	auto* parentObject = workData.findImportedObject(jsonGameObj.parentId);

	auto folderPath = workData.processFolderPath(jsonGameObj);
	UE_LOG(JsonLog, Log, TEXT("Num components for object %d(%s): %d"), jsonGameObj.id, *folderPath, jsonGameObj.getNumComponents());

	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return ImportedObject(); 
	}

	ImportedObjectArray createdObjects;

	auto objectType = DesiredObjectType::Default;
	bool createActorNodes = true;

	//In situation where there's no parent, we have to create an actor. Otherwise we will have no valid outer
	createActorNodes = createActorNodes || !parentObject;

	/*
	Here we handle creation of display geometry and colliders. This particular function call harvests colliders, reigidbody properties, builds them into a somewhat sensible hierarchy,
	and returns root object to us
	*/
	ImportedObject rootObject = GeometryComponentBuilder::processMeshAndColliders(workData, jsonGameObj, jsonGameObj.id, parentObject, folderPath, 
		createActorNodes ? DesiredObjectType::Actor: objectType, this);
	if (rootObject.isValid()){
		createdObjects.Add(rootObject);
	}

	/*
	The block below walks through every component type we currently support, and spawns unreal-side representation.
	*/
	if (jsonGameObj.hasProbes()){
		ReflectionProbeBuilder::processReflectionProbes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this);
	}
	
	if (jsonGameObj.hasLights()){
		LightBuilder::processLights(workData, jsonGameObj, parentObject, folderPath, &createdObjects);
	}

	if (jsonGameObj.hasTerrain()){
		TerrainComponentBuilder::processTerrains(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this);
	}

	if (jsonGameObj.hasSkinMeshes()){
		SkeletalMeshComponentBuilder::processSkinMeshes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this);
	}

	if (createdObjects.Num() > 1){
		if (!rootObject.isValid()){
			//More than one object exists, however, in case this is a mesh node, then the mesh actor is registered as a root by default.
			rootObject = workData.createBlankActor(jsonGameObj);
			check(rootObject.isValid());
		}
	}
	else if (createdObjects.Num() == 1){
		//Oh. There  is a bug. In case we have a mesh with a single component parented to it, the check wil fail.
		//check(!rootObject.isValid());
		rootObject = createdObjects[0];
	}
	else if (createdObjects.Num() == 0){
		//No objects has been created. In this scenario, we do not make any nodes, unless requested.
		if (createEmptyTransforms){
			rootObject = workData.createBlankNode(jsonGameObj, createActorNodes);
				//workData.createBlankActor(jsonGameObj);
			check(rootObject.isValid());
		}
	}
	else{
		check(false);//This shouldn't happen, buuut....
	}

	if (rootObject.isValid()){
		for (auto& cur : createdObjects){
			if (!cur.isValid())
				continue;
			if (cur == rootObject)
				continue;
			/*
			And then we parent created objects to the root object. The objects returned by previous methods do not form a hierarchy, 
			and are all treated as "sibling" nodes.
			*/
			setObjectHierarchy(cur, &rootObject, folderPath, workData, jsonGameObj);
		}

		workData.registerGameObject(jsonGameObj, rootObject);
		setObjectHierarchy(rootObject, parentObject, folderPath, workData, jsonGameObj);
		rootObject.setFolderPath(folderPath, true);
	}
#if 0
	/*
	Previous calls created more than one object, and as a result, we now need to spawn an object that will serve as a root for everything else.

	Currently it is going to be an AActor, and in future I'd likely need to deal with situation where it should be an USceneComponent
	*/
	if ((createdObjects.Num() > 1) && (!rootObject.isValid())){
		rootObject = workData.createBlankActor(jsonGameObj);
	}

	/*
	Processing root object...
	*/
	if (rootObject.isValid()){
		for (auto& cur : createdObjects){
			if (!cur.isValid())
				continue;

			/*
			And then we parent created objects to the root object. The objects returned by previous methods do not form a hierarchy, 
			and are all treated as "sibling" nodes.
			*/
			setObjectHierarchy(cur, &rootObject, folderPath, workData, jsonGameObj);
		}
	}
	else{
		/*
		Verifying existence of root object. The object may or may not have been created.
		*/
		if (createdObjects.Num() == 1){
			//Only one object exists, and it is going to be treated as root now.
			check(!rootObject.isValid());
			rootObject = createdObjects[0];
		}
		else{
			/*
			We're in scenario when no objects were created, and no root object exists
			If created objects were > 1, then root has been created.

			I need to refator that...
			*/
		}
	}

	if (rootObject.isValid()){
		/*
		A root object exists. Either it is a lone object, or it is a blank created to unite multiple "sibling" objects harvested from components.

		We setup parent relationship, and assign correct scene folder
		*/
		workData.registerGameObject(jsonGameObj, rootObject);
		setObjectHierarchy(rootObject, parentObject, folderPath, workData, jsonGameObj);
		rootObject.setFolderPath(folderPath, true);
	}
#endif

	/*
	Let's summarize collision approach and differences.

	Unity has separate rigidbody component.
	Unreal does not. Instead, random primitives can hold properties which are transferred to rigidbody.
	Unity has separate mesh collider component.
	Unreal does not.
	Also, apparently in unreal mesh may not have collision data generated?

	Unity creates compound bodiess by attaching colliders to the rigibody found.

	Unreal.... apparently can mere colliders down to some degree.

	So, what's the approach.

	In situation where phytsics controlled entities are present - or if there are collides present...
	Most likely resulting colliders should be joined into a hierarchy, where the first collider holds properties of the rigidbody from unity.
	The rest must be attached to it in order to be merged down as colliders.
	What's more, all the other components such as lights should be attached to that first "rigidbody" component, which will serve as root.

	So, in order to do it properly, we'll need to first build colliders. Then build static mesh.
	Then make a list out of them, and parent everything to the first collider then pray to cthulhu that this works as intended.
	The root component should also be set as actor root.

	Following components, such as probes, lights and whatever else are ALL going to be parented to that root physic component, whether they're actors or components themselves.

	This is ... convoluted.

	Let's see if I can, after all, turn rigibody into a component instead of relying on semi-random merges.
	*/

	/*
	This portion really doesn't mesh well with the rest of functionality. Needs to be changed.
	*/
	if (jsonGameObj.hasAnimators()){
		processAnimators(workData, jsonGameObj, parentObject, folderPath);

		/*
		We're creating blank hiearchy nodes in situation where the object is controlled by animator. This is to accomodate for unitys' floating bones...
		And this needs to be changed as well.
		*/
		if (!workData.importedObjects.Contains(jsonGameObj.id)){
			auto blankActor = workData.createBlankActor(jsonGameObj);
			setObjectHierarchy(blankActor, parentObject, folderPath, workData, jsonGameObj);
			workData.registerGameObject(jsonGameObj, blankActor);
		}
	}

	return rootObject;
}

USkeletalMesh* JsonImporter::loadSkeletalMeshById(ResId id) const{
	auto foundPath = skinMeshIdMap.Find(id);
	if (!foundPath){
		UE_LOG(JsonLog, Warning, TEXT("Could not load skin mesh %d"), id.toIndex());
		return nullptr;
	}

	auto result = LoadObject<USkeletalMesh>(nullptr, **foundPath);
	return result;

	//if (!skel
	//return nullptr;
}

void JsonImporter::registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg){
	if (!outArray)
		return;
	if (!arg.isValid())
		return;
	outArray->Push(arg);
}

