#include "JsonImportPrivatePCH.h"
#include "TerrainBuilder.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "JsonImporter.h"

#include "UnrealUtilities.h"
#include "MeshBuilder.h"
#include "Materials/Material.h"
#include "JsonObjects/JsonBinaryTerrain.h"

#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Runtime/Landscape/Classes/LandscapeGrassType.h"

using namespace JsonObjects;
using namespace UnrealUtilities;

TerrainBuilder::TerrainBuilder(ImportContext &workData_, JsonImporter *importer_, const JsonGameObject &gameObj_, 
	const JsonTerrain &jsonTerrain_, const JsonTerrainData &terrainData_)
:importer(importer_), workData(workData_), jsonGameObj(gameObj_), jsonTerrain(jsonTerrain_), terrainData(terrainData_){
}

UStaticMesh* TerrainBuilder::createClonedMesh(const JsonMesh &jsonMesh, const FString &baseName, const FString &terrainDataPath, 
		const IntArray &matIds, 
		std::function<void(UMaterialInstanceConstant *matInst, FStaticParameterSet &statParams)> matInstCallback){
	MaterialBuilder matBuilder;
	TArray<UMaterialInterface*> matInstances;

	//duplicating materials and making them tint
	const auto& meshMatIds = matIds;//detPrototype.detailMeshMaterials;
	for(int i = 0; i < meshMatIds.Num(); i++){
		auto matId = meshMatIds[i];
		auto jsonMat = importer->getJsonMaterial(matId);
		if (!jsonMat){
			matInstances.Add(nullptr);
			continue;
		}

		auto matName = FString::Printf(TEXT("%s_mat%d"), *baseName, i);
		matName = sanitizePackageName(matName);
		auto baseMat = matBuilder.getBaseMaterial(*jsonMat);
		auto assetPath = FPaths::Combine(terrainDataPath, matName);

		//auto matInst = matBuilder.createMaterialInstance(matName, &terrainDataPath, baseMat, importer, 
		auto matInst = matBuilder.createMaterialInstance(matName, &assetPath, baseMat, importer, 
			[&](UMaterialInstanceConstant* newInst){
				if (!newInst)
					return;
				if (!matInstCallback)
					return;
				FStaticParameterSet statParams;
				newInst->GetStaticParameterValues(statParams);
				matInstCallback(newInst, statParams);
				newInst->UpdateStaticPermutation(statParams);
			}
		);
		
		matInstances.Add(matInst);
	}

	//auto jsonMesh = importer->loadJsonMesh(detPrototype.detailMeshId);
	auto meshName = sanitizePackageName(baseName);

	auto result = createAssetObject<UStaticMesh>(meshName, &terrainDataPath, importer, 
		[&](UStaticMesh *mesh){
			MeshBuilder builder;
			builder.setupStaticMesh(mesh, jsonMesh, 
				[&](TArray<FStaticMaterial> &meshMaterials){
					meshMaterials.Empty();
					for(auto cur: matInstances){
						meshMaterials.Add(cur);
					}
				}
			);
		},
		[&](auto pkg, auto sanitizedName){
			return NewObject<UStaticMesh>(pkg, FName(*sanitizedName), RF_Standalone|RF_Public);
		}
	);
	return result;
}

UStaticMesh* TerrainBuilder::createGrassMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, int layerIndex, const FString &terrainDataPath){
	auto jsonMesh = importer->loadJsonMesh(detPrototype.detailMeshId);
	auto meshName = FString::Printf(TEXT("layer%d_mesh"), layerIndex);

	return createClonedMesh(jsonMesh, meshName, terrainDataPath, detPrototype.detailMeshMaterials, 
		[&](UMaterialInstanceConstant *matInst, FStaticParameterSet &statParams){
			MaterialBuilder matBuilder;
			matBuilder.setStaticSwitch(statParams, "enableLandscapeColorBlending", true);
			//landscapeCellScale = scaleFactor, default 0.01
			//healthyColor
			matBuilder.setVectorParam(matInst, "healthyColor", detPrototype.healthyColor);
			//dryColor
			matBuilder.setVectorParam(matInst, "dryColor", detPrototype.dryColor);
			//noiseSpread
			matBuilder.setScalarParam(matInst, "noiseSpread", detPrototype.noiseSpread);
			//landscape cell scale?
		}
	);
}

