#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
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

#include "DesktopPlatformModule.h"

static void setParentAndFolder(AActor *actor, AActor *parentActor, const FString& folderPath){
	if (!actor)
		return;
	if (parentActor){
		actor->AttachToActor(parentActor, FAttachmentTransformRules::KeepWorldTransform);
	}
	else{
		if (folderPath.Len())
			actor->SetFolderPath(*folderPath);
	}
}

void JsonImporter::importObject(JsonObjPtr obj, int32 objId){
	UE_LOG(JsonLog, Log, TEXT("Importing object %d"), objId);

#define GETPARAM2(varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(name, getString)
	GETPARAM(id, getInt)
	GETPARAM(instanceId, getInt)
	GETPARAM(localPosition, getVector)
	GETPARAM(localRotation, getQuat)
	GETPARAM(localScale, getVector)
	GETPARAM(worldMatrix, getMatrix)
	GETPARAM(localMatrix, getMatrix)
	GETPARAM2(parentId, parent, getInt)
	GETPARAM2(meshId, mesh, getInt)

	GETPARAM(isStatic, getBool)
	GETPARAM(lightMapStatic, getBool)
	GETPARAM(navigationStatic, getBool)
	GETPARAM(occluderStatic, getBool)
	GETPARAM(occludeeStatic, getBool)

	GETPARAM(nameClash, getBool)
	GETPARAM(uniqueName, getString)
	//renderer
	//light
#undef GETPARAM
#undef GETPARAM2
	auto rendererArray = obj->GetArrayField("renderer");
	auto lightArray = obj->GetArrayField("light");

	FString folderPath;

	if (nameClash && (uniqueName.Len() > 0)){
		UE_LOG(JsonLog, Warning, TEXT("Name clash detected on object %d: %s. Renaming to %s"), 
			id, *name, *uniqueName);		
		name = uniqueName;
	}

	AActor *parentActor = objectActors.FindRef(parentId);

	FString childFolderPath = name;
	if (parentId >= 0){
		const FString* found = objectFolderPaths.Find(parentId);
		if (found){
			folderPath = *found;
			childFolderPath = folderPath + "/" + name;
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("Object parent not found, folder path may be invalid"));
		}
	}
	UE_LOG(JsonLog, Log, TEXT("Folder path for object: %d: %s"), id, *folderPath);
	objectFolderPaths.Add(id, childFolderPath);

	bool hasRenderer = false, hasLight = false;
	bool receiveShadows = true;
	FString shadowCastingMode;

	TArray<int32> materials;
	//FVector4 lightmapScaleOffset
	if (rendererArray.Num() > 0){
		auto rendVal = rendererArray[0];
		auto rendObj = rendVal->AsObject();
		if (rendObj.IsValid()){
			hasRenderer = true;
			receiveShadows = getBool(rendObj, "receiveShadows");
			shadowCastingMode = getString(rendObj, "shadowCastingMode");
			const JsonValPtrs* matValues = 0;
			loadArray(rendObj, matValues, "materials");
			if (matValues)
				materials = toIntArray(*matValues);
		}
	}

	float lightRange = 0.0f;
	float lightSpotAngle = 0.0f;
	bool lightCastShadow = false;
	FString lightType;
	float lightShadowStrength = 0.0f;
	float lightIntensity = 0.0f;
	FLinearColor lightColor;
	float lightBounceIntensity = 0.0f;
	FString lightRenderMode;
	FString lightShadows;

	if (lightArray.Num() > 0){
		auto lightVal= lightArray[0];
		auto lightObj = lightVal->AsObject();
		if (lightObj.IsValid()){
			hasLight = true;
			lightRange = getFloat(lightObj, "range");
			lightSpotAngle = getFloat(lightObj, "spotAngle");
			lightType = getString(lightObj, "type");
			lightShadowStrength= getFloat(lightObj, "shadowStrength");
			lightIntensity = getFloat(lightObj, "intensity");
			lightBounceIntensity = getFloat(lightObj, "bounceIntensity");
			lightColor = getColor(lightObj, "color");
			lightRenderMode = getString(lightObj, "renderMode");
			lightShadows = getString(lightObj, "shadows");
			lightCastShadow = lightShadows != "Off";
		}
	}

	FVector xAxis, yAxis, zAxis;
	worldMatrix.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = worldMatrix.GetOrigin();
	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);
	FMatrix ueMatrix;
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);

	if (!hasLight && (meshId < 0))
		return;
	auto world = GEditor->GetEditorWorldContext().World();
	if (!world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return; 
	}

	const float ueAttenuationBoost = 2.0f;

	if (hasLight){
		UE_LOG(JsonLog, Log, TEXT("Creating light"));
		FActorSpawnParameters spawnParams;
		FTransform spotLightTransform;
		FVector lightX, lightY, lightZ;
		ueMatrix.GetScaledAxes(lightX, lightY, lightZ);
		logValue("LightX (orig)", lightX);
		logValue("LightY (orig)", lightY);
		logValue("LightZ (orig)", lightZ);
		FVector lightNewX = lightZ;
		FVector lightNewY = lightY;
		FVector lightNewZ = -lightX;
		FMatrix lightMatrix = ueMatrix;
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

		if (lightType == "Point"){
			FTransform pointLightTransform;
			pointLightTransform.SetFromMatrix(ueMatrix);
			APointLight *actor = Cast<APointLight>(GEditor->AddActor(GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				APointLight::StaticClass(), pointLightTransform));//spotLightTransform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn point light"));
			}
			else{
				actor->SetActorLabel(name, true);

				auto moveResult = actor->SetActorTransform(pointLightTransform, false, nullptr, ETeleportType::ResetPhysics);
				logValue("Actor move result: ", moveResult);

				auto light = actor->PointLightComponent;
				//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity

				light->SetIntensity(lightIntensity);
				light->bUseInverseSquaredFalloff = false;
				//light->LightFalloffExponent = 2.0f;
				light->SetLightFalloffExponent(2.0f);

				light->SetLightColor(lightColor);
				float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;//those are fine
				light->AttenuationRadius = attenRadius;
				light->SetAttenuationRadius(attenRadius);
				light->CastShadows = lightCastShadow;// != FString("None");
				//light->SetVisibility(params.visible);
				if (isStatic)
					actor->SetMobility(EComponentMobility::Static);
				actor->MarkComponentsRenderStateDirty();

				//createdActors.Add(actor);
				setParentAndFolder(actor, parentActor, folderPath);
			}
		}
		else if (lightType == "Spot"){
			ASpotLight *actor = Cast<ASpotLight>(GEditor->AddActor(GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				ASpotLight::StaticClass(), spotLightTransform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn spot light"));
			}
			else{
				actor->SetActorLabel(name, true);

				auto light = actor->SpotLightComponent;
				//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
				light->SetIntensity(lightIntensity);
				light->bUseInverseSquaredFalloff = false;
				//light->LightFalloffExponent = 2.0f;
				light->SetLightFalloffExponent(2.0f);


				light->SetLightColor(lightColor);
				float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;
				light->AttenuationRadius = attenRadius;
				light->SetAttenuationRadius(attenRadius);
				light->CastShadows = lightCastShadow;// != FString("None");
				//light->InnerConeAngle = lightSpotAngle * 0.25f;
				light->InnerConeAngle = 0.0f;
				light->OuterConeAngle = lightSpotAngle * 0.5f;
				//light->SetVisibility(params.visible);
				if (isStatic)
					actor->SetMobility(EComponentMobility::Static);
				actor->MarkComponentsRenderStateDirty();

				//createdActors.Add(actor);
				setParentAndFolder(actor, parentActor, folderPath);
			}
		}
		else if (lightType == "Directional"){
		#if 0
			FMatrix dirLightMatrix = ueMatrix;
			/*
			FVector lightNewX = lightZ;
			FVector lightNewY = lightY;
			FVector lightNewZ = -lightX;
			*/
			FVector dirLightZ = lightNewX; // lightZ
			FVector dirLightX = -lightNewZ; // lightX
			FVector dirLightY = lightNewY; // lightY
			dirLightMatrix.SetAxes(&dirLightX, &dirLightY, &dirLightZ);
		#endif

			FTransform dirLightTransform;
			dirLightTransform.SetFromMatrix(ueMatrix);//dirLightMatrix);

			ADirectionalLight *dirLightActor = Cast<ADirectionalLight>(GEditor->AddActor(GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				ADirectionalLight::StaticClass(), dirLightTransform));
			//Well, here we go. For some reason data from lightTransform isn't being passed.
			if (!dirLightActor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn directional light"));
			}
			else{
				dirLightActor->SetActorLabel(name, true);

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
				light->SetIntensity(lightIntensity);
				//light->bUseInverseSquaredFalloff = false;
				//light->LightFalloffExponent = 2.0f;
				//light->SetLightFalloffExponent(2.0f);


				light->SetLightColor(lightColor);
				//float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;
				//light->AttenuationRadius = attenRadius;
				//light->SetAttenuationRadius(attenRadius);
				light->CastShadows = lightCastShadow;// != FString("None");
				//light->InnerConeAngle = lightSpotAngle * 0.25f;

				//light->InnerConeAngle = 0.0f;
				//light->OuterConeAngle = lightSpotAngle * 0.5f;

				//light->SetVisibility(params.visible);
				if (isStatic)
					dirLightActor->SetMobility(EComponentMobility::Static);
				dirLightActor->MarkComponentsRenderStateDirty();

				setParentAndFolder(dirLightActor, parentActor, folderPath);
			}
		}
	}

	if (meshId < 0)
		return;

	UE_LOG(JsonLog, Log, TEXT("Mesh found in object %d, name %s"), objId, *name);
	
	//ueMatrix.SetScaledAxes(zAxis, xAxis, yAxis);
	//ueMatrix.SetOrigin(pos);

	//visible/static parameters
	auto meshPath = meshIdMap[meshId];
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(ueMatrix);

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return;
	}

	AActor *meshActor = world->SpawnActor<AActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
	if (!meshActor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn actor"));
		return;
	}

	meshActor->SetActorLabel(name, true);

	AStaticMeshActor *worldMesh = Cast<AStaticMeshActor>(meshActor);
	//if params is static
	if (!worldMesh){
		UE_LOG(JsonLog, Warning, TEXT("Wrong actor class"));
		return;
	}

	auto meshComp = worldMesh->GetStaticMeshComponent();
	meshComp->SetStaticMesh(meshObject);
	//meshComp->StaticMesh = meshObject;

	//materials
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
	if (shadowCastingMode == FString("ShadowsOnly")){
		twoSidedShadows = false;
		hasShadows = true;
		hideInGame = true;
	}
	else if (shadowCastingMode == FString("On")){
		hasShadows = true;
		twoSidedShadows = false;
	}
	else if (shadowCastingMode == FString("TwoSided")){
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

	if (isStatic)
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

	objectActors.Add(id, meshActor);
	setParentAndFolder(meshActor, parentActor, folderPath);

	meshComp->SetCastShadow(hasShadows);
	meshComp->bCastShadowAsTwoSided = twoSidedShadows;

	worldMesh->MarkComponentsRenderStateDirty();
}
