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

	/*if (jsonGameObj.isStatic)
		rootComponent->SetMobility(EComponentMobility::Static);*/

	ImportedObject importedObject(blankActor);
	return importedObject;
}


void JsonImporter::importObject(const JsonGameObject &jsonGameObj , int32 objId, ImportWorkData &workData){
	//UE_LOG(JsonLog, Log, TEXT("Importing object %d"), objId);

	FString folderPath;

	auto* parentObject = workData.importedObjects.Find(jsonGameObj.parentId);

	FString childFolderPath = jsonGameObj.ueName;
	if (jsonGameObj.parentId >= 0){
		const FString* found = workData.objectFolderPaths.Find(jsonGameObj.parentId);
		if (found){
			folderPath = *found;
			childFolderPath = folderPath + "/" + jsonGameObj.ueName;
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("Object parent not found, folder path may be invalid"));
		}
	}

	//UE_LOG(JsonLog, Log, TEXT("Folder path for object: %d: %s"), jsonGameObj.id, *folderPath);
	workData.objectFolderPaths.Add(jsonGameObj.id, childFolderPath);
	UE_LOG(JsonLog, Log, TEXT("Num components for object %d(%s): %d"), jsonGameObj.id, *folderPath, jsonGameObj.getNumComponents());

	bool multiComponentObject = jsonGameObj.getNumComponents() > 1;

	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return; 
	}

	ImportedObjectArray colliderObjects;
	ImportedObjectArray createdObjects;

	if (jsonGameObj.hasMesh()){
		registerImportedObject(&createdObjects,
			processStaticMesh(workData, jsonGameObj, objId, parentObject, folderPath));
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

	ImportedObject objectRoot;

	if ((createdObjects.Num() > 1) 
		|| ((createdObjects.Num() == 1) && (!createdObjects[0].actor))
		|| ((createdObjects.Num() == 0) && (jsonGameObj.hasColliders()))
		){
		//Collapse nodes under single actor. 
		auto blankActor = createBlankActor(workData, jsonGameObj);
		setObjectHierarchy(blankActor, parentObject, folderPath, workData, jsonGameObj);
		workData.importedObjects.Add(jsonGameObj.id, blankActor);

		for (auto& cur : createdObjects){
			check(cur.isValid());
			setObjectHierarchy(cur, &blankActor, folderPath, workData, jsonGameObj);
		}
		objectRoot = blankActor;
	}
	else if (createdObjects.Num() == 1){
		objectRoot = createdObjects[0];
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
	if (jsonGameObj.hasColliders()){
		check(objectRoot.isValid());//Should've been created within previous if/else

		ImportedObjectArray newColliders;
		processColliders(workData, jsonGameObj, &objectRoot, &newColliders);//&createdObjects);
		for (auto& cur : newColliders){
			check(cur.isValid());
			cur.attachTo(&objectRoot);
			if (!cur.component)
				continue;
			//if we don't do that... the component won't show up in the editor.
			auto *root = cur.component->GetAttachmentRootActor();
			if (!root)
				continue;
			//cur.component->RegisterComponent();
			root->AddInstanceComponent(cur.component);
			//setObjectHierarchy(cur, &blankActor, folderPath, workData, jsonGameObj);
		}
	}

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
			workData.importedObjects.Add(jsonGameObj.id, blankActor);
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
	workData.importedObjects.Add(jsonGameObj.id, importedObject);
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


ImportedObject JsonImporter::processStaticMesh(ImportWorkData &workData, const JsonGameObject &jsonGameObj, int objId, ImportedObject *parentObject, const FString& folderPath){
	if (!jsonGameObj.hasMesh())
		return ImportedObject();

	UE_LOG(JsonLog, Log, TEXT("Mesh found in object %d, name %s"), objId, *jsonGameObj.ueName);
	
	auto meshPath = meshIdMap[jsonGameObj.meshId];
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return ImportedObject();
	}

	//I wonder why it is "spawn" here and Add everywhere else. But whatever.
	AActor *meshActor = workData.world->SpawnActor<AActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
	if (!meshActor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn actor"));
		return ImportedObject();
	}

	meshActor->SetActorLabel(jsonGameObj.ueName, true);

	AStaticMeshActor *worldMesh = Cast<AStaticMeshActor>(meshActor);
	//if params is static
	if (!worldMesh){
		UE_LOG(JsonLog, Warning, TEXT("Wrong actor class"));
		return ImportedObject();
	}

	auto meshComp = worldMesh->GetStaticMeshComponent();
	meshComp->SetStaticMesh(meshObject);

	if (!jsonGameObj.hasRenderers()){
		UE_LOG(JsonLog, Warning, TEXT("Renderer not found on %s(%d), cannot create mesh"), *jsonGameObj.ueName, jsonGameObj.id);
		return ImportedObject();
	}

	const auto &renderer = jsonGameObj.renderers[0];
	auto materials = jsonGameObj.getFirstMaterials();
	if (materials.Num() > 0){
		for(int i = 0; i < materials.Num(); i++){
			auto matId = materials[i];

			auto material = loadMaterialInterface(matId);
			meshComp->SetMaterial(i, material);
		}
	}

	bool hasShadows = false;
	bool twoSidedShadows = false;
	bool hideInGame = false;
	if (renderer.shadowCastingMode == FString("ShadowsOnly")){
		twoSidedShadows = false;
		hasShadows = true;
		hideInGame = true;
	}
	else if (renderer.shadowCastingMode == FString("On")){
		hasShadows = true;
		twoSidedShadows = false;
	}
	else if (renderer.shadowCastingMode == FString("TwoSided")){
		hasShadows = true;
		twoSidedShadows = true;
	}
	else{
		hasShadows = false;
		twoSidedShadows = false;
	}
	logValue("hasShadows", hasShadows);
	logValue("twoSidedShadows", twoSidedShadows);

	worldMesh->SetActorHiddenInGame(hideInGame);

	meshComp->SetMobility(jsonGameObj.getUnrealMobility());
	/*if (jsonGameObj.isStatic)
		meshComp->SetMobility(EComponentMobility::Static);*/

	if (meshObject){
		bool emissiveMesh = false;
		//for(auto cur: meshObject->Materials){
		for(auto cur: meshObject->StaticMaterials){
			auto matIntr = cur.MaterialInterface;//cur->GetMaterial();
			if (!matIntr)
				continue;
			auto mat = matIntr->GetMaterial();
			if (!mat)
				continue;
			if (mat->EmissiveColor.IsConnected()){
				emissiveMesh = true;
				break;
			}
		}
		meshComp->LightmassSettings.bUseEmissiveForStaticLighting = emissiveMesh;
	}

	/*
	We actually probably need to switch it to , well, constructing object on the fly at some point.
	*/
	//workData.objectActors.Add(jsonGameObj.id, meshActor);
	auto result = ImportedObject(meshActor);
	workData.importedObjects.Add(jsonGameObj.id, result);
	setObjectHierarchy(result, parentObject, folderPath, workData, jsonGameObj);

	meshComp->SetCastShadow(hasShadows);
	meshComp->bCastShadowAsTwoSided = twoSidedShadows;

	worldMesh->MarkComponentsRenderStateDirty();

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
		//importedActor.setActorLabel(
		actor->SetActorLabel(gameObj.ueName, true);
		if (gameObj.isStatic)
			actor->SetMobility(EComponentMobility::Static);
		//setActorHierarchy(&importedActor, parentObject, folderPath, workData, gameObj);
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

UBoxComponent* JsonImporter::createBoxCollider(UObject *parentPtr, const JsonGameObject &jsonGameObj, ImportedObject *parentObject, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *boxComponent = NewObject<UBoxComponent>(parentPtr ? parentPtr : GetTransientPackage(), UBoxComponent::StaticClass());

	auto centerAdjust = unityPosToUe(collider.center);

	boxComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	boxComponent->SetMobility(jsonGameObj.getUnrealMobility());
	boxComponent->SetBoxExtent(unitySizeToUe(collider.size) * 0.5f);
	//boxComponent->RegisterComponent();

	return boxComponent;
}

USphereComponent* JsonImporter::createSphereCollider(UObject *parentPtr, const JsonGameObject &jsonGameObj, ImportedObject *parentObject, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *sphereComponent = NewObject<USphereComponent>(parentPtr ? parentPtr : GetTransientPackage(), USphereComponent::StaticClass());
	sphereComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	sphereComponent->SetMobility(jsonGameObj.getUnrealMobility());
	sphereComponent->SetSphereRadius(unityDistanceToUe(collider.radius));
	//sphereComponent->RegisterComponent();
	return sphereComponent;
}

UCapsuleComponent* JsonImporter::createCapsuleCollider(UObject *parentPtr, const JsonGameObject &jsonGameObj, ImportedObject *parentObject, const JsonCollider &collider) const{
	using namespace UnrealUtilities;
	auto *capsule = NewObject<UCapsuleComponent>(parentPtr ? parentPtr : GetTransientPackage(), UCapsuleComponent::StaticClass());

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

	//capsule->RegisterComponent();
	return capsule;
}

ImportedObject JsonImporter::processCollider(ImportWorkData &workData, const JsonGameObject &jsonGameObj, ImportedObject *parentObject, const JsonCollider &collider){
	using namespace UnrealUtilities;
	//trigger support...?
	UObject *parentPtr = parentObject ? parentObject->getPtrForOuter() : nullptr;
	UPrimitiveComponent *colliderComponent = nullptr;
	if (collider.colliderType == "box"){
		colliderComponent = createBoxCollider(parentPtr, jsonGameObj, parentObject, collider);
	}
	else if (collider.colliderType == "sphere"){
		colliderComponent = createSphereCollider(parentPtr, jsonGameObj, parentObject, collider);
	}
	else if (collider.colliderType == "capsule"){
		colliderComponent = createCapsuleCollider(parentPtr, jsonGameObj, parentObject, collider);
	}
	/*
	else if (collider.colliderType == "mesh"){
	}
	*/
	else{
		UE_LOG(JsonLog, Warning, TEXT("Unknown or unsupported collider type \'%s\" on object \'%s\' (%d)"),
			*collider.colliderType, *jsonGameObj.name, jsonGameObj.id);
		return ImportedObject();
	}
	if (!colliderComponent){
		return ImportedObject();
	}

	colliderComponent->RegisterComponent();
	if (collider.trigger){
		colliderComponent->SetCollisionProfileName(FName("OverlapAll"));//this is not available as a constant 
		colliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//colliderComponent->Simulat
	}
	else{
		colliderComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		colliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	const auto *rigBody = workData.locateRigidbody(jsonGameObj);
	if (rigBody){
		//rigBody->
	}
	
	return ImportedObject(colliderComponent);
}

void JsonImporter::processColliders(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, ImportedObjectArray *createdObjects){
	if (!gameObj.hasColliders())
		return;

	for (int i = 0; i < gameObj.colliders.Num(); i++){
		const auto &curCollider = gameObj.colliders[i];

		registerImportedObject(createdObjects,
			processCollider(workData, gameObj, parentObject, curCollider));
	}
}
