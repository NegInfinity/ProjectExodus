#include "JsonImportPrivatePCH.h"
#include "TerrainBuilder.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "JsonImporter.h"
//#include "JsonObjects/.h"
#include "UnrealUtilities.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"

using namespace JsonObjects;
using namespace UnrealUtilities;

TerrainBuilder::TerrainBuilder(ImportWorkData &workData_, JsonImporter *importer_, const JsonGameObject &gameObj_, 
	const JsonTerrain &jsonTerrain_, const JsonTerrainData &terrainData_)
:workData(workData_), importer(importer_), jsonGameObj(gameObj_), jsonTerrain(jsonTerrain_), terrainData(terrainData_){
}

ULandscapeGrassType* TerrainBuilder::createGrassType(int layerIndex, const FString &terrainDataPth){
	return nullptr;
}

ULandscapeLayerInfoObject* TerrainBuilder::createTerrainLayerInfo(int layerIndex, bool grassLayer, 
		const FString &terrainDataPath){

	auto layerName = grassLayer ? terrainData.getGrassLayerName(layerIndex): terrainData.getLayerName(layerIndex);
	auto basePackageName = layerName;

	auto importPath = importer->getProjectImportPath();
	auto assetCommonPath = importer->getAssetCommonPath();

	auto layerPackagePath = buildPackagePath(basePackageName, 
		&terrainDataPath, &importPath, &assetCommonPath);

	UE_LOG(JsonLogTerrain, Log, TEXT("Creating package for layer %d of terrain \"%s\" at \"%s\""), 
		layerIndex, *terrainData.name, *layerPackagePath);

	auto layerPackage = CreatePackage(0, *layerPackagePath);

	auto layerObj = 
		NewObject<ULandscapeLayerInfoObject>(layerPackage, ULandscapeLayerInfoObject::StaticClass(), *sanitizeObjectName(*layerName), 
			RF_Standalone|RF_Public);
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


void TerrainBuilder::processFoliageTreeActors(ALandscape *landscape){
	UE_LOG(JsonLogTerrain, Log, TEXT("Processing terrain mesh actors. %d types, %d instances"), terrainData.treePrototypes.Num(), terrainData.treeInstances.Num());

	auto* ifa = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(workData.world, true);

	FString terrPath, terrFileName, terrExt;
	FPaths::Split(terrainData.exportPath, terrPath, terrFileName, terrExt);

	TMap<int32, UFoliageType*> foliageTypes;
	TMap<int32, FFoliageMeshInfo*> foliageMeshInfos;
	//TMap<int32, UFoliageType*> foliages;

	//TMap<JsonId, 
	int prototypeIndex = 0;
	for(const auto& curProtoTree: terrainData.treePrototypes){
		auto curProtoIdx = prototypeIndex++;
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing tree prototype %d. meshId %d; prefabId %d; prefabObj %d"),
			curProtoIdx, curProtoTree.meshId, curProtoTree.prefabId, curProtoTree.prefabObjectId);

		if (!isValidId(curProtoTree.meshId)){
			UE_LOG(JsonLogTerrain, Log, TEXT("Prototype %d has no mesh, nothing to do"), curProtoIdx);
			continue;
		}
		auto meshId = curProtoTree.meshId;

		auto treeMesh = importer->loadStaticMeshById(meshId);
		if (!treeMesh){
			UE_LOG(JsonLogTerrain, Warning, TEXT("Mesh %d could not be loaded for prototype %d"), meshId, curProtoIdx);
		}

		UFoliageType *foliageInfo = nullptr;
		auto meshInfo = ifa->AddMesh(treeMesh, &foliageInfo);
		foliageTypes.Add(curProtoIdx, foliageInfo);
		foliageMeshInfos.Add(curProtoIdx, meshInfo);
		//for(const auto& curInstance: terrainData.treeInstances)
	}

	auto objPos = jsonGameObj.ueWorldMatrix.GetOrigin();

	for(int instanceIndex = 0; instanceIndex < terrainData.treeInstances.Num(); instanceIndex++){
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing tree instance %d"), instanceIndex);
		auto srcInst = terrainData.treeInstances[instanceIndex];
		auto protoIndex = srcInst.prototypeIndex;
		auto foundFoliage = foliageTypes.Find(protoIndex);
		auto *foundMeshInfo = foliageMeshInfos.Find(protoIndex);
		if (!foundFoliage || !foundMeshInfo){
			UE_LOG(JsonLogTerrain, Warning, TEXT("Could not find foliage %d for instance %d while processing terrain %s"),
				protoIndex, instanceIndex, *terrainData.name);
			continue;
		}
		
		auto meshInfo = *foundMeshInfo;
		auto foliageType = *foundFoliage;

		auto srcPos = srcInst.position;
		UE_LOG(JsonLogTerrain, Log, TEXT("Src coord %f %f %f"), srcPos.X, srcPos.Y, srcPos.Z);

		FFoliageInstance dstInst;

		auto scale = FVector(srcInst.widthScale, srcInst.widthScale, srcInst.heightScale);
		//FVector unityScale(srcInst.widthScale, srcInst.he

		dstInst.DrawScale3D = scale * 0.1f; //??why??

		auto treePos = terrainData.getNormalizedPosAsWorld(srcInst.position, objPos);
		UE_LOG(JsonLogTerrain, Log, TEXT("Dst coord %f %f %f"), treePos.X, treePos.Y, treePos.Z);
		//Hmm. Instance coordinates are within 0..1 range on terrain. 
		dstInst.Location = treePos;//unityPosToUe(treePos);
		meshInfo->AddInstance(ifa, foliageType, dstInst, true);
	}
}

