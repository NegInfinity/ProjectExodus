#include "JsonImportPrivatePCH.h"
#include "LandscapeTest.h"
#include "JsonLog.h"
/*
This demonstrates landscape crash I've experienced. The problematic part is this:

--------
	landProxy->Import(FGuid::NewGuid(),
		0, 0, xSize - 1, ySize - 1, sectionsPerComp, quadsPerSection, hMap.GetData(), 
		TEXT(""), importLayers, ELandscapeImportAlphamapType::Additive);
--------

And is located at the line 130 at the time of writing.The rest is simply configuration of the test landscape.

Symptoms: After calling "Import()" the engine triggers shader recompilation, which then proceeds seemingly normally.
In the middle of compilation landscape appears black, and once that happen, the editor crashes with a failed assert once shader recompilation is done.

The cause of the issue is unknown, but multithreaded nature is suspected.

The only way to avoid the problem is not to render the landscape. That means either turning the camera away from it, or 
spawning landscape in an external UWorld, that is not currently being displayed.
*/

#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"

#include "JsonObjects/DataPlane2D.h"
#include "JsonObjects/DataPlane3D.h"
#include "UnrealUtilities.h"

#include "Materials/Material.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerSwitch.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"

#include "MaterialBuilder/MaterialTools.h"

using namespace MaterialTools;

static FString getTestRootPath(){
	return FString("/Game/Import/LandscapeTest");
}

static FLinearColor indexToColor(int index){
	int32 colIndex = 1 + index % 7;
	return FLinearColor(
		(colIndex & 0x1) ? 1.0f: 0.0f,
		(colIndex & 0x2) ? 1.0f: 0.0f,
		(colIndex & 0x4) ? 1.0f: 0.0f
	);
}

static ULandscapeLayerInfoObject* createTerrainLayerInfo(const FString &layerName, int layerIndex){
	auto fullPath = FString::Printf(TEXT("%s/%s"), *getTestRootPath(), *layerName);
	auto pkg = CreatePackage(0, *fullPath);

	auto result = NewObject<ULandscapeLayerInfoObject>(pkg);
	result->LayerName = *layerName;
	int32 colIndex = 1 + layerIndex % 7;
	result->LayerUsageDebugColor = indexToColor(layerIndex);
			
	result->SetFlags(RF_Standalone|RF_Public);

	if (result){
		result->PreEditChange(0);
		result->PostEditChange();
		FAssetRegistryModule::AssetCreated(result);
		pkg->SetDirtyFlag(true);
	}

	return result;
}


ALandscape* createTestLandscape(UWorld *world, const TArray<FString> &layerNames, UMaterial *landMaterial){
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

	for(int layerIndex = 0; layerIndex < layerNames.Num(); layerIndex++){
		auto layerName = layerNames[layerIndex];
		auto &curLayer = importLayers.AddDefaulted_GetRef();
		DataPlane2D<uint8> alphaMap;
		alphaMap.resize(xSize, ySize);
		for (int y = 0; y < ySize; y++){
			auto scanline = alphaMap.getRow(y);
			auto val = (((y / 16) % layerNames.Num()) == layerIndex) ? 0xFF: 0x00;
			for(int x = 0; x < xSize; x++){
				scanline[x] = val;
			}
		}
		auto infoObj = createTerrainLayerInfo(layerName, layerIndex);

		curLayer.LayerData = alphaMap.getArrayCopy();
		curLayer.LayerName = *layerName;
		curLayer.SourceFilePath = TEXT("");
		curLayer.LayerInfo = infoObj;
	}

	result = world->SpawnActor<ALandscape>(ALandscape::StaticClass());
	result->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((xSize * ySize) / (2048 * 2048) + 1), (uint32)2);//?

	auto guid = FGuid::NewGuid();
	auto *landProxy = Cast<ALandscapeProxy>(result);
	landProxy->SetLandscapeGuid(guid);

	landProxy->LandscapeMaterial = landMaterial;

	landProxy->Import(FGuid::NewGuid(),
		0, 0, xSize - 1, ySize - 1, sectionsPerComp, quadsPerSection, hMap.GetData(), 
		TEXT(""), importLayers, ELandscapeImportAlphamapType::Additive);

	/*
	Following lines are meant to trigger shader recompilation (when needed), but upon completion the editor crashes with a failed assert.
	PreEditChange/PostEditChange produce similar effect.
	*/

	ULandscapeInfo *landscapeInfo  = result->CreateLandscapeInfo();
	landscapeInfo->UpdateLayerInfoMap(result);

	result->MarkComponentsRenderStateDirty();
	result->PostLoad();

	/*
	result->PreEditChange(0);
	result->PostEditChange();
	*/

	return result;
}

UMaterial *createLandscapeMaterial(const TArray<FString> &names){
	auto fullPath = FString::Printf(TEXT("%s/%s"), *getTestRootPath(), TEXT("material"));
	auto matPkg = CreatePackage(0, *fullPath);

	auto matFactory = NewObject<UMaterialFactoryNew>();
	auto material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPkg, FName(TEXT("testLandscapeMaterial")), RF_Standalone|RF_Public, 0, GWarn);

	//auto layerBlend = createExpression<MaterialExpressionLandscapeLayerSwitch>(material);
	auto blendExpr = createExpression<UMaterialExpressionLandscapeLayerBlend>(material);
	blendExpr->Layers.Empty();
	for(int layerIndex = 0; layerIndex < names.Num(); layerIndex++){
		auto& curTarget = blendExpr->Layers.AddDefaulted_GetRef();
		const auto& layerName = names[layerIndex];
		auto color = indexToColor(layerIndex);
		auto colorExpr = createExpression<UMaterialExpressionConstant4Vector>(material);
		colorExpr->Constant = color;

		curTarget.LayerName = *layerName;
		curTarget.BlendType = LB_WeightBlend;
		curTarget.LayerInput.Expression = colorExpr;
	}
	material->BaseColor.Expression = blendExpr;

	arrangeMaterialNodesAsTree(material);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();
		FAssetRegistryModule::AssetCreated(material);
		matPkg->SetDirtyFlag(true);
	}
	return material;
}

void LandscapeTest::run(){
	auto world = GEditor->GetEditorWorldContext().World();
	TArray<FString> layerNames;
	layerNames.Add(TEXT("layer0"));
	layerNames.Add(TEXT("layer1"));

	auto landMaterial = createLandscapeMaterial(layerNames);
	auto landscape = createTestLandscape(world, layerNames, landMaterial);
}
