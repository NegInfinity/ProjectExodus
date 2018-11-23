#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"
#include "MaterialTools.h"

UMaterialExpression* createTerrainLayerCoords(UMaterial *material, const JsonTerrain &terr, const JsonTerrainData &terrData
	, const FVector2D& splatSize, const FVector2D& splatOffset, const FIntPoint &terrainVertSize,  const TCHAR* text = 0){
	auto landCoord = createExpression<UMaterialExpressionLandscapeLayerCoords>(material, text);

	landCoord->MappingScale = 1.0f;//1 per one vert unit
	landCoord->MappingPanU = 0.0f;
	landCoord->MappingPanV = 0.0f;

	auto swapMaskX = createComponentMask(material, true, false, false, false);
	swapMaskX->Input.Expression = landCoord;

	auto swapMaskY = createComponentMask(material, false, true, false, false);
	swapMaskY->Input.Expression = landCoord;

	auto landCoordSwapped = createAppendVectorExpression(material, swapMaskY, swapMaskX);

	auto ueToQuadUv = FVector2D(1.0f, 1.0f);
	if (terrainVertSize.X != 1)
		ueToQuadUv.X = 1.0f/(float)(terrainVertSize.X - 1);
	if (terrainVertSize.Y != 1)
		ueToQuadUv.Y = 1.0f/(float)(terrainVertSize.Y - 1);

	auto splatUvOffset = splatOffset;
	if (splatSize.X)
		splatUvOffset.X /= splatSize.X;
	if (splatSize.Y)
		splatUvOffset.Y /= splatSize.Y;

	auto quadToFinalScale = FVector2D(terrData.worldSize.X, terrData.worldSize.Z);
	quadToFinalScale.X = splatSize.X ? terrData.worldSize.X / splatSize.X : 1.0f;
	quadToFinalScale.Y = splatSize.Y ? terrData.worldSize.Z / splatSize.Y : 1.0f;	

	auto scaleUvConst = createExpression<UMaterialExpressionConstant4Vector>(material, TEXT("Terrain channel downscale"));
	scaleUvConst->Constant.R = ueToQuadUv.X * quadToFinalScale.X;
	scaleUvConst->Constant.G = -ueToQuadUv.Y * quadToFinalScale.Y;
	auto scaleUvVal = createComponentMask(material, true, true, false, false);
	scaleUvVal->Input.Expression = scaleUvConst;

	auto offsetUvConst = createExpression<UMaterialExpressionConstant4Vector>(material, TEXT("Terrain channel offset"));
	offsetUvConst->Constant.R = splatUvOffset.X;
	offsetUvConst->Constant.G = -splatUvOffset.Y;
	auto offsetUvVal = createComponentMask(material, true, true, false, false);
	offsetUvVal ->Input.Expression = offsetUvConst;

	auto mul = createMulExpression(material, landCoordSwapped, scaleUvVal);
	auto add = createAddExpression(material, mul, offsetUvVal);

	return add;
}

UMaterialExpression* createLayerBlending(UMaterial* material, bool needSeparateBlend, const JsonTerrainData &terrData
	, std::function<UMaterialExpression*(UMaterial* material, int index, const JsonSplatPrototype &splat)> channelFunc
	, const TCHAR* blendName = 0){
	if (!channelFunc){
		UE_LOG(JsonLogTerrain, Error, TEXT("Null channel func while generating material for terrain \"%s\", cannot proceed"), *terrData.name);
		return nullptr;
	}

	if (terrData.splatPrototypes.Num() <= 0){
		UE_LOG(JsonLogTerrain, Error, TEXT("No splats on terrain \"%s\", material generation won't proceed far"), *terrData.name);
		return nullptr;
	}

	if (!needSeparateBlend){
		return channelFunc(material, 0, terrData.splatPrototypes[0]);
	}

	auto blendExpr = createExpression<UMaterialExpressionLandscapeLayerBlend>(material, blendName);
	for(int layerIndex = 0; layerIndex < terrData.splatPrototypes.Num(); layerIndex++){
		const auto &srcSplat = terrData.splatPrototypes[layerIndex];
		auto layerName = terrData.getLayerName(layerIndex);

		auto &dst = blendExpr->Layers.AddDefaulted_GetRef();
		dst.PreviewWeight = 1.0f;
		dst.LayerName = *layerName;
		dst.BlendType = LB_WeightBlend;

		auto* expr =channelFunc(material, layerIndex, srcSplat);

		dst.LayerInput.Expression = expr;
	}

	return blendExpr;
}