ALandscape* TerrainBuilder::buildTerrain(){
	FString terrPath, terrFileName, terrExt;
	FPaths::Split(terrainData.exportPath, terrPath, terrFileName, terrExt);
	auto terrainDataPath = FPaths::Combine(terrPath, terrFileName + TEXT("_Data"));

	FString assetRootPath = importer->getAssetRootPath();

	JsonBinaryTerrain binaryTerrain;
	auto fullExportPath = FPaths::Combine(assetRootPath, terrainData.exportPath);
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
	//normal layers
	if (convertedTerrain.alphaMaps.Num() > 0){
		for(int i = 0; i < convertedTerrain.alphaMaps.Num(); i++){
			auto layerName = terrainData.getLayerName(i);
			auto layerInfoObj = createTerrainLayerInfo(i, false, terrainDataPath);

			auto &newLayer = importLayers.AddDefaulted_GetRef();
			newLayer.LayerName = *layerName;
			newLayer.LayerData = convertedTerrain.alphaMaps[i].getArrayCopy();
			newLayer.LayerInfo = layerInfoObj;
			newLayer.SourceFilePath = TEXT("");
		}
	}

	//grass
	TArray<ULandscapeGrassType*> grassTypes;
	if (convertedTerrain.detailMaps.Num() > 0){
		for(int i = 0; i < convertedTerrain.detailMaps.Num(); i++){
			auto layerName = terrainData.getGrassLayerName(i);
			auto layerInfoObj = createTerrainLayerInfo(i, true, terrainDataPath);

			auto &newLayer = importLayers.AddDefaulted_GetRef();
			newLayer.LayerName = *layerName;
			newLayer.LayerData = convertedTerrain.detailMaps[i].getArrayCopy();
			newLayer.LayerInfo = layerInfoObj;
			newLayer.SourceFilePath = TEXT("");

			auto newGrassType = createGrassType(i, terrainDataPath);
			grassTypes.Add(newGrassType);
		}
	}


	auto terrainVertSize = FIntPoint(xSize, ySize);
	MaterialBuilder materialBuilder;
	UMaterial *terrainMaterial = materialBuilder.buildTerrainMaterial(jsonGameObj, jsonTerrain, terrainData, terrainVertSize, terrainDataPath, importer);

	FTransform terrainTransform;
	FMatrix terrainMatrix = jsonGameObj.ueWorldMatrix;

	auto ueWorldSize = unitySizeToUe(terrainData.worldSize);
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

	processFoliageTreeActors(result);

	return result;
}
