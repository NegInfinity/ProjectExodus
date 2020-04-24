#include "JsonImportPrivatePCH.h"
#include "LightBuilder.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "UnrealUtilities.h"
#include <utility>

void LightBuilder::setupPointLightComponent(UPointLightComponent *pointLight, const JsonLight &jsonLight){
	pointLight->SetIntensity(jsonLight.intensity);
	pointLight->bUseInverseSquaredFalloff = false;
	pointLight->SetLightFalloffExponent(2.0f);

	pointLight->SetLightColor(jsonLight.color);
	float attenRadius = jsonLight.range*100.0f;
	pointLight->AttenuationRadius = attenRadius;
	pointLight->SetAttenuationRadius(attenRadius);
	pointLight->CastShadows = jsonLight.castsShadows;//lightCastShadow;// != FString("None");
}

void LightBuilder::setupSpotLightComponent(USpotLightComponent *spotLight, const JsonLight &jsonLight){
	spotLight->SetIntensity(jsonLight.intensity);
	spotLight->bUseInverseSquaredFalloff = false;
	spotLight->SetLightFalloffExponent(2.0f);

	spotLight->SetLightColor(jsonLight.color);
	float attenRadius = jsonLight.range*100.0f;
	spotLight->AttenuationRadius = attenRadius;
	spotLight->SetAttenuationRadius(attenRadius);
	spotLight->CastShadows = jsonLight.castsShadows;
	spotLight->InnerConeAngle = 0.0f;
	spotLight->OuterConeAngle = jsonLight.spotAngle * 0.5f;
	//spotLight->SetVisibility(params.visible);
}

void LightBuilder::setupDirLightComponent(ULightComponent *dirLight, const JsonLight &jsonLight){
	dirLight->SetIntensity(jsonLight.intensity);
	dirLight->SetLightColor(jsonLight.color);
	dirLight->CastShadows = jsonLight.castsShadows;
}

template <typename ActorClass, typename ComponentClass> 
std::pair<ActorClass*, ComponentClass*> createLightActorAndComponent(
		ImportContext &workData,
		FTransform lightTransform, 
		std::function<ComponentClass*(ActorClass*)> componentGetter, 
		std::function<void(ComponentClass*)> componentConfigurator,
		const TCHAR* lightName, bool createActors, std::function<UObject * ()> outerCreator){

	check(componentGetter);
	check(lightName != nullptr);

	ActorClass *lightActor = nullptr;
	ComponentClass *lightComponent = nullptr;

	if (createActors){
		lightActor = UnrealUtilities::createActor<ActorClass>(workData, lightTransform, TEXT("point light"));
		lightComponent = componentGetter(lightActor);
	}
	else{
		if (outerCreator) {
			auto outer = outerCreator();
			lightComponent = NewObject<ComponentClass>(outer);
		}
		else
			lightComponent = NewObject<ComponentClass>();
		check(lightComponent);
		lightComponent->SetWorldTransform(lightTransform);
	}
	if (componentConfigurator && lightComponent)
		componentConfigurator(lightComponent);

	return std::make_pair(lightActor, lightComponent);
}

ImportedObject LightBuilder::processLight(ImportContext &workData, const JsonGameObject &gameObj, const JsonLight &jsonLight, ImportedObject *parentObject,
		const FString& folderPath, bool createActors, std::function<UObject*()> outerCreator){
	using namespace UnrealUtilities;

	UE_LOG(JsonLog, Log, TEXT("Creating light"));

	FTransform lightTransform;
	lightTransform.SetFromMatrix(gameObj.ueWorldMatrix);

	ALight *lightActor = nullptr;
	USceneComponent *lightComponent = nullptr;
	if (jsonLight.lightType == "Point"){
		auto data = createLightActorAndComponent<APointLight, UPointLightComponent>(
			workData, lightTransform, 
			[](auto arg){return arg->PointLightComponent;}, 
			[&](auto arg){setupPointLightComponent(arg, jsonLight);},
			TEXT("point light"), createActors, outerCreator
		);
		lightActor = data.first;
		lightComponent = data.second;
	}
	else if (jsonLight.lightType == "Spot"){
		auto data = createLightActorAndComponent<ASpotLight, USpotLightComponent>(
			workData, lightTransform, 
			[](auto arg){return arg->SpotLightComponent;}, 
			[&](auto arg){setupSpotLightComponent(arg, jsonLight);},
			TEXT("spot light"), createActors, outerCreator
		);
		lightActor = data.first;
		lightComponent = data.second;
	}
	else if (jsonLight.lightType == "Directional"){
		auto data = createLightActorAndComponent<ADirectionalLight, ULightComponent>(
			workData, lightTransform, 
			[](auto arg){return arg->GetLightComponent();}, 
			[&](auto arg){setupDirLightComponent(arg, jsonLight);},
			TEXT("directional light"), createActors, outerCreator
		);
		lightActor = data.first;
		lightComponent = data.second;
	}

	if (lightActor){
		lightActor->SetActorLabel(gameObj.ueName, true);
		if (gameObj.isStatic)
			lightActor->SetMobility(EComponentMobility::Static);
		setObjectHierarchy(ImportedObject(lightActor), parentObject, folderPath, workData, gameObj);
		lightActor->MarkComponentsRenderStateDirty();
	}
	else if (lightComponent){
		if (gameObj.isStatic)
			lightComponent->SetMobility(EComponentMobility::Static);
		auto compName = FString::Printf(TEXT("%s_light(%d_%llu)"), *gameObj.ueName, gameObj.id, workData.getUniqueUint());
		lightComponent->Rename(*compName);
	}

	check(lightActor || lightComponent);
	if (lightActor)
		return ImportedObject(lightActor);
	return ImportedObject(lightComponent);
	//return ImportedObject(lightActor, lightComponent);
}

void LightBuilder::processLights(ImportContext &workData, const JsonGameObject &gameObj, ImportedObject *parentObject,
		const FString& folderPath, ImportedObjectArray *createdObjects, bool createActors, std::function<UObject*()> outerCreator){
	using namespace UnrealUtilities;
	if (!gameObj.hasLights())
		return;

	for(int i = 0; i < gameObj.lights.Num(); i++){
		const auto &curLight = gameObj.lights[i];
		//processLight(workData, gameObj, curLight, parentActor, folderPath);
		auto light = processLight(workData, gameObj, curLight, parentObject, folderPath, createActors, outerCreator);
		registerImportedObject(createdObjects, light);
	}
}