UStaticMesh* TerrainBuilder::createBillboardMesh(const FString &baseName, const JsonTerrainDetailPrototype &detPrototype, int layerIndex, const FString &terrainDataPath){
	MaterialBuilder matBuilder;
	auto billboardMaterial = 
		matBuilder.createBillboardMatInstance(&detPrototype, layerIndex, this, terrainDataPath);

	auto result = createAssetObject<UStaticMesh>(baseName, &terrainDataPath, importer, 
		[&](UStaticMesh *mesh){
			MeshBuilder builder;
			builder.generateBillboardMesh(mesh, billboardMaterial);
			/*
			if (detPrototype.billboardFlag)
				mesh->shad*/
		},
		[&](auto pkg, auto sanitizedName){
			return NewObject<UStaticMesh>(pkg, FName(*sanitizedName), RF_Standalone|RF_Public);
		}
	);
	return result;
}

ULandscapeGrassType* TerrainBuilder::createGrassType(int layerIndex, const FString &terrainDataPath){
	auto grassTypeName = terrainData.getGrassTypeName(layerIndex);
	const auto &srcType = terrainData.detailPrototypes[layerIndex];
	
	auto result = createAssetObject<ULandscapeGrassType>(
		grassTypeName, &terrainDataPath, importer, 
		[&](ULandscapeGrassType* obj){
			auto& dstType = obj->GrassVarieties.AddDefaulted_GetRef();
			float scaleFactor = 1.0f;
			if (srcType.usePrototypeMesh){
				//auto mesh = createGrassMesh
				auto meshName = grassTypeName + TEXT("mesh");
				auto mesh = createGrassMesh(meshName, srcType, layerIndex, terrainDataPath);
				//auto mesh = importer->loadStaticMeshById(srcType.detailMeshId);
				if (!mesh){
					UE_LOG(JsonLogTerrain, Warning, TEXT("Could not load mesh %d used by detail layer %d"), srcType.detailMeshId, layerIndex);
					return;
				}
				dstType.GrassMesh = mesh;

				dstType.Scaling = EGrassScaling::Uniform;
				auto scaleInterval = FFloatInterval(srcType.minHeight * scaleFactor, srcType.maxHeight * scaleFactor);
				dstType.ScaleX = scaleInterval;
				dstType.ScaleY = scaleInterval;
				dstType.ScaleZ = scaleInterval;
			}
			else{
				dstType.Scaling = EGrassScaling::Uniform;
				auto scaleInterval = FFloatInterval(srcType.minHeight * scaleFactor, srcType.maxHeight * scaleFactor);
				dstType.ScaleX = scaleInterval;
				dstType.ScaleY = scaleInterval;
				dstType.ScaleZ = scaleInterval;

				//billboard?
				auto mesh = createBillboardMesh(grassTypeName + TEXT("_Mesh"), srcType, layerIndex, terrainDataPath);
				if (!mesh){
					UE_LOG(JsonLogTerrain, Warning, TEXT("Could not create billboard mesh for layer %d"), srcType.detailMeshId, layerIndex);
					return;
				} 
				dstType.GrassMesh = mesh;
			}

			auto totalCells = (float)terrainData.detailHeight * (float)terrainData.detailWidth;
			auto totalGrass = totalCells * 16.0f;//On unity side, maximum number of grass spots per cell is 16. I don't know why.
			auto landArea = terrainData.worldSize.X * terrainData.worldSize.Z;

			auto density = totalGrass / landArea; //that's per square meter
			/*
			Unreal documentation(comments) claims that density is specified per 10 (TEN) square meters.
			The documentation appears to be incorrect, and code suggest that it is actually per 100 (Hundred) square meters.
			*/
			density *= 100.0f;
			//density /= 10.0f;
			/*
			if (	srcType.billboardFlag)
				dstType.bCastDynamicShadow 
				*/
			if (srcType.billboardFlag)
				dstType.bCastDynamicShadow = false;

			dstType.GrassDensity = density;
		}, RF_Standalone|RF_Public
	);

	return result;
}

ULandscapeLayerInfoObject* TerrainBuilder::createTerrainLayerInfo(int layerIndex, bool grassLayer, 
		const FString &terrainDataPath){

	auto layerName = grassLayer ? terrainData.getGrassLayerName(layerIndex): terrainData.getLayerName(layerIndex);
	
	auto result = createAssetObject<ULandscapeLayerInfoObject>(
		layerName, &terrainDataPath, importer, 
		[&](ULandscapeLayerInfoObject* layerObj){
			layerObj->LayerName = *layerName;

			int32 colIndex = 1 + layerIndex % 7;
			layerObj->LayerUsageDebugColor = FLinearColor(
				(colIndex & 0x1) ? 1.0f: 0.0f,
				(colIndex & 0x2) ? 1.0f: 0.0f,
				(colIndex & 0x4) ? 1.0f: 0.0f
			);
			
			layerObj->SetFlags(RF_Standalone|RF_Public);
		}
	);

	return result;
}


