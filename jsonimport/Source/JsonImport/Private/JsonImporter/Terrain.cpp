#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "JsonObjects/JsonBinaryTerrain.h"

#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"

#include "JsonObjects/DataPlane2D.h"
#include "JsonObjects/DataPlane3D.h"
#include "JsonObjects/utilities.h"
#include "JsonObjects/converters.h"
#include "ImportWorkData.h"
#include "UnrealUtilities.h"

#include "TerrainBuilder.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

void JsonImporter::processTerrains(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath){
	UE_LOG(JsonLog, Log, TEXT("Processing terrains for object %d"), gameObj.id);
	for(const auto& cur: gameObj.terrains){
		processTerrain(workData, gameObj, cur, parentActor, folderPath);
	}
}

ALandscape* JsonImporter::createDefaultLandscape(ImportWorkData &workData, const JsonGameObject &jsonGameObj){
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

	result = workData.world->SpawnActor<ALandscape>(ALandscape::StaticClass());
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

ALandscape* JsonImporter::createDefaultLandscape(UWorld *world){
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

	ULandscapeInfo *landscapeInfo  = result->CreateLandscapeInfo();
	landscapeInfo->UpdateLayerInfoMap(result);

	return result;
}

void JsonImporter::processTerrain(ImportWorkData &workData, const JsonGameObject &jsonGameObj, const JsonTerrain &jsonTerrain, 
		AActor *parentActor, const FString& folderPath){

	auto dataId = jsonTerrain.terrainDataId;
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain data id found: %d"), dataId);

	auto terrainData = terrainDataMap.Find(dataId);
	if (!terrainData){
		UE_LOG(JsonLogTerrain, Warning, TEXT("Terrain data could not be found for id: %d"), dataId);
		return;
	}
	UE_LOG(JsonLogTerrain, Log, TEXT("Export path: \"%s\""), *(terrainData->exportPath));

	TerrainBuilder terrainBuilder(workData, this, jsonGameObj, jsonTerrain, *terrainData);
	auto builtTerrain = terrainBuilder.buildTerrain();

	if (!builtTerrain){
		UE_LOG(JsonLogTerrain, Error, TEXT("Failed to build terrain \"%s\""), *(terrainData->exportPath));
		return;
	}
	builtTerrain->PostEditChange();

	setParentAndFolder(builtTerrain, parentActor, folderPath, workData);

	//return builtTerrain;
}
