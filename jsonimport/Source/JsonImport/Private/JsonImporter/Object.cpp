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
	
#include "RawMesh.h"

#include "JsonObjects.h"

#include "DesktopPlatformModule.h"

static void setParentAndFolder(AActor *actor, AActor *parentActor, const FString& folderPath){
	if (!actor)
		return;
	if (parentActor){
		actor->AttachToActor(parentActor, FAttachmentTransformRules::KeepWorldTransform);
	}
	else{
		//actor->AddToRoot();
		if (folderPath.Len())
			actor->SetFolderPath(*folderPath);
	}
}

void JsonImporter::importObject(JsonObjPtr obj, int32 objId, ImportWorkData &workData){
	UE_LOG(JsonLog, Log, TEXT("Importing object %d"), objId);

	auto jsonGameObj = JsonGameObject(obj);

	FString folderPath;

	AActor *parentActor = workData.objectActors.FindRef(jsonGameObj.parentId);

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

	UE_LOG(JsonLog, Log, TEXT("Folder path for object: %d: %s"), jsonGameObj.id, *folderPath);
	workData.objectFolderPaths.Add(jsonGameObj.id, childFolderPath);


	if (!workData.world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return; 
	}

	if (jsonGameObj.hasProbes())
		processReflectionProbes(workData, jsonGameObj, objId, parentActor, folderPath);
	
	if (jsonGameObj.hasLights())
		processLights(workData, jsonGameObj, parentActor, folderPath);

	if (jsonGameObj.hasMesh())
		processMesh(workData, jsonGameObj, objId, parentActor, folderPath);
}

