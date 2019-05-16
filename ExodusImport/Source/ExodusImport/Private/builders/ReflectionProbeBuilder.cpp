#include "JsonImportPrivatePCH.h"
#include "ReflectionProbeBuilder.h"
#include "Engine/SphereReflectionCapture.h"
#include "Engine/BoxReflectionCapture.h"
#include "Engine/Classes/Components/BoxReflectionCaptureComponent.h"
#include "Engine/Classes/Components/ReflectionCaptureComponent.h"
#include "Engine/Classes/Components/SphereReflectionCaptureComponent.h"
#include "UnrealUtilities.h"
#include "JsonObjects/loggers.h"
#include "JsonImporter.h"

//void setupReflectionCapture(UReflectionCaptureComponent *reflComponent, const JsonReflectionProbe &probe);
ImportedObject ReflectionProbeBuilder::processReflectionProbe(ImportWorkData &workData, const JsonGameObject &gameObj,
	const JsonReflectionProbe &probe, ImportedObject *parentObject, const FString &folderPath, JsonImporter *importer){
	using namespace UnrealUtilities;
	using namespace JsonObjects;

	check(importer);
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
		UE_LOG(JsonLog, Warning, TEXT("Realtime reflections are not supported. object %s(%d)"), *gameObj.ueName, gameObj.id);
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
			auto cube = importer->getCubemap(probe.customCubemapId);
			if (!cube){
				UE_LOG(JsonLog, Warning, TEXT("Custom cubemap not set on reflection probe on object \"%s\"(%d)"),
					*gameObj.ueName, gameObj.id);
			}
			else
				reflComponent->Cubemap = cube;
			//UE_LOG(JsonLog, Warning, TEXT("Cubemaps are not yet fully supported: %s(%d)"), *gameObj.ueName, objId);
		}
		if (probe.mode == "Realtime"){
			UE_LOG(JsonLog, Warning, TEXT("Realtime reflection probes are not support: %s(%d)"), *gameObj.ueName, gameObj.id);
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

void ReflectionProbeBuilder::processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, 
		const FString &folderPath, ImportedObjectArray *createdObjects, JsonImporter *importer){
	using namespace UnrealUtilities;
	if (!gameObj.hasProbes())
		return;

	if (!gameObj.isStatic){
		UE_LOG(JsonLog, Warning, TEXT("Moveable reflection captures are not supported. Object %s(%d)"), *gameObj.ueName, gameObj.id);
	}

	for (int i = 0; i < gameObj.probes.Num(); i++){
		const auto &probe = gameObj.probes[i];
		auto probeObject = processReflectionProbe(workData, gameObj, gameObj.probes[i], parentObject, folderPath, importer);
		registerImportedObject(createdObjects, probeObject);
	}
}

