#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/SphereReflectionCapture.h"
#include "Engine/BoxReflectionCapture.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"

#include "Engine/Classes/Components/BoxReflectionCaptureComponent.h"
#include "Engine/Classes/Components/ReflectionCaptureComponent.h"
#include "Engine/Classes/Components/SphereReflectionCaptureComponent.h"

#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Engine/Classes/Components/PoseableMeshComponent.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"

#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Engine/CollisionProfile.h"
	
#include "RawMesh.h"

#include "JsonObjects.h"
#include "UnrealUtilities.h"

#include "DesktopPlatformModule.h"

using namespace JsonObjects;


void JsonImporter::setObjectHierarchy(const ImportedObject &object, ImportedObject *parentObject, 
		const FString& folderPath, ImportWorkData &workData, const JsonGameObject &gameObj){
	if (parentObject){
		object.attachTo(parentObject);
	}
	else{
		if (folderPath.Len())
			object.setFolderPath(*folderPath);
	}

	object.setActiveInHierarchy(gameObj.activeInHierarchy);
}

ImportedObject JsonImporter::createBlankActor(ImportWorkData &workData, const JsonGameObject &jsonGameObj){
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	AActor *blankActor = workData.world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	auto *rootComponent = NewObject<USceneComponent>(blankActor);
	rootComponent->SetWorldTransform(transform);
	blankActor->SetRootComponent(rootComponent);
	blankActor->SetActorLabel(jsonGameObj.ueName, true);
	rootComponent->SetMobility(jsonGameObj.getUnrealMobility());
	ImportedObject importedObject(blankActor);
	return importedObject;
}

