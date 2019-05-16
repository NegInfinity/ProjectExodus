#include "JsonImportPrivatePCH.h"
#include "LightBuilder.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "UnrealUtilities.h"

void LightBuilder::setupPointLightComponent(UPointLightComponent *pointLight, const JsonLight &jsonLight){
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

void LightBuilder::setupSpotLightComponent(USpotLightComponent *spotLight, const JsonLight &jsonLight){
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

void LightBuilder::setupDirLightComponent(ULightComponent *dirLight, const JsonLight &jsonLight){
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

ImportedObject LightBuilder::processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &jsonLight, ImportedObject *parentObject, const FString& folderPath){
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

void LightBuilder::processLights(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath, ImportedObjectArray *createdObjects){
	using namespace UnrealUtilities;
	if (!gameObj.hasLights())
		return;

	for(int i = 0; i < gameObj.lights.Num(); i++){
		const auto &curLight = gameObj.lights[i];
		//processLight(workData, gameObj, curLight, parentActor, folderPath);
		auto light = processLight(workData, gameObj, curLight, parentObject, folderPath);
		registerImportedObject(createdObjects, light);
	}
}