UMaterialExpression* createDetailControl(UMaterial* material, const TerrainBuilder *terrainBuilder){
	//if (terrData.
	check(terrainBuilder);
	const auto& terrData = terrainBuilder->terrainData;
	if (terrData.detailPrototypes.Num() <= 0)
		return nullptr;

	check(terrainBuilder->grassTypes.Num() == terrainBuilder->terrainData.detailPrototypes.Num());

	auto numDetailLayers = terrData.detailPrototypes.Num();
	auto grassControl = createExpression<UMaterialExpressionLandscapeGrassOutput>(material, TEXT("Grass control"));

	auto grassEnabled = createExpression<UMaterialExpressionStaticBoolParameter>(material, TEXT("enableGrass"));
	grassEnabled->DefaultValue = terrainBuilder->jsonTerrain.drawTreesAndFoliage;

	auto zeroConst = createExpression<UMaterialExpressionConstant>(material, TEXT("Zero"));
	zeroConst->R = 0.0f;

	for(int i = 0; i < numDetailLayers; i++){
		auto layerName = terrData.getGrassLayerName(i);

		auto &grassTypes = grassControl->GrassTypes;
		//what the hell.
		//auto& dstGrass = grassTypes.Num() > 0 ? grassTypes[0]: grassTypes.AddDefaulted_GetRef();
		auto& dstGrass = (grassTypes.Num() >= (i + 1)) ? grassTypes[i]: grassTypes.AddDefaulted_GetRef();
		dstGrass.Name = *layerName;
		dstGrass.GrassType = terrainBuilder->grassTypes[i];

		auto layerSample = createExpression<UMaterialExpressionLandscapeLayerSample>(material, *layerName);
		layerSample->ParameterName = *layerName;
		layerSample->PreviewWeight = 1.0f;

		auto grassSwitch = createExpression<UMaterialExpressionStaticSwitch>(material);
		grassSwitch->Value.Expression = grassEnabled;
		grassSwitch->B.Expression = zeroConst;
		grassSwitch->A.Expression = layerSample;

		dstGrass.Input.Expression = grassSwitch;//layerSample;
	}

	return grassControl;
}