void TerrainBuilder::processFoliageTreeActors(ALandscape *landscape){
	UE_LOG(JsonLogTerrain, Log, TEXT("Processing terrain mesh actors. %d types, %d instances"), terrainData.treePrototypes.Num(), terrainData.treeInstances.Num());

	auto* ifa = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(workData.world.Get(), true);

	FString terrPath, terrFileName, terrExt;
	FPaths::Split(terrainData.exportPath, terrPath, terrFileName, terrExt);

	TMap<int32, UFoliageType*> foliageTypes;
	TMap<int32, FFoliageMeshInfo*> foliageMeshInfos;

	int prototypeIndex = 0;
	for(const auto& curProtoTree: terrainData.treePrototypes){
		auto curProtoIdx = prototypeIndex++;
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing tree prototype %d. meshId %d; prefabId %d; prefabObj %d"),
			curProtoIdx, (int)curProtoTree.meshId, curProtoTree.prefabId, curProtoTree.prefabObjectId);

		if (!isValidId(curProtoTree.meshId)){
			UE_LOG(JsonLogTerrain, Log, TEXT("Prototype %d has no mesh, nothing to do"), curProtoIdx);
			continue;
		}
		auto meshId = curProtoTree.meshId;

		auto treeMesh = importer->loadStaticMeshById(meshId);
		if (!treeMesh){
			UE_LOG(JsonLogTerrain, Warning, TEXT("Mesh %d could not be loaded for prototype %d"), (int)meshId, curProtoIdx);
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

	//normal layers
	TArray<FLandscapeImportLayerInfo> importLayers;
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

//#define TERRAIN_SAVE_DEBUG_IMAGES

	//grass
	grassTypes.Empty();
	if (convertedTerrain.detailMaps.Num() > 0){
		for(int i = 0; i < convertedTerrain.detailMaps.Num(); i++){
			auto layerName = terrainData.getGrassLayerName(i);
			auto layerInfoObj = createTerrainLayerInfo(i, true, terrainDataPath);

			auto &newLayer = importLayers.AddDefaulted_GetRef();
			newLayer.LayerName = *layerName;
			auto& detail = convertedTerrain.detailMaps[i];
			newLayer.LayerData = detail.getArrayCopy();
			newLayer.LayerInfo = layerInfoObj;
			newLayer.SourceFilePath = TEXT("");
		#ifdef TERRAIN_SAVE_DEBUG_IMAGES
			auto fullDstPath = FPaths::Combine(TEXT("D:\\work\\EpicGames\\debug"), layerName + FString::Printf(TEXT("_%dx%d"), xSize, ySize) + TEXT(".raw"));
			detail.saveToRaw(fullDstPath);
		#endif

			auto newGrassType = createGrassType(i, terrainDataPath);
			grassTypes.Add(newGrassType);
		}
	}

	auto terrainVertSize = FIntPoint(xSize, ySize);
	MaterialBuilder materialBuilder;
	UMaterial *terrainMaterial = materialBuilder.createTerrainMaterial(this, terrainVertSize, terrainDataPath);

	FTransform terrainTransform;
	FMatrix terrainMatrix = jsonGameObj.ueWorldMatrix;

	auto ueWorldSize = unitySizeToUe(terrainData.worldSize);
	UE_LOG(JsonLogTerrain, Log, TEXT("Terrain size: %f %f %f"), ueWorldSize.X, ueWorldSize.Y, ueWorldSize.Z);
	auto halfWorldSize = ueWorldSize * 0.5f;

	FVector vTerX, vTerY, vTerZ, vTerPos;
	/* 
		Oh, this is interesting. 
		Terrain in unity is permanently axis aligned and can be neither rotated nor scaled. 
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
	terrainScale.Z *= 2.0f;
	//terrainScale.X = 100.0f * (ueWorldSize.X / 100.0f);
	FVector terrainOffset = FVector::ZeroVector;

	logValue(TEXT("Terrain offset: "), terrainOffset);
	//terrainOffset += vTerZ * ueWorldSize.Z * 0.5f;// / 100.0f;
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
	landProxy->LandscapeHoleMaterial = terrainMaterial;

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

	if (terrainData.detailPrototypes.Num() > 0){
		//landProxy->FlushGrassComponents();
		/*
		auto comps = landProxy->LandscapeComponents;
		TSet<ULandscapeComponent*> components;
		components.Append(comps);
		ALandscapeProxy::InvalidateGeneratedComponentData(components);
		*/
	}

	return result;
}