/*
This does it. 

This method processess collision and mesh during object import
*/
ImportedObject JsonImporter::processMeshAndColliders(ImportWorkData &workData, const JsonGameObject &jsonGameObj, int objId, ImportedObject *parentObject, const FString &folderPath, DesiredObjectType desiredObjectType){
	/*
	There are several scenarios ....

	* If there is no mesh and no colliders, we return blank objects.
	* If mesh is not present, but colliders are here, then one collider is selected as a "root" and the rest of them are parented to it.
	* If mesh is present, but does not have a collider, then it is parented to the collider mini-tree. The reasoning for that is that colliders govern rigidbody movement, so the mesh should follow components.
	* If the mesh is present, and is used as a collider, then it becomes the root of collider mini-tree, and the rest of the colliders are parented to it.

	Mesh must be attached to a moving collider, as collider doubles down as a rigidbody.
	*/

	/*
	"Outer" woes.

	A component cannot be created in vacuum and needs to be enclosed in a package.
	By default the package is "Transient", but transient packages disappear.

	Meaning.... by default we need to find closest parent, and grab "Outer" from there.

	If there's no such parent, OR if the user requested an actor, then we either spawn a blank, OR process static mesh as an actor and return that.
	*/

	UObject *outer = workData.findSuitableOuter(jsonGameObj);
	//can be null at this point

	bool hasMainMesh = jsonGameObj.hasMesh();
	int mainMeshColliderIndex = jsonGameObj.findMainMeshColliderIndex();
	auto mainMeshCollider = jsonGameObj.getColliderByIndex(mainMeshColliderIndex);

	ImportedObject collisionMesh, displayOnlyMesh;

	/*
	Not spawning mesh as component means spawning it as actor.
	We... we only want to spawn static mesh as an actor if:
	1. The object has no colliders
	2. It has a signle collider that uses the same mesh

	And that's it.
	*/
	if (jsonGameObj.hasMesh()){
		bool componentRequested = (desiredObjectType == DesiredObjectType::Component);
		if (jsonGameObj.colliders.Num() == 0){//only display mesh is present
			return processStaticMesh(workData, jsonGameObj, objId, parentObject, folderPath, nullptr, componentRequested && outer, outer);
		}
		if ((jsonGameObj.colliders.Num() == 1) && mainMeshCollider){
			return processStaticMesh(workData, jsonGameObj, objId, parentObject, folderPath, mainMeshCollider, componentRequested && outer, outer);
		}
	}

	if (!jsonGameObj.hasMesh() && !jsonGameObj.hasColliders()){
		return ImportedObject();//We're processing an empty and by default they're not recreated as scene components. This may change in future.
	}

	/*
	Goddamit this is getting too complicated.
	*/

	if (desiredObjectType == DesiredObjectType::Actor){
		outer = nullptr;//this will force creation of blank actor that will also serve as outer package.
	}

	USceneComponent *rootComponent = nullptr;
	AActor *rootActor = nullptr;

	bool spawnMeshAsComponent = true;
	if (!outer){
		rootActor = workData.world->SpawnActor<AActor>(AActor::StaticClass(), jsonGameObj.getUnrealTransform());
		rootActor->SetActorLabel(jsonGameObj.ueName);
		rootActor->SetFolderPath(*folderPath);
		outer = rootActor;
	}

	check(outer);
	if (hasMainMesh){
		if (mainMeshCollider){
			collisionMesh = processStaticMesh(workData, jsonGameObj, objId, nullptr, folderPath, mainMeshCollider, spawnMeshAsComponent, outer);
			auto name = FString::Printf(TEXT("%s_collisionMesh"), *jsonGameObj.ueName);
			collisionMesh.setNameOrLabel(*name);
		}
		else{
			displayOnlyMesh = processStaticMesh(workData, jsonGameObj, objId, nullptr, folderPath, nullptr, spawnMeshAsComponent, outer);
			auto name = FString::Printf(TEXT("%s_displayMesh"), *jsonGameObj.ueName);
			displayOnlyMesh.setNameOrLabel(*name);
		}
	}
	check(outer);

	ImportedObject rootObject;
	check(outer);

	TArray<UPrimitiveComponent*> newColliders;
	//Walk through collider list, create primtivies, except that one collider used for the main static mesh.
	for (int i = 0; i < jsonGameObj.colliders.Num(); i++){
		const auto &curCollider = jsonGameObj.colliders[i];

		if (hasMainMesh && (i == mainMeshColliderIndex) && (curCollider.isMeshCollider())){
			newColliders.Add(nullptr);
			continue;
		}

		//auto collider = processCollider(workData, jsonGameObj, rootActor, curCollider);
		auto collider = processCollider(workData, jsonGameObj, outer, curCollider);
		auto name = FString::Printf(TEXT("%s_collider#%d(%s)"), *jsonGameObj.ueName, i, *curCollider.colliderType);

		collider->Rename(*name);
		newColliders.Add(collider);
	}

	//Pick a component suitable for the "root" of the collider hierarchy
	int rootCompIndex = mainMeshColliderIndex;
	if (!mainMeshCollider){
		rootCompIndex = jsonGameObj.findSuitableRootColliderIndex();
		if ((rootCompIndex < 0) || (rootCompIndex >= newColliders.Num())){
			UE_LOG(JsonLog, Warning, TEXT("Could not find suitable root collider on %s(%d)"), *jsonGameObj.name, objId);
			rootCompIndex = 0;
		}

		check(newColliders.Num() > 0);
		rootComponent = newColliders[rootCompIndex];
		check(rootComponent);
		if (rootActor)
			rootActor->SetRootComponent(rootComponent);
		///newColliders.RemoveAt(rootCompIndex);//It is easier to handle this here... or not
	}

	check(rootComponent);
	rootObject = ImportedObject(rootComponent);

	for (int i = 0; i < newColliders.Num(); i++){
		auto curCollider = newColliders[i];
		if (!curCollider)
			continue;
		if (i != rootCompIndex){
			auto tmpObj = ImportedObject(curCollider);
			tmpObj.attachTo(&rootObject);
		}

		makeComponentVisibleInEditor(curCollider);
		convertToInstanceComponent(curCollider);
	}

	if (displayOnlyMesh.isValid()){
		check(rootObject.isValid());
		displayOnlyMesh.attachTo(&rootObject);
		displayOnlyMesh.fixEditorVisibility();
	}

	if (displayOnlyMesh.isValid()){
		displayOnlyMesh.fixEditorVisibility();
		displayOnlyMesh.convertToInstanceComponent();
	}

	if (collisionMesh.isValid()){
		collisionMesh.fixEditorVisibility();
		collisionMesh.convertToInstanceComponent();
	}

	if (rootObject.isValid() && !rootActor){
		/*
		Well... in this case we're rebuilding as components and there's no dummy to replicate unity name.
		So we rename the component.
		*/
		rootObject.setNameOrLabel(jsonGameObj.ueName);
	}

	return rootObject;
}

void JsonImporter::convertToInstanceComponent(USceneComponent *comp) const{
	if (!comp)
		return;
	auto rootActor = comp->GetAttachmentRootActor();
	check(rootActor);
	rootActor->AddInstanceComponent(comp);
}

void JsonImporter::makeComponentVisibleInEditor(USceneComponent *comp) const{
	if (!comp)
		return;
	auto rootActor = comp->GetAttachmentRootActor();
	check(rootActor);
	comp->bEditableWhenInherited = true;
	comp->RegisterComponent();
}