void JsonImporter::processMesh(ImportWorkData &workData, const JsonGameObject &jsonGameObj, int objId, AActor *parentActor, const FString& folderPath){
	if (!jsonGameObj.hasMesh())
		return;

	UE_LOG(JsonLog, Log, TEXT("Mesh found in object %d, name %s"), objId, *jsonGameObj.ueName);
	
	auto meshPath = meshIdMap[jsonGameObj.meshId];
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return;
	}

	AActor *meshActor = workData.world->SpawnActor<AActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
	if (!meshActor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn actor"));
		return;
	}

	meshActor->SetActorLabel(jsonGameObj.ueName, true);

	AStaticMeshActor *worldMesh = Cast<AStaticMeshActor>(meshActor);
	//if params is static
	if (!worldMesh){
		UE_LOG(JsonLog, Warning, TEXT("Wrong actor class"));
		return;
	}

	auto meshComp = worldMesh->GetStaticMeshComponent();
	meshComp->SetStaticMesh(meshObject);

	if (!jsonGameObj.hasRenderers()){
		UE_LOG(JsonLog, Warning, TEXT("Renderer not found on %s(%d), cannot create mesh"), *jsonGameObj.ueName, jsonGameObj.id);
		return;
	}

	const auto &renderer = jsonGameObj.renderers[0];
	auto materials = jsonGameObj.getFirstMaterials();
	if (materials.Num() > 0){
		for(int i = 0; i < materials.Num(); i++){
			auto matId = materials[i];
			UMaterial *material = loadMaterial(matId);
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

	if (jsonGameObj.isStatic)
		meshComp->SetMobility(EComponentMobility::Static);

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

	workData.objectActors.Add(jsonGameObj.id, meshActor);
	setParentAndFolder(meshActor, parentActor, folderPath);

	meshComp->SetCastShadow(hasShadows);
	meshComp->bCastShadowAsTwoSided = twoSidedShadows;

	worldMesh->MarkComponentsRenderStateDirty();
}

void JsonImporter::processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, int32 objId, AActor *parentActor, const FString &folderPath){
	if (!gameObj.hasProbes())
		return;

	if (!gameObj.isStatic){
		UE_LOG(JsonLog, Warning, TEXT("Moveable reflection captures are not supported. Object %s(%d)"), *gameObj.ueName, objId);
		//return;
	}

	for (int i = 0; i < gameObj.probes.Num(); i++){
		const auto &probe = gameObj.probes[i];

		FMatrix captureMatrix = gameObj.ueWorldMatrix;

		FVector ueCenter = unityToUe(probe.center);
		FVector ueSize = unityToUe(probe.size);
		FVector xAxis, yAxis, zAxis;
		captureMatrix.GetScaledAxes(xAxis, yAxis, zAxis);
		auto origin = captureMatrix.GetOrigin();
		origin += xAxis * ueCenter.X * 100.0f + yAxis * ueCenter.Y * 100.0f + zAxis * ueCenter.Z * 100.0f;
		xAxis *= ueSize.X;
		yAxis *= ueSize.Y;
		zAxis *= ueSize.Z;
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
		if (!probe.boxProjection){
			//auto actor = JsonObjects::createActor<ASphereReflectionCapture>(world, 
			ASphereReflectionCapture *actor = Cast<ASphereReflectionCapture>(
				GEditor->AddActor(
					workData.world->GetCurrentLevel(),
					//GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				ASphereReflectionCapture::StaticClass(), captureTransform));//spotLightTransform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn sphere capture"));
			}
			else{
				actor->SetActorLabel(gameObj.ueName);
				auto moveResult = actor->SetActorTransform(captureTransform, false, nullptr, ETeleportType::ResetPhysics);
				logValue("Actor move result: ", moveResult);

				auto captureComp = actor->GetCaptureComponent();
				reflComponent = captureComp;
				//captureComp->Brightness = intensity;
				/*if (isStatic)
					actor->SetMobility(EComponentMobility::Static);*/
				actor->MarkComponentsRenderStateDirty();
				setParentAndFolder(actor, parentActor, folderPath);
			}
		}
		else{
			ABoxReflectionCapture *actor = Cast<ABoxReflectionCapture>(
					GEditor->AddActor(
						workData.world->GetCurrentLevel(),
						//GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				ABoxReflectionCapture::StaticClass(), captureTransform));//spotLightTransform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn box reflection capture"));
			}
			else{
				actor->SetActorLabel(gameObj.ueName);
				auto moveResult = actor->SetActorTransform(captureTransform, false, nullptr, ETeleportType::ResetPhysics);
				logValue("Actor move result: ", moveResult);

				auto captureComp = actor->GetCaptureComponent();
				reflComponent = captureComp;
				//captureComp->Brightness = intensity;

				//TODO: Cubemaps
				/*if (isStatic)
					actor->SetMobility(EComponentMobility::Static);*/
				actor->MarkComponentsRenderStateDirty();
				setParentAndFolder(actor, parentActor, folderPath);
			}
		}
		if (reflComponent){
			reflComponent->Brightness = probe.intensity;
			reflComponent->ReflectionSourceType = EReflectionSourceType::CapturedScene;
			if (probe.mode == "Custom"){
				reflComponent->ReflectionSourceType = EReflectionSourceType::SpecifiedCubemap;
				UE_LOG(JsonLog, Warning, TEXT("Cubemaps are not yet fully supported: %s(%d)"), *gameObj.ueName, objId);
			}
			if (probe.mode == "Realtime"){
				UE_LOG(JsonLog, Warning, TEXT("Realtime reflection probes are not support: %s(%d)"), *gameObj.ueName, objId);
			}
			//reflComp->
		}
	}
}

void JsonImporter::processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &jsonLight, AActor *parentActor, const FString& folderPath){
	UE_LOG(JsonLog, Log, TEXT("Creating light"));
	FActorSpawnParameters spawnParams;
	FTransform spotLightTransform;
	FVector lightX, lightY, lightZ;
	gameObj.ueWorldMatrix.GetScaledAxes(lightX, lightY, lightZ);

	logValue("LightX (orig)", lightX);
	logValue("LightY (orig)", lightY);
	logValue("LightZ (orig)", lightZ);
	FVector lightNewX = lightZ;
	FVector lightNewY = lightY;
	FVector lightNewZ = -lightX;
	FMatrix lightMatrix = gameObj.ueWorldMatrix;
	logValue("lightNewX", lightNewX);
	logValue("lightNewY", lightNewY);
	logValue("lightNewZ", lightNewZ);

	logValue("lightMatrix", lightMatrix);
	lightMatrix.SetAxes(&lightNewX, &lightNewY, &lightNewZ);
	spotLightTransform.SetFromMatrix(lightMatrix);

	logValue("Transform.Translation", spotLightTransform.GetTranslation());
	logValue("Transform.Scale3D", spotLightTransform.GetScale3D());
	logValue("Transform.Rotation", spotLightTransform.GetRotation());
	logValue("Transform.XAxis", spotLightTransform.GetUnitAxis(EAxis::X));
	logValue("Transform.YAxis", spotLightTransform.GetUnitAxis(EAxis::Y));
	logValue("Transform.ZAxis", spotLightTransform.GetUnitAxis(EAxis::Z));

	if (jsonLight.lightType == "Point"){
		FTransform pointLightTransform;
		pointLightTransform.SetFromMatrix(gameObj.ueWorldMatrix);
		APointLight *actor = Cast<APointLight>(GEditor->AddActor(
			workData.world->GetCurrentLevel(),
			//GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
			APointLight::StaticClass(), pointLightTransform));//spotLightTransform));
		if (!actor){
			UE_LOG(JsonLog, Warning, TEXT("Could not spawn point light"));
		}
		else{
			actor->SetActorLabel(gameObj.ueName, true);

			auto moveResult = actor->SetActorTransform(pointLightTransform, false, nullptr, ETeleportType::ResetPhysics);
			logValue("Actor move result: ", moveResult);

			auto light = actor->PointLightComponent;
			//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity

			light->SetIntensity(jsonLight.intensity);
			light->bUseInverseSquaredFalloff = false;
			//light->LightFalloffExponent = 2.0f;
			light->SetLightFalloffExponent(2.0f);

			light->SetLightColor(jsonLight.color);
			float attenRadius = jsonLight.range*100.0f;//*ueAttenuationBoost;//those are fine
			light->AttenuationRadius = attenRadius;
			light->SetAttenuationRadius(attenRadius);
			light->CastShadows = jsonLight.castsShadows;//lightCastShadow;// != FString("None");
			//light->SetVisibility(params.visible);
			if (gameObj.isStatic)
				actor->SetMobility(EComponentMobility::Static);
			actor->MarkComponentsRenderStateDirty();

			//createdActors.Add(actor);
			setParentAndFolder(actor, parentActor, folderPath);
		}
	}
	else if (jsonLight.lightType == "Spot"){
		ASpotLight *actor = Cast<ASpotLight>(GEditor->AddActor(
			workData.world->GetCurrentLevel(),
			//GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
			ASpotLight::StaticClass(), spotLightTransform));
		if (!actor){
			UE_LOG(JsonLog, Warning, TEXT("Could not spawn spot light"));
		}
		else{
			actor->SetActorLabel(gameObj.ueName, true);

			auto light = actor->SpotLightComponent;
			//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
			light->SetIntensity(jsonLight.intensity);
			light->bUseInverseSquaredFalloff = false;
			//light->LightFalloffExponent = 2.0f;
			light->SetLightFalloffExponent(2.0f);


			light->SetLightColor(jsonLight.color);
			float attenRadius = jsonLight.range*100.0f;//*ueAttenuationBoost;
			light->AttenuationRadius = attenRadius;
			light->SetAttenuationRadius(attenRadius);
			light->CastShadows = jsonLight.castsShadows;//lightCastShadow;// != FString("None");
			//light->InnerConeAngle = lightSpotAngle * 0.25f;
			light->InnerConeAngle = 0.0f;
			light->OuterConeAngle = jsonLight.spotAngle * 0.5f;
			//light->SetVisibility(params.visible);
			if (gameObj.isStatic)
				actor->SetMobility(EComponentMobility::Static);
			actor->MarkComponentsRenderStateDirty();

			//createdActors.Add(actor);
			setParentAndFolder(actor, parentActor, folderPath);
		}
	}
	else if (jsonLight.lightType == "Directional"){
		FTransform dirLightTransform;
		dirLightTransform.SetFromMatrix(gameObj.ueWorldMatrix);//dirLightMatrix);

		ADirectionalLight *dirLightActor = Cast<ADirectionalLight>(GEditor->AddActor(
			workData.world->GetCurrentLevel(),
			//GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
			ADirectionalLight::StaticClass(), dirLightTransform));
		//Well, here we go. For some reason data from lightTransform isn't being passed.
		if (!dirLightActor){
			UE_LOG(JsonLog, Warning, TEXT("Could not spawn directional light"));
		}
		else{
			dirLightActor->SetActorLabel(gameObj.ueName, true);

			logValue("Dir light rotation (Euler): ", dirLightActor->GetActorRotation().Euler());
			logValue("Dir light transform: ", dirLightActor->GetActorTransform().ToMatrixWithScale());
			logValue("Dir light scale: ", dirLightActor->GetActorScale3D());
			logValue("Dir light location: ", dirLightActor->GetActorLocation());
			// ??? For some reason it ignores data passed through AddActor. 

			auto moveResult = dirLightActor->SetActorTransform(dirLightTransform, false, nullptr, ETeleportType::ResetPhysics);
			logValue("Actor move result: ", moveResult);

			logValue("Dir light rotation (Euler): ", dirLightActor->GetActorRotation().Euler());
			logValue("Dir light transform: ", dirLightActor->GetActorTransform().ToMatrixWithScale());
			logValue("Dir light scale: ", dirLightActor->GetActorScale3D());
			logValue("Dir light location: ", dirLightActor->GetActorLocation());

			auto light = dirLightActor->GetLightComponent();
			//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
			light->SetIntensity(jsonLight.intensity);
			//light->bUseInverseSquaredFalloff = false;
			//light->LightFalloffExponent = 2.0f;
			//light->SetLightFalloffExponent(2.0f);

			light->SetLightColor(jsonLight.color);
			//float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;
			//light->AttenuationRadius = attenRadius;
			//light->SetAttenuationRadius(attenRadius);
			light->CastShadows = jsonLight.castsShadows;// != FString("None");
			//light->InnerConeAngle = lightSpotAngle * 0.25f;

			//light->InnerConeAngle = 0.0f;
			//light->OuterConeAngle = lightSpotAngle * 0.5f;

			//light->SetVisibility(params.visible);
			if (gameObj.isStatic)
				dirLightActor->SetMobility(EComponentMobility::Static);
			dirLightActor->MarkComponentsRenderStateDirty();

			setParentAndFolder(dirLightActor, parentActor, folderPath);
		}
	}
}

void JsonImporter::processLights(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath){
	if (!gameObj.hasLights())
		return;

	for(int i = 0; i < gameObj.lights.Num(); i++){
		const auto &curLight = gameObj.lights[i];
		processLight(workData, gameObj, curLight, parentActor, folderPath);
	}
}
