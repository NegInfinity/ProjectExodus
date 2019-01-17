#include "JsonImportPrivatePCH.h"

#if 0
ALandscape* createDefaultLandscape(UWorld *world){
	ALandscape * result = nullptr;
	const int32 xComps = 1;
	const int32 yComps = 1;
	const int32 quadsPerSection = 63;
	const int32 sectionsPerComp = 1;
	const int32 quadsPerComp = quadsPerSection * sectionsPerComp;
	int32 xSize = xComps * quadsPerComp + 1;
	int32 ySize = yComps * quadsPerComp + 1;

	DataPlane2D<uint16> hMap;
	hMap.resize(xSize, ySize);	
	auto* dstPtr = hMap.getData();
	for(int i = 0; i < hMap.getNumElements(); i++){
		dstPtr[i] = 32767;
	}

	TArray<FLandscapeImportLayerInfo> importLayers;

	FTransform transform; 
	FActorSpawnParameters spawnParams;
	transform.SetFromMatrix(FMatrix::Identity);

	result = world->SpawnActor<ALandscape>(ALandscape::StaticClass());
	if (!result){
		UE_LOG(JsonLogTerrain, Error, TEXT("Failed to spawn landscape"));
		return result; 
	}
	result->SetActorRelativeScale3D(FVector::OneVector);
		
	result->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((xSize * ySize) / (2048 * 2048) + 1), (uint32)2);//?

	auto guid = FGuid::NewGuid();
	auto *landProxy = Cast<ALandscapeProxy>(result);
	landProxy->SetLandscapeGuid(guid);

	landProxy->Import(FGuid::NewGuid(),
		0, 0, xSize - 1, ySize - 1, sectionsPerComp, quadsPerSection, hMap.getData(), 
		TEXT(""), importLayers, ELandscapeImportAlphamapType::Additive);

	ULandscapeInfo *landscapeInfo  = result->CreateLandscapeInfo();
	landscapeInfo->UpdateLayerInfoMap(result);

	result->MarkComponentsRenderStateDirty();
	result->PostLoad();

	return result;
}


ALandscape* createDefaultLandscape2(UWorld *world, UMaterial *landMaterial){
	ALandscape * result = nullptr;
	const int32 xComps = 1;
	const int32 yComps = 1;
	const int32 quadsPerSection = 63;
	const int32 sectionsPerComp = 1;
	const int32 quadsPerComp = quadsPerSection * sectionsPerComp;
	int32 xSize = xComps * quadsPerComp + 1;
	int32 ySize = yComps * quadsPerComp + 1;

	TArray<uint16> hMap;
	hMap.SetNumUninitialized(xSize * ySize);
	for(int i = 0; i < hMap.Num(); i++){
		hMap[i] = 32768;
	}
	
	TArray<FLandscapeImportLayerInfo> importLayers;

	result = world->SpawnActor<ALandscape>(ALandscape::StaticClass());
	result->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((xSize * ySize) / (2048 * 2048) + 1), (uint32)2);//?

	auto guid = FGuid::NewGuid();
	auto *landProxy = Cast<ALandscapeProxy>(result);
	landProxy->SetLandscapeGuid(guid);

	landProxy->Import(FGuid::NewGuid(),
		0, 0, xSize - 1, ySize - 1, sectionsPerComp, quadsPerSection, hMap.GetData(), 
		TEXT(""), importLayers, ELandscapeImportAlphamapType::Additive);

	landProxy->LandscapeMaterial = landMaterial;

	ULandscapeInfo *landscapeInfo  = result->CreateLandscapeInfo();
	landscapeInfo->UpdateLayerInfoMap(result);

	return result;
}
#endif