void JsonImporter::importObject(const JsonGameObject &jsonGameObj , int32 objId, ImportWorkData &workData){
	//UE_LOG(JsonLog, Log, TEXT("Importing object %d"), objId);

	auto* parentObject = workData.findImportedObject(jsonGameObj.parentId);

	auto folderPath = workData.processFolderPath(jsonGameObj);
	UE_LOG(JsonLog, Log, TEXT("Num components for object %d(%s): %d"), jsonGameObj.id, *folderPath, jsonGameObj.getNumComponents());

	bool multiComponentObject = jsonGameObj.getNumComponents() > 1;

	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return; 
	}

	//ImportedObjectArray colliderObjects;
	ImportedObjectArray createdObjects;

	auto objectType = DesiredObjectType::Default;

	ImportedObject rootObject = processMeshAndColliders(workData, jsonGameObj, objId, parentObject, folderPath, objectType);

	if (rootObject.isValid()){
	}

	if (jsonGameObj.hasProbes()){
		processReflectionProbes(workData, jsonGameObj, objId, parentObject, folderPath, &createdObjects);
	}
	
	if (jsonGameObj.hasLights()){
		processLights(workData, jsonGameObj, parentObject, folderPath, &createdObjects);
	}

	if (jsonGameObj.hasTerrain()){
		processTerrains(workData, jsonGameObj, parentObject, folderPath, &createdObjects);
	}

	if (jsonGameObj.hasSkinMeshes()){
		processSkinMeshes(workData, jsonGameObj, parentObject, folderPath, &createdObjects);
	}

	if ((createdObjects.Num() > 1) && (!rootObject.isValid())){
		rootObject = createBlankActor(workData, jsonGameObj);
	}

	if (rootObject.isValid()){
		for (auto& cur : createdObjects){
			if (!cur.isValid())
				continue;
			setObjectHierarchy(cur, &rootObject, folderPath, workData, jsonGameObj);
		}
	}
	else{
		if (createdObjects.Num() == 1){
			check(!rootObject.isValid());
			rootObject = createdObjects[0];
		}
	}

	if (rootObject.isValid()){
		workData.registerGameObject(jsonGameObj, rootObject);
		setObjectHierarchy(rootObject, parentObject, folderPath, workData, jsonGameObj);
		rootObject.setFolderPath(folderPath, true);
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
			auto blankActor = createBlankActor(workData, jsonGameObj);
			/*
			FTransform transform;
			transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

			AActor *blankActor = workData.world->SpawnActor<AActor>(AActor::StaticClass(), transform);
			auto *rootComponent = NewObject<USceneComponent>(blankActor);
			rootComponent->SetWorldTransform(transform);
			blankActor->SetRootComponent(rootComponent);
			blankActor->SetActorLabel(jsonGameObj.ueName, true);

			ImportedObject importedObject(blankActor);
			*/
			setObjectHierarchy(blankActor, parentObject, folderPath, workData, jsonGameObj);
			//workData.importedObjects.Add(jsonGameObj.id, blankActor);
			workData.registerGameObject(jsonGameObj, blankActor);
		}
	}
}

USkeletalMesh* JsonImporter::loadSkeletalMeshById(JsonId id) const{
	auto foundPath = skinMeshIdMap.Find(id);
	if (!foundPath){
		UE_LOG(JsonLog, Warning, TEXT("Could not load skin mesh %d"), id);
		return nullptr;
	}

	auto result = LoadObject<USkeletalMesh>(nullptr, **foundPath);
	return result;

	//if (!skel
	//return nullptr;
}

ImportedObject JsonImporter::processSkinRenderer(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		const JsonSkinRenderer &skinRend, ImportedObject *parentObject, const FString &folderPath){

	UE_LOG(JsonLog, Log, TEXT("Importing skin mesh %d for object %s"), skinRend.meshId, *jsonGameObj.name);
	if (skinRend.meshId < 0)
		return ImportedObject();

	auto foundMeshPath = skinMeshIdMap.Find(skinRend.meshId);
	if (!foundMeshPath){
		UE_LOG(JsonLog, Log, TEXT("Could not locate skin mesh %d for object %s"), skinRend.meshId, *jsonGameObj.name);
		return ImportedObject();
	}

	auto *skelMesh = loadSkeletalMeshById(skinRend.meshId);
	if (!skelMesh){
		UE_LOG(JsonLog, Error, TEXT("Coudl not load skinMesh %d on object %d(%s)"), skinRend.meshId, jsonGameObj.id, *jsonGameObj.name);
		return ImportedObject();
	}


	/*
	This is great.

	Looks like there's major discrepancy in how components work in unity and unreal engine.

	Unity skinned mesh acts as BOTH PoseableMesh and SkeletalMesh, meaning you can move individual bones around while they're being animated.
	*/
	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	ASkeletalMeshActor *meshActor = workData.world->SpawnActor<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(), transform, spawnParams);
	if (!meshActor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn skeletal actor"));
		return ImportedObject();
	}

	meshActor->SetActorLabel(jsonGameObj.ueName, true);

	USkeletalMeshComponent *meshComponent = meshActor->GetSkeletalMeshComponent();

	meshComponent->SetSkeletalMesh(skelMesh, true);

	const auto& materials = skinRend.materials;
	if (materials.Num() > 0){
		for(int i = 0; i < materials.Num(); i++){
			auto matId = materials[i];

			auto material = loadMaterialInterface(matId);
			meshComponent->SetMaterial(i, material);
		}
	}

	ImportedObject importedObject(meshActor);
	//workData.importedObjects.Add(jsonGameObj.id, importedObject);
	workData.registerGameObject(jsonGameObj, importedObject);
	setObjectHierarchy(importedObject, parentObject, folderPath, workData, jsonGameObj);
	return importedObject;
}

