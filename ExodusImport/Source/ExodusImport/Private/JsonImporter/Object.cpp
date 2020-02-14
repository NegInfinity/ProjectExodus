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

/*
We've ... actually I've run into a pickle.

It goes like this:
* Scene graph constructed from multiple actors cannot be automatically harvested into a blueprint.
* Harvesting objects into blueprint opens blueprint editor (troublesome with a lot of blueprints)
* Apparently skinned mesh can crash the whole process.

So, there's a need of major overhaul.

First, there's a category of situations where objects will need to be reconstructed as a single actor, with actor node being enabled for root.
This happens in case of prefabs, or prefab instances.

Question remains how to track non-prefab objects linked to prefab instances. But this can wait for later.

In a situation where we're dealing with a prefab instance, we'll have to create a root at the bottom AND spawn empty scene nodes as components.
As contents are no longer working as folders.

Additionally, we would want to track situations where there's a single object with a single component that spawns a specialized actor.
This is not strictly necessary, but desirable, so lights will be point light actors and not actors.

Additionally, there's trouble with ownership. It seems that creating component as part of transient package and then switching ownership 
to a non-transient package causes that component to disappear. I kinda wonder why, perhaps I missed a part where I update number of gc links or something.

Therefore, we need to create a blank actor IN ADVANCE, and use that blank actor as an owner to all the child components.

Additioanlly, previously discarded version where I attempted to create chidl components with specifying owners had a glitch where the components created this way were not properly registering in the scene.

So. I scrapped the whole branch, and started over.

To summarize the rules:
* an object spawns actor instance, if it is not part of a prefab instance and has components.
* an objects spawns nothing, if it is not a part of a prefab instance, and has no components.
* an objects spawns components, if it is a part of prefab instance. 
* prefab instance has AActor as its base class, EXCEPT the situation where it is dealing with a single component object...? 

Hmm... (-_-)

Current plan - disable creation of prefabs.
Add handling of prefab instances according to those rules.
*/

/*
*/
ImportedObject JsonImporter::importObject(const JsonGameObject &jsonGameObj, ImportContext &workData, bool createEmptyTransforms){
	using namespace UnrealUtilities;

	auto* parentObject = workData.findImportedObject(jsonGameObj.parentId);

	auto folderPath = workData.processFolderPath(jsonGameObj);
	UE_LOG(JsonLog, Log, TEXT("importing object: %d(%s), folder: %s, Num Components: %d"), 
		jsonGameObj.id, *jsonGameObj.name, *folderPath, jsonGameObj.getNumComponents());

	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return ImportedObject(); 
	}

	ImportedObjectArray createdObjects;

	//auto objectType = DesiredObjectType::Default;
	/*
	When importing prefabs, child actor nodes are not being harvested correctly. They have to be rebuilt as component-only structures
	*/
	bool objectIsPrefab = jsonGameObj.usesPrefab();
	bool mustCreateBlankNodes = objectIsPrefab || createEmptyTransforms;//prefab objects can have no folders, so we're going to faithfully rebuild node hierarchy.
	bool createActorNodes = jsonGameObj.isPrefabRoot() || !objectIsPrefab;
	//createActorNodes = true;

	//In situation where there's no parent, we have to create an actor. Otherwise we will have no valid outer
	createActorNodes = createActorNodes || !parentObject;

	//createActorNodes = true;
	bool rootMustBeActor = createActorNodes;

	UObject *existingOuter = workData.findSuitableOuter(jsonGameObj);
	AActor *existingRootActor = nullptr;
	AActor *createdRootActor = nullptr;
	auto outerCreator = [&]() -> UObject*{
		if (existingOuter)
			return existingOuter;
		if (existingRootActor)
			return existingRootActor;
		if (!createdRootActor){
			createdRootActor = workData.createBlankActor(jsonGameObj, false);
		}
		check(createdRootActor != nullptr);
		return createdRootActor;
	};

	/*
	Here we handle creation of display geometry and colliders. This particular function call harvests colliders, reigidbody properties, builds them into a somewhat sensible hierarchy,
	and returns root object to us
	*/
	ImportedObject rootObject = GeometryComponentBuilder::processMeshAndColliders(workData, jsonGameObj, parentObject, folderPath, 
		!createActorNodes,
		//createActorNodes ? DesiredObjectType::Actor: objectType, 
		this, outerCreator);
	if (rootObject.isValid()){
		createdObjects.Add(rootObject);
		if (rootObject.hasActor()){
			//leaky abstractions, leaky abstractions everywhere....
			existingRootActor = rootObject.actor;
			check(existingRootActor != nullptr);
		}
	}

	//Oh, I know. This si kinda nuts, but let's initialize root actor using lazy evaluation.

	/*
	The block below walks through every component type we currently support, and spawns unreal-side representation.
	*/
	if (jsonGameObj.hasProbes()){
		ReflectionProbeBuilder::processReflectionProbes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}
	
	if (jsonGameObj.hasLights()){
		LightBuilder::processLights(workData, jsonGameObj, parentObject, folderPath, &createdObjects, createActorNodes, outerCreator);
	}

	if (jsonGameObj.hasTerrain()){
		TerrainComponentBuilder::processTerrains(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}

	if (jsonGameObj.hasSkinMeshes()){
		SkeletalMeshComponentBuilder::processSkinMeshes(workData, jsonGameObj, parentObject, folderPath, &createdObjects, this, outerCreator);
	}

	/*
	At this point, one of the scenarios is true:

	1. Root actor has been created by mesh component. (existingRootActor set)
	2. Root actor has been created by actor creator and has no scene root component.
	3. Root actor hasn't been created, but outerPtr was located on request.

	So....

	It seems we only need to process the root normally, and an actor is created, but lacks root component, we set the root object as its component.
	*/

	if (createdObjects.Num() > 1){
		if (!rootObject.isValid()){
			//More than one object exists, however, in case this is a mesh node, then the mesh actor is registered as a root by default.
			//rootObject = workData.createBlankActor(jsonGameObj);
			rootObject = workData.createBlankNode(jsonGameObj, createActorNodes, true, outerCreator);
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
		if (mustCreateBlankNodes){
			rootObject = workData.createBlankNode(jsonGameObj, createActorNodes, true, outerCreator);
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

		if (rootMustBeActor && !rootObject.hasActor()){
			//whoops. Creating blank node.
			check(rootObject.hasComponent());
			check(createdRootActor != nullptr);//This HAS to be created by this point.
			//if (!)
			//auto blankNode = workData.createBlankActor(jsonGameObj, rootObject.component, true);
			//rootObject = blankNode;
			check(createdRootActor->GetRootComponent() == nullptr);
			createdRootActor->SetRootComponent(rootObject.component);

			auto blankNode = ImportedObject(createdRootActor);
			rootObject = blankNode;
		}

		workData.registerGameObject(jsonGameObj, rootObject);
		setObjectHierarchy(rootObject, parentObject, folderPath, workData, jsonGameObj);
		rootObject.setFolderPath(folderPath, true);
	}

	if (rootObject.isValid() && parentObject && parentObject->isValid()){
		//We need to change owner if the object tree was made without root actor. Otherwise it'll poof.
		if (!rootObject.hasActor()){
		}
	}

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
}

void JsonImporter::registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg){
	if (!outArray)
		return;
	if (!arg.isValid())
		return;
	outArray->Push(arg);
}