void MaterialBuilder::buildTerrainMaterial(UMaterial* material, 
		const TerrainBuilder *terrainBuilder,
		const FIntPoint &terrainVertSize, const FString &terrainDataPath){

	checkf(terrainBuilder, TEXT("Terrain builder cannot be null"));

	material->BlendMode = BLEND_Masked;

	const auto &terrData = terrainBuilder->terrainData;

	bool needColorBlend = false;
	bool needNormalBlend = false;
	bool needMetallicBlend = false;
	bool needSpecularBlend = false;
	bool needSmoothnessBlend = false;
	bool needUvScales = false;

	FVector2D defaultTileSize(1.0f, 1.0f), defaultTileOffset(0.0f, 0.0f);

	float defaultMetallic = 0.0f, defaultSmoothness = 0.0f;
	FLinearColor defaultSpecular = FLinearColor::White;

	for(int i = 0; i < terrData.splatPrototypes.Num(); i++){
		auto& cur = terrData.splatPrototypes[i];
		if (i == 0){
			defaultMetallic = cur.metallic;
			defaultSpecular = cur.specular;
			defaultSmoothness = cur.smoothness;
			defaultTileSize = cur.tileSize;
			defaultTileOffset = cur.tileOffset;
		}
		else{
			needMetallicBlend = needMetallicBlend || (defaultMetallic != cur.metallic);
			needSpecularBlend = needSpecularBlend || (defaultSpecular != cur.specular);
			needSmoothnessBlend = needSmoothnessBlend || (defaultSmoothness != cur.smoothness);
			needUvScales = needUvScales || (defaultTileSize != cur.tileSize) || (defaultTileOffset != cur.tileSize);
		}
		needColorBlend = needColorBlend || (cur.textureId >= 0);
		needNormalBlend = needNormalBlend || (cur.normalMapId >= 0);
	}

	TArray<UMaterialExpression*> layerUvCoords;
	const auto &terr = terrainBuilder->jsonTerrain;
	if (needUvScales){
		for(int i = 0; i < terrData.splatPrototypes.Num(); i++){
			const auto& src = terrData.splatPrototypes[i];
			auto coordName = FString::Printf(TEXT("uv coords #%d: %s"), i, *terrData.getLayerName(i));
			auto curExpr = createTerrainLayerCoords(material, terr, terrData, src.tileSize, src.tileOffset, terrainVertSize, TEXT("uv coords"));
			layerUvCoords.Add(curExpr);
		}
	}
	else{
		auto defaultExpr = createTerrainLayerCoords(material, terr, terrData, defaultTileSize, defaultTileOffset, terrainVertSize, TEXT("default uv coords"));
		for(int i = 0; i < terrData.splatPrototypes.Num(); i++){
			layerUvCoords.Add(defaultExpr);
		}
	}

	auto* importer = terrainBuilder->getImporter();
	auto* colorBlendExpr = createLayerBlending(material, needColorBlend, terrData, 
		[&](UMaterial* mat, int layerIndex, const JsonSplatPrototype& srcSplat) -> UMaterialExpression*{
			auto* colorTex = importer->getTexture(srcSplat.textureId);
			if (colorTex){
				auto texExpr = createTextureExpression(material, colorTex, 0);
				texExpr->Coordinates.Expression = layerUvCoords[layerIndex];
				return texExpr;
			}
			else{
				auto expr = createExpression<UMaterialExpressionConstant4Vector>(material);
				expr->Constant = FLinearColor::White;
				return expr;
			}
		}, TEXT("Color")
	);
	material->BaseColor.Expression = colorBlendExpr;

	if (needNormalBlend){
		auto* normBlendExpr = createLayerBlending(material, needNormalBlend, terrData,
			[&](UMaterial* mat, int layerIndex, const JsonSplatPrototype& srcSplat) -> UMaterialExpression*{
				auto* normTex = importer->getTexture(srcSplat.normalMapId);

				if (normTex){
					auto texExpr = createTextureExpression(material, normTex, 0);
					texExpr->Coordinates.Expression = layerUvCoords[layerIndex];
					return texExpr;
				}
				else{
					auto expr = createExpression<UMaterialExpressionConstant4Vector>(material);
					expr->Constant = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f);
					return expr;
				}
			}, TEXT("Normals")
		);
		material->Normal.Expression = normBlendExpr;
	}

	auto metallicBlendExpr = createLayerBlending(material, needMetallicBlend, terrData,
		[&](UMaterial* mat, int layerIndex, const JsonSplatPrototype& srcSplat) -> UMaterialExpression*{
			auto name = FString::Printf(TEXT("Metallic for layer %d(%s)"), layerIndex, *terrData.getLayerName(layerIndex));
			auto result = createConstantExpression(material, srcSplat.metallic, *name);
			return result;
		}, TEXT("Metallic")
	);
	material->Metallic.Expression = metallicBlendExpr;

	auto roughnessExpr = createLayerBlending(material, needSmoothnessBlend, terrData,
		[&](UMaterial* mat, int layerIndex, const JsonSplatPrototype& srcSplat) -> UMaterialExpression*{
			auto name = FString::Printf(TEXT("Roughness for layer %d(%s)"), layerIndex, *terrData.getLayerName(layerIndex));
			auto result = createConstantExpression(material, 1.0f - srcSplat.smoothness, 0);
			return result;
		}, TEXT("Smoothness/Roughness")
	);
	//auto smoothnessBlend
	material->Roughness.Expression = roughnessExpr;

	createDetailControl(material, terrainBuilder);

	//if (!terrainBuilder->jsonTerrain.drawHeightmap){
		//auto statBool = createExpression<UMaterialExpressionStaticBool>(material, TEXT("terrainDrawEnabled"));
		auto statBool = createExpression<UMaterialExpressionStaticBoolParameter>(material, TEXT("terrainDrawEnabled"));
		//statBool->Value = terrainBuilder->jsonTerrain.drawHeightmap;
		statBool->DefaultValue = terrainBuilder->jsonTerrain.drawHeightmap;
		auto statSwitch = createExpression<UMaterialExpressionStaticSwitch>(material);

		auto constExpr = createExpression<UMaterialExpressionConstant>(material, TEXT("Zero alpha"));
		constExpr->R = 0.0f;

		statSwitch->DefaultValue = true;
		statSwitch->Value.Expression = statBool;
		//statSwitch->A.Expression = 
		statSwitch->B.Expression = constExpr;

		material->OpacityMask.Expression = statSwitch;
	//}

	//no specular
	MaterialTools::arrangeMaterialNodesAsTree(material);
}

UMaterial* MaterialBuilder::createTerrainMaterial(const TerrainBuilder *terrainBuilder, 
		const FIntPoint &terrainVertSize, const FString &terrainDataPath){

	checkf(terrainBuilder, TEXT("Terrain builder cannot be null"));

	const auto& terrainData = terrainBuilder->terrainData;
	const auto* importer = terrainBuilder->getImporter();
	
	auto materialName = terrainData.name + TEXT("_Material");
	auto basePackageName = sanitizePackageName(materialName);
	auto importPath = importer->getProjectImportPath();
	auto materialPackagePath = buildPackagePath(basePackageName, 
		&terrainDataPath, &importPath, &importer->getAssetCommonPath());

	UE_LOG(JsonLogTerrain, Log, TEXT("Creating material package for terrain \"%s\"(\"%s\") at \"%s\""), 
		*terrainData.name, *terrainData.exportPath, *materialPackagePath);

	auto materialPackage = CreatePackage(0, *materialPackagePath);

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* materialObj = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), 
		materialPackage, 
		FName(*basePackageName), 
		RF_Standalone|RF_Public,
		0, GWarn
	);

	buildTerrainMaterial(materialObj, terrainBuilder, terrainVertSize, terrainDataPath);

	if (materialObj){
		materialObj->PreEditChange(0);
		materialObj->PostEditChange();

		FAssetRegistryModule::AssetCreated(materialObj);
		materialPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return materialObj;
}