void JsonImporter::registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg){
	if (!outArray)
		return;
	if (!arg.isValid())
		return;
	outArray->Push(arg);
}

void JsonImporter::processSkinMeshes(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString &folderPath, ImportedObjectArray *createdObjects){
	for(const auto &jsonSkin: gameObj.skinRenderers){
		if (!gameObj.activeInHierarchy)//Temporary hack to debug
			continue;
		auto skinMesh = processSkinRenderer(workData, gameObj, jsonSkin, parentObject, folderPath);
		registerImportedObject(createdObjects, skinMesh);
	}
}

bool JsonImporter::configureStaticMeshComponent(ImportWorkData &workData, UStaticMeshComponent *meshComp, const JsonGameObject &jsonGameObj, bool configForRender, const JsonCollider *collider) const{
	check(meshComp);

	if (!jsonGameObj.hasRenderers()){
		UE_LOG(JsonLog, Warning, TEXT("Renderer not found on %s(%d), cannot create mesh"), *jsonGameObj.ueName, jsonGameObj.id);
		return false;
	}
	check(jsonGameObj.renderers.Num() > 0);

	auto meshPath = meshIdMap[jsonGameObj.meshId];
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return false;
	}

	meshComp->SetStaticMesh(meshObject);
	meshComp->SetMobility(jsonGameObj.getUnrealMobility());

	if (collider){
		setupCommonColliderSettings(workData, meshComp, jsonGameObj, *collider);
	}
	else{
		meshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (!configForRender){
		meshComp->bHiddenInGame = true;//Is this the right way, though...
		return true;
	}

	const auto &renderer = jsonGameObj.renderers[0];
	auto materials = jsonGameObj.getFirstMaterials();

	bool emissiveMesh = false;
	if (materials.Num() > 0){
		for (int i = 0; i < materials.Num(); i++){
			auto matId = materials[i];

			auto *jsonMat = getJsonMaterial(matId);
			if (jsonMat && (jsonMat->isEmissive()))
				emissiveMesh = true;

			auto material = loadMaterialInterface(matId);
			meshComp->SetMaterial(i, material);
		}
	}

	logValue("hasShadows", renderer.castsShadows());
	logValue("twoSidedShadows", renderer.castsTwoSidedShadows());

	meshComp->SetCastShadow(renderer.castsShadows());
	meshComp->bCastShadowAsTwoSided = renderer.castsTwoSidedShadows();//twoSidedShadows;

	if (emissiveMesh)
		meshComp->LightmassSettings.bUseEmissiveForStaticLighting = true;

	return true;
}

