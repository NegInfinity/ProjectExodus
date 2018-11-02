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

using namespace UnrealUtilities;

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

ULandscapeLayerInfoObject* JsonImporter::createTerrainLayerInfo(ImportWorkData &workData, 
		const JsonGameObject &jsonGameObj, const JsonTerrainData &terrainData, int layerIndex){

	FString terrPath, terrFileName, terrExt;
	FPaths::Split(terrainData.exportPath, terrPath, terrFileName, terrExt);

	auto layerName = terrainData.getLayerName(layerIndex);//genTerrainLayerName(terrainData, layerIndex);
	auto basePackageName = terrainData.name + FString("_") + layerName;

	auto importPath = getProjectImportPath();//getDefaultImportPath();
	auto layerPackagePath = buildPackagePath(basePackageName, 
		&terrPath, &importPath, &assetCommonPath);


	UE_LOG(JsonLogTerrain, Log, TEXT("Creating package for layer %d of terrain \"%s\" at \"%s\""), 
		layerIndex, *terrainData.name, *layerPackagePath);

	auto layerPackage = CreatePackage(0, *layerPackagePath);

	auto layerObj = //NewObject<ULandscapeLayerInfoObject>(layerPackage);
		NewObject<ULandscapeLayerInfoObject>(layerPackage, ULandscapeLayerInfoObject::StaticClass(), *sanitizeObjectName(*layerName), 
			RF_Standalone|RF_Public);
	//newLayer.LayerInfo = layerObj;
	layerObj->LayerName = *layerName;

	int32 colIndex = 1 + layerIndex % 7;
	layerObj->LayerUsageDebugColor = FLinearColor(
		(colIndex & 0x1) ? 1.0f: 0.0f,
		(colIndex & 0x2) ? 1.0f: 0.0f,
		(colIndex & 0x4) ? 1.0f: 0.0f
	);
			
	layerObj->SetFlags(RF_Standalone|RF_Public);

	if (layerObj){
		FAssetRegistryModule::AssetCreated(layerObj);
		layerPackage->SetDirtyFlag(true);
	}

	return layerObj;
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

	JsonBinaryTerrain binaryTerrain;
	auto fullExportPath = FPaths::Combine(assetRootPath, terrainData->exportPath);
	if (!binaryTerrain.load(fullExportPath)){
		UE_LOG(JsonLogTerrain, Error, TEXT("Could not load binary terrain \"%s\", aborting"), *fullExportPath);
	}
	JsonConvertedTerrain convertedTerrain;
	convertedTerrain.assignFrom(binaryTerrain);

	const auto& heightMapData = convertedTerrain.heightMap;

	ALandscape * result = nullptr;
	const int32 xComps = 1;
	const int32 yComps = 1;
	const int32 quadsPerSection = 63;
	const int32 sectionsPerComp = 1;
	const int32 quadsPerComp = quadsPerSection * sectionsPerComp;
	int32 xSize = xComps * quadsPerComp + 1;
	int32 ySize = yComps * quadsPerComp + 1;

	xSize = heightMapData.getWidth();
	ySize = heightMapData.getHeight();

	TArray<FLandscapeImportLayerInfo> importLayers;
	if (convertedTerrain.alphaMaps.Num() > 0){
		for(int i = 0; i < convertedTerrain.alphaMaps.Num(); i++){
			auto layerName = terrainData->getLayerName(i);
			auto layerInfoObj = createTerrainLayerInfo(workData, jsonGameObj, *terrainData, i);

			auto &newLayer = importLayers.AddDefaulted_GetRef();
			newLayer.LayerName = *layerName;
			newLayer.LayerData = convertedTerrain.alphaMaps[i].getArrayCopy();
			newLayer.LayerInfo = layerInfoObj;
			newLayer.SourceFilePath = TEXT("");
		}
	}

	UMaterial *terrainMaterial = materialBuilder.buildTerrainMaterial(jsonGameObj, jsonTerrain, *terrainData, this);

	FTransform terrainTransform;
	FMatrix terrainMatrix = jsonGameObj.ueWorldMatrix;

	auto ueWorldSize = unitySizeToUe(terrainData->worldSize);
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain size: %f %f %f"), ueWorldSize.X, ueWorldSize.Y, ueWorldSize.Z);
	auto halfWorldSize = ueWorldSize * 0.5f;

	FVector vTerX, vTerY, vTerZ, vTerPos;
	/* 
		Oh, this is interesting. 
		Terrain in unity is permanently axis aligned and can be neither rotated nor scald. 
	*/
	terrainMatrix.GetScaledAxes(vTerX, vTerY, vTerZ);
	vTerX = FVector(1.0f, 0.0f, 0.0f);
	vTerY = FVector(0.0f, 1.0f, 0.0f);
	vTerZ = FVector(0.0f, 0.0f, 1.0f);
	vTerPos = terrainMatrix.GetOrigin();

	/*
	Default wiki says that landscape scale at 100 is 1 meter per unit and -256+256 range. Sooo....
	*/
	auto terrainScale = FVector::ZeroVector;//halfWorldSize * 0.001f;
	logValue(TEXT("Terrain ueWorldSize: "), ueWorldSize);
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain xSize: %d; ySize: %d"), xSize, ySize);
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain hMap width: %d; height: %d"), heightMapData.getWidth(), heightMapData.getHeight());
	auto sizeDefault = FVector(100.0f * (float)(xSize - 1), 100.0f * (float)(ySize - 1), 25600.0f * 2.0f);
	terrainScale = 100.0f * ueWorldSize / sizeDefault;
	//terrainScale.X = 100.0f * (ueWorldSize.X / 100.0f);
	FVector terrainOffset = FVector::ZeroVector;

	logValue(TEXT("Terrain offset: "), terrainOffset);
	terrainOffset += vTerZ * ueWorldSize.Z * 0.5f;// / 100.0f;
	logValue(TEXT("Terrain offset: "), terrainOffset);

	vTerX *= terrainScale.X;
	vTerY *= terrainScale.Y;
	vTerZ *= terrainScale.Z;

	vTerPos += terrainOffset;

	logValue(TEXT("terrainX: "), vTerX);
	logValue(TEXT("terrainY: "), vTerY);
	logValue(TEXT("terrainZ: "), vTerZ);
	logValue(TEXT("terrainPos: "), vTerPos);

	terrainMatrix.SetAxes(&vTerX, &vTerY, &vTerZ, &vTerPos);

	terrainTransform.SetFromMatrix(terrainMatrix);

	result = workData.world->SpawnActor<ALandscape>(ALandscape::StaticClass());
	result->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((xSize * ySize) / (2048 * 2048) + 1), (uint32)2);//?

	auto guid = FGuid::NewGuid();
	auto *landProxy = Cast<ALandscapeProxy>(result);
	landProxy->SetLandscapeGuid(guid);

	landProxy->LandscapeMaterial = terrainMaterial;

	landProxy->Import(FGuid::NewGuid(),
		0, 0, xSize - 1, ySize - 1, sectionsPerComp, quadsPerSection, heightMapData.getData(), 
		TEXT(""), importLayers, ELandscapeImportAlphamapType::Additive);

	for(int i = 0; i < importLayers.Num(); i++){
		auto &curLayer = importLayers[i];
		UE_LOG(JsonLogTerrain, Log, TEXT("Checking layer %d: %x"), i, curLayer.LayerInfo);
	}

	ULandscapeInfo *landscapeInfo  = result->CreateLandscapeInfo();
	landscapeInfo->UpdateLayerInfoMap(result);

	result->SetActorTransform(terrainTransform);

	setParentAndFolder(result, parentActor, folderPath, workData);

	//return result;
}
