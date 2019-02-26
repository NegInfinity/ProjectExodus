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

And is located at the line 157 at the time of writing.The rest is simply configuration of the test landscape.

Symptoms: After calling "Import()" the engine triggers shader recompilation, which then proceeds seemingly normally.
In the middle of compilation landscape appears black, and once that happen, the editor crashes with a failed assert once shader recompilation is done.

The cause of the issue is unknown, but multithreaded nature is suspected, as the bug does not ALWAYS happen.

The only way to avoid the problem is not to render the landscape. That means either turning the camera away from it, or 
spawning landscape in an external UWorld, that is not currently being displayed. This is the typical crash:
--------

Assertion failed: bValid || !bFailOnInvalid [File:d:\build\++ue4\sync\engine\source\runtime\engine\public\MaterialShared.h] [Line: 881] 
FMaterialShaderMap testLandscapeMaterial invalid for rendering: bCompilationFinalized: 1, bCompiledSuccessfully: 1, bDeletedThroughDeferredCleanup: 1


0x000007fefd5fa06d KERNELBASE.dll!UnknownFunction []
0x000007fede9bcd1a UE4Editor-Core.dll!FWindowsErrorOutputDevice::Serialize() [d:\build\++ue4\sync\engine\source\runtime\core\private\windows\windowserroroutputdevice.cpp:63]
0x000007fede79a967 UE4Editor-Core.dll!FOutputDevice::LogfImpl() [d:\build\++ue4\sync\engine\source\runtime\core\private\misc\outputdevice.cpp:71]
0x000007fede6d0df7 UE4Editor-Core.dll!FDebug::AssertFailed() [d:\build\++ue4\sync\engine\source\runtime\core\private\misc\assertionmacros.cpp:417]
0x000007fedb0420ad UE4Editor-Renderer.dll!FMaterialShaderMap::IsValidForRendering() [d:\build\++ue4\sync\engine\source\runtime\engine\public\materialshared.h:882]
0x000007fedafec9e8 UE4Editor-Renderer.dll!FMaterialShader::SetParametersInner<FRHIVertexShader * __ptr64>() [d:\build\++ue4\sync\engine\source\runtime\renderer\private\shaderbaseclasses.cpp:183]
0x000007fedaa17ca5 UE4Editor-Renderer.dll!TBasePassDrawingPolicy<FUniformLightMapPolicy>::SetSharedState() [d:\build\++ue4\sync\engine\source\runtime\renderer\private\basepassrendering.h:834]
0x000007feda9bfe39 UE4Editor-Renderer.dll!TStaticMeshDrawList<TBasePassDrawingPolicy<FUniformLightMapPolicy> >::DrawElement() [d:\build\++ue4\sync\engine\source\runtime\renderer\private\staticmeshdrawlist.inl:186]
0x000007feda9c178c UE4Editor-Renderer.dll!TStaticMeshDrawList<TBasePassDrawingPolicy<FUniformLightMapPolicy> >::DrawVisibleInner() [d:\build\++ue4\sync\engine\source\runtime\renderer\private\staticmeshdrawlist.inl:391]
0x000007feda9bfa04 UE4Editor-Renderer.dll!FDrawVisibleAnyThreadTask<TBasePassDrawingPolicy<FUniformLightMapPolicy> >::DoTask() [d:\build\++ue4\sync\engine\source\runtime\renderer\private\staticmeshdrawlist.inl:505]
0x000007feda9c3553 UE4Editor-Renderer.dll!TGraphTask<FDrawVisibleAnyThreadTask<TBasePassDrawingPolicy<FUniformLightMapPolicy> > >::ExecuteTask() [d:\build\++ue4\sync\engine\source\runtime\core\public\async\taskgraphinterfaces.h:830]
0x000007fede560aec UE4Editor-Core.dll!FTaskThreadAnyThread::ProcessTasks() [d:\build\++ue4\sync\engine\source\runtime\core\private\async\taskgraph.cpp:1022]
0x000007fede561d70 UE4Editor-Core.dll!FTaskThreadAnyThread::ProcessTasksUntilQuit() [d:\build\++ue4\sync\engine\source\runtime\core\private\async\taskgraph.cpp:847]
0x000007fede56baae UE4Editor-Core.dll!FTaskThreadAnyThread::Run() [d:\build\++ue4\sync\engine\source\runtime\core\private\async\taskgraph.cpp:923]
0x000007fede9bc500 UE4Editor-Core.dll!FRunnableThreadWin::Run() [d:\build\++ue4\sync\engine\source\runtime\core\private\windows\windowsrunnablethread.cpp:76]
0x000007fede9acac1 UE4Editor-Core.dll!FRunnableThreadWin::GuardedRun() [d:\build\++ue4\sync\engine\source\runtime\core\private\windows\windowsrunnablethread.cpp:33]
0x00000000775959bd kernel32.dll!UnknownFunction []
0x00000000777ca2e1 ntdll.dll!UnknownFunction []

Crash in runnable thread TaskGraphThreadNP 1
---------
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

FString LandscapeTest::getTestRootPath(){
	return FString("/Game/Import/LandscapeTest");
}

FLinearColor LandscapeTest::indexToColor(int index){
	int32 colIndex = 1 + index % 7;
	return FLinearColor(
		(colIndex & 0x1) ? 1.0f: 0.0f,
		(colIndex & 0x2) ? 1.0f: 0.0f,
		(colIndex & 0x4) ? 1.0f: 0.0f
	);
}

ULandscapeLayerInfoObject* LandscapeTest::createTerrainLayerInfo(const FString &layerName, int layerIndex){
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


ALandscape* LandscapeTest::createTestLandscape(UWorld *world, const TArray<FString> &layerNames, UMaterial *landMaterial){
	//FlushRenderingCommands(true);
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

UMaterial* LandscapeTest::createLandscapeMaterial(const TArray<FString> &names){
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