ImportedObject JsonImporter::processStaticMesh(ImportWorkData &workData, const JsonGameObject &jsonGameObj, int objId, ImportedObject *parentObject, const FString& folderPath, const JsonCollider *colliderData, bool spawnAsComponent, UObject *outer){
	if (!jsonGameObj.hasMesh())
		return ImportedObject();

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	AStaticMeshActor *meshActor = nullptr;
	UStaticMeshComponent *meshComp = nullptr;

	//
	if (spawnAsComponent){
		UObject* curOuter = outer ? outer: GetTransientPackage();
		meshComp = NewObject<UStaticMeshComponent>(curOuter);
		meshComp->SetWorldTransform(transform);
	}
	else{
		//I wonder why it is "spawn" here and Add everywhere else. But whatever.
		meshActor = workData.world->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
		if (!meshActor){
			UE_LOG(JsonLog, Warning, TEXT("Couldn ot spawn mesh actor"));
			return ImportedObject();
		}

		meshActor->SetActorLabel(jsonGameObj.ueName, true);
		meshComp = meshActor->GetStaticMeshComponent();
	}

	///This is awkward. Previously we couldd attempt loading the mesh prior to building the actor, but now...
	if (!configureStaticMeshComponent(workData, meshComp, jsonGameObj, true, colliderData)){
		UE_LOG(JsonLog, Warning, TEXT("Configuration of static mesh component failed on object '%s'(%d)"), *jsonGameObj.name, objId);
		//return ImportedObject(meshActor);
	}

	const auto* renderer = jsonGameObj.getFirstRenderer();
	if (renderer){
		if (renderer->castsShadowsOnly()){
			if (meshActor){
				meshActor->SetActorHiddenInGame(true);//this doesn't seem to do anything? (-_-)
			}
			if (meshComp){
				meshComp->bCastHiddenShadow = true;
				meshComp->bHiddenInGame = true;
			}
		}
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("First renderer not found on %s(%d)"), *jsonGameObj.name, objId);
	}

	auto result = meshActor ? ImportedObject(meshActor) : ImportedObject(meshComp);

	if (meshActor)
		meshActor->MarkComponentsRenderStateDirty();

	result.setNameOrLabel(jsonGameObj.ueName);
	setObjectHierarchy(result, parentObject, folderPath, workData, jsonGameObj);
	workData.registerGameObject(jsonGameObj, result);

	return result;
}

//void setupReflectionCapture(UReflectionCaptureComponent *reflComponent, const JsonReflectionProbe &probe);
ImportedObject JsonImporter::processReflectionProbe(ImportWorkData &workData, const JsonGameObject &gameObj,
		const JsonReflectionProbe &probe, int32 objId, ImportedObject *parentObject, const FString &folderPath){
	using namespace UnrealUtilities;

	FMatrix captureMatrix = gameObj.ueWorldMatrix;

	FVector ueCenter = unityPosToUe(probe.center);
	FVector ueSize = unitySizeToUe(probe.size);
	FVector xAxis, yAxis, zAxis;
	captureMatrix.GetScaledAxes(xAxis, yAxis, zAxis);
	auto origin = captureMatrix.GetOrigin();
	//origin += xAxis * ueCenter.X * 100.0f + yAxis * ueCenter.Y * 100.0f + zAxis * ueCenter.Z * 100.0f;
	origin += xAxis * ueCenter.X + yAxis * ueCenter.Y + zAxis * ueCenter.Z;

	auto sphereInfluence = FMath::Max3(ueSize.X, ueSize.Y, ueSize.Z) * 0.5f;
	if (probe.boxProjection){
		xAxis *= ueSize.X * 0.5f;
		yAxis *= ueSize.Y * 0.5f;
		zAxis *= ueSize.Z * 0.5f;
	}

	captureMatrix.SetOrigin(origin);
	captureMatrix.SetAxes(&xAxis, &yAxis, &zAxis);

	//bool realtime = mode == "Realtime";
	bool baked = (probe.mode == "Baked");
	if (!baked){
		UE_LOG(JsonLog, Warning, TEXT("Realtime reflections are not supported. object %s(%d)"), *gameObj.ueName, objId);
	}

	FTransform captureTransform;
	captureTransform.SetFromMatrix(captureMatrix);
	UReflectionCaptureComponent *reflComponent = 0;

	auto preInit = [&](AReflectionCapture *refl){
		if (!refl) 
			return;
		refl->SetActorLabel(gameObj.ueName);
		auto moveResult = refl->SetActorTransform(captureTransform, false, nullptr, ETeleportType::ResetPhysics);
		logValue("Actor move result: ", moveResult);
	};

	auto postInit = [&](AReflectionCapture *refl){
		if (!refl)
			return;
		refl->MarkComponentsRenderStateDirty();
		//setActorHierarchy(refl, parentActor, folderPath, workData, gameObj);

		setObjectHierarchy(ImportedObject(refl), parentObject, folderPath, workData, gameObj);
	};

	auto setupComponent = [&](UReflectionCaptureComponent *reflComponent) -> void{
		if (!reflComponent)
			return;
		reflComponent->Brightness = probe.intensity;
		reflComponent->ReflectionSourceType = EReflectionSourceType::CapturedScene;
		if (probe.mode == "Custom"){
			reflComponent->ReflectionSourceType = EReflectionSourceType::SpecifiedCubemap;
			auto cube = getCubemap(probe.customCubemapId);
			if (!cube){
				UE_LOG(JsonLog, Warning, TEXT("Custom cubemap not set on reflection probe on object \"%s\"(%d)"),
					*gameObj.ueName, objId);
			}
			else
				reflComponent->Cubemap = cube;
			//UE_LOG(JsonLog, Warning, TEXT("Cubemaps are not yet fully supported: %s(%d)"), *gameObj.ueName, objId);
		}
		if (probe.mode == "Realtime"){
			UE_LOG(JsonLog, Warning, TEXT("Realtime reflection probes are not support: %s(%d)"), *gameObj.ueName, objId);
		}
	};

	if (!probe.boxProjection){
		auto sphereActor = createActor<ASphereReflectionCapture>(workData, captureTransform, TEXT("sphere capture"));
		if (sphereActor){
			preInit(sphereActor);

			auto captureComp = sphereActor->GetCaptureComponent();
			reflComponent = captureComp;
			auto* sphereComp = Cast<USphereReflectionCaptureComponent>(captureComp);
			if (sphereComp){
				sphereComp->InfluenceRadius = sphereInfluence;
			}
			setupComponent(sphereComp);
			postInit(sphereActor);
		}
		return ImportedObject(sphereActor);
	}
	else{
		auto boxActor = createActor<ABoxReflectionCapture>(workData, captureTransform, TEXT("box reflection capture"));
		if (boxActor){
			preInit(boxActor);

			auto captureComp = boxActor->GetCaptureComponent();
			reflComponent = captureComp;
			auto *boxComp = Cast<UBoxReflectionCaptureComponent>(captureComp);
			if (boxComp){
				boxComp->BoxTransitionDistance = unityDistanceToUe(probe.blendDistance * 0.5f);
			}
			setupComponent(boxComp);

			//TODO: Cubemaps
			/*if (isStatic)
				actor->SetMobility(EComponentMobility::Static);*/
			postInit(boxActor);
		}
		return ImportedObject(boxActor);
	}
}


void JsonImporter::processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, int32 objId, ImportedObject *parentObject, const FString &folderPath, ImportedObjectArray *createdObjects){
	if (!gameObj.hasProbes())
		return;

	if (!gameObj.isStatic){
		UE_LOG(JsonLog, Warning, TEXT("Moveable reflection captures are not supported. Object %s(%d)"), *gameObj.ueName, objId);
		//return;
	}

	for (int i = 0; i < gameObj.probes.Num(); i++){
		const auto &probe = gameObj.probes[i];
		auto probeObject = processReflectionProbe(workData, gameObj, gameObj.probes[i], objId, parentObject, folderPath);
		registerImportedObject(createdObjects, probeObject);
	}
}

void JsonImporter::setupPointLightComponent(UPointLightComponent *pointLight, const JsonLight &jsonLight){
	//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity

	pointLight->SetIntensity(jsonLight.intensity);
	pointLight->bUseInverseSquaredFalloff = false;
	//pointLight->LightFalloffExponent = 2.0f;
	pointLight->SetLightFalloffExponent(2.0f);

	pointLight->SetLightColor(jsonLight.color);
	float attenRadius = jsonLight.range*100.0f;//*ueAttenuationBoost;//those are fine
	pointLight->AttenuationRadius = attenRadius;
	pointLight->SetAttenuationRadius(attenRadius);
	pointLight->CastShadows = jsonLight.castsShadows;//lightCastShadow;// != FString("None");
}

void JsonImporter::setupSpotLightComponent(USpotLightComponent *spotLight, const JsonLight &jsonLight){
	//spotLight->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
	spotLight->SetIntensity(jsonLight.intensity);
	spotLight->bUseInverseSquaredFalloff = false;
	//spotLight->LightFalloffExponent = 2.0f;
	spotLight->SetLightFalloffExponent(2.0f);


	spotLight->SetLightColor(jsonLight.color);
	float attenRadius = jsonLight.range*100.0f;//*ueAttenuationBoost;
	spotLight->AttenuationRadius = attenRadius;
	spotLight->SetAttenuationRadius(attenRadius);
	spotLight->CastShadows = jsonLight.castsShadows;//lightCastShadow;// != FString("None");
	//spotLight->InnerConeAngle = lightSpotAngle * 0.25f;
	spotLight->InnerConeAngle = 0.0f;
	spotLight->OuterConeAngle = jsonLight.spotAngle * 0.5f;
	//spotLight->SetVisibility(params.visible);
}

void JsonImporter::setupDirLightComponent(ULightComponent *dirLight, const JsonLight &jsonLight){
	//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
	dirLight->SetIntensity(jsonLight.intensity);
	//light->bUseInverseSquaredFalloff = false;
	//light->LightFalloffExponent = 2.0f;
	//light->SetLightFalloffExponent(2.0f);

	dirLight->SetLightColor(jsonLight.color);
	//float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;
	//light->AttenuationRadius = attenRadius;
	//light->SetAttenuationRadius(attenRadius);
	dirLight->CastShadows = jsonLight.castsShadows;// != FString("None");
	//light->InnerConeAngle = lightSpotAngle * 0.25f;

	//light->InnerConeAngle = 0.0f;
	//light->OuterConeAngle = lightSpotAngle * 0.5f;

	//light->SetVisibility(params.visible);
}


ImportedObject JsonImporter::processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &jsonLight, ImportedObject *parentObject, const FString& folderPath){
	using namespace UnrealUtilities;

	UE_LOG(JsonLog, Log, TEXT("Creating light"));

	FTransform lightTransform;
	lightTransform.SetFromMatrix(gameObj.ueWorldMatrix);

	ALight *actor = nullptr;
	if (jsonLight.lightType == "Point"){
		auto pointActor = createActor<APointLight>(workData, lightTransform, TEXT("point light"));
		actor = pointActor;
		if (pointActor){
			auto light = pointActor->PointLightComponent;
			setupPointLightComponent(light, jsonLight);
		}
	}
	else if (jsonLight.lightType == "Spot"){
		auto spotActor = createActor<ASpotLight>(workData, lightTransform, TEXT("spot light"));
		actor = spotActor;
		if (actor){
			auto light = spotActor->SpotLightComponent;
			setupSpotLightComponent(light, jsonLight);
		}
	}
	else if (jsonLight.lightType == "Directional"){
		auto dirLightActor = createActor<ADirectionalLight>(workData, lightTransform, TEXT("directional light"));
		actor = dirLightActor;
		if (dirLightActor){
			auto light = dirLightActor->GetLightComponent();
			setupDirLightComponent(light, jsonLight);
		}
	}

	if (actor){
		actor->SetActorLabel(gameObj.ueName, true);
		if (gameObj.isStatic)
			actor->SetMobility(EComponentMobility::Static);
		setObjectHierarchy(ImportedObject(actor), parentObject, folderPath, workData, gameObj);
		actor->MarkComponentsRenderStateDirty();
	}

	return ImportedObject(actor);
}

//void JsonImporter::processLights(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath){
void JsonImporter::processLights(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath, ImportedObjectArray *createdObjects){
	if (!gameObj.hasLights())
		return;

	for(int i = 0; i < gameObj.lights.Num(); i++){
		const auto &curLight = gameObj.lights[i];
		//processLight(workData, gameObj, curLight, parentActor, folderPath);
		auto light = processLight(workData, gameObj, curLight, parentObject, folderPath);
		registerImportedObject(createdObjects, light);
	}
}

UBoxComponent* JsonImporter::createBoxCollider(UObject *ownerPtr, const JsonGameObject &jsonGameObj, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *boxComponent = NewObject<UBoxComponent>(ownerPtr ? ownerPtr : GetTransientPackage(), UBoxComponent::StaticClass());

	auto centerAdjust = unityPosToUe(collider.center);

	boxComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	boxComponent->SetMobility(jsonGameObj.getUnrealMobility());
	boxComponent->SetBoxExtent(unitySizeToUe(collider.size) * 0.5f);
	return boxComponent;
}

USphereComponent* JsonImporter::createSphereCollider(UObject *ownerPtr, const JsonGameObject &jsonGameObj, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *sphereComponent = NewObject<USphereComponent>(ownerPtr ? ownerPtr : GetTransientPackage(), USphereComponent::StaticClass());
	sphereComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	sphereComponent->SetMobility(jsonGameObj.getUnrealMobility());
	sphereComponent->SetSphereRadius(unityDistanceToUe(collider.radius));
	return sphereComponent;
}

UCapsuleComponent* JsonImporter::createCapsuleCollider(UObject *ownerPtr, const JsonGameObject &jsonGameObj, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *capsule = NewObject<UCapsuleComponent>(ownerPtr ? ownerPtr : GetTransientPackage(), UCapsuleComponent::StaticClass());

	FMatrix capsuleMatrix = FMatrix::Identity;
	capsuleMatrix.SetOrigin(collider.center);

	//auto capsuleTransform = jsonGameObj.getUnrealTransform(collider.center);
	//capsule->SetAxis... //Erm? No axis control?
	switch (collider.direction){
		case(JsonCollider::XAxis):{
			auto xAxis = FVector(0.0f, -1.0f, 0.0f);
			auto yAxis = FVector(1.0f, 0.0f, 0.0f);
			auto zAxis = FVector(0.0f, 0.0f, 1.0f);
			capsuleMatrix.SetAxes(&xAxis, &yAxis, &zAxis);
			break;
		}
		case(JsonCollider::YAxis):{
			//nothing.
			break;
		}
		case(JsonCollider::ZAxis):{
			auto xAxis = FVector(1.0f, 0.0f, 0.0f);
			auto yAxis = FVector(0.0f, 0.0f, 1.0f);
			auto zAxis = FVector(0.0f, -1.0f, 0.0f);
			capsuleMatrix.SetAxes(&xAxis, &yAxis, &zAxis);
			break;
		}
	}

	FMatrix capsuleCombinedMatrix = capsuleMatrix * jsonGameObj.worldMatrix;
	FTransform capsuleTransform;
	capsuleTransform.SetFromMatrix(unityWorldToUe(capsuleCombinedMatrix));

	capsule->SetWorldTransform(capsuleTransform);
	capsule->SetMobility(jsonGameObj.getUnrealMobility());

	capsule->SetCapsuleHalfHeight(unityDistanceToUe(collider.height*0.5f));
	capsule->SetCapsuleRadius(unityDistanceToUe(collider.radius));

	return capsule;
}

UStaticMeshComponent* JsonImporter::createMeshCollider(UObject *ownerPtr, const JsonGameObject &jsonGameObj, const JsonCollider &collider, ImportWorkData &workData) const{
	using namespace UnrealUtilities;
	auto *meshComponent = NewObject<UStaticMeshComponent>(ownerPtr ? ownerPtr : GetTransientPackage(), UStaticMeshComponent::StaticClass());

	configureStaticMeshComponent(workData, meshComponent, jsonGameObj, false, &collider);

	meshComponent->SetWorldTransform(jsonGameObj.getUnrealTransform());//no center for the static mesh
	meshComponent->SetMobility(jsonGameObj.getUnrealMobility());

	return meshComponent;
}


void JsonImporter::setupCommonColliderSettings(const ImportWorkData &workData, UPrimitiveComponent *dstCollider, const JsonGameObject &jsonGameObj, const JsonCollider &collider) const{
	check(dstCollider);
	if (collider.trigger){
		dstCollider->SetCollisionProfileName(FName("OverlapAll"));//this is not available as a constant 
		dstCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else{
		dstCollider->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		dstCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	const auto *rigBody = workData.locateRigidbody(jsonGameObj);
	//Hmm. Shoudl this even be here?
	if (rigBody){
		//bool compoundColliderRoot = workData.isCompoundRigidbodyRootCollider(jsonGameObj);//TODO: Cache this?
		//int rootIndex = workData.

		bool physicsEnabled = !rigBody->isKinematic;
		if (rigBody){
			//It is necessary to do this in order to utilize body welding on compound colliders
			auto compoundColliderChild = true;
			if (workData.isCompoundRigidbodyRootCollider(jsonGameObj)){
				if (jsonGameObj.findSuitableRootColliderIndex() == collider.colliderIndex){
					compoundColliderChild = false;
				}
			}
			physicsEnabled = physicsEnabled && !compoundColliderChild;
		}

		//kinematic rigidbodies?
		dstCollider->SetSimulatePhysics(physicsEnabled);
		dstCollider->SetMassOverrideInKg(NAME_None, rigBody->mass, true);
		dstCollider->SetEnableGravity(rigBody->useGravity);

		auto ccd = rigBody->usesContinuousCollision() 
			|| rigBody->usesContinuousDynamicCollision() 
			|| rigBody->usesContinuousSpeculativeCollision();

		dstCollider->SetAllUseCCD(ccd);

		//Well, we can't set rigidbody drag and angular drag, it seems.
	}
}

UPrimitiveComponent* JsonImporter::processCollider(ImportWorkData &workData, const JsonGameObject &jsonGameObj, UObject *ownerPtr, const JsonCollider &collider){
	using namespace UnrealUtilities;
	//trigger support...?
	UPrimitiveComponent *colliderComponent = nullptr;
	if (collider.isBoxCollider()){
		colliderComponent = createBoxCollider(ownerPtr, jsonGameObj, collider);
	}
	else if (collider.isSphereCollider()){
		colliderComponent = createSphereCollider(ownerPtr, jsonGameObj, collider);
	}
	else if (collider.isCapsuleCollider()){
		colliderComponent = createCapsuleCollider(ownerPtr, jsonGameObj, collider);
	}
	else if (collider.isMeshCollider()){
		colliderComponent = createMeshCollider(ownerPtr, jsonGameObj, collider, workData);
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("Unknown or unsupported collider type \'%s\" on object \'%s\' (%d)"),
			*collider.colliderType, *jsonGameObj.name, jsonGameObj.id);
		return nullptr;//ImportedObject();
	}
	if (!colliderComponent){
		return nullptr;// ImportedObject();
	}

	colliderComponent->RegisterComponent();
	setupCommonColliderSettings(workData, colliderComponent, jsonGameObj, collider);

	return colliderComponent;// ImportedObject(colliderComponent);
}
