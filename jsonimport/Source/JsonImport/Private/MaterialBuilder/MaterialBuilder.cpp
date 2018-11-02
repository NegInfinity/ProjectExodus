#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"

#include "JsonImporter.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionSquareRoot.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionMax.h"

#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerSwitch.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"

#include "MaterialTools.h"
#include "JsonObjects/utilities.h"

DEFINE_LOG_CATEGORY(JsonLogMatNodeSort);

using namespace MaterialTools;

//#define MATBUILDER_OLDGEN

void MaterialBuilder::arrangeNodesGrid(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	auto numExpressions = material->Expressions.Num();
	int expressionRows = (int)(sqrtf((float)numExpressions))+1;
	if (expressionRows == 0)
		return;

	for (int i = 0; i < numExpressions; i++){
		auto cur = material->Expressions[i];
		auto row = i / expressionRows;
		auto col = i % expressionRows;

		int32 size = 256;

		int32 x = (col - expressionRows) * size;
		int32 y = row * size;

		cur->MaterialExpressionEditorX = x;
		cur->MaterialExpressionEditorY = y;
	}
}

void MaterialBuilder::arrangeNodesTree(UMaterial* material /*, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData*/){
	TMap<UMaterialExpression*, TSet<UMaterialExpression*>> srcToDst, dstToSrc;
	auto registerConnection = [&](UMaterialExpression* src, UMaterialExpression* dst){
		if (!src || !dst)
			return;
		if (src == dst)
			return;
		srcToDst.FindOrAdd(src).Add(dst);
		dstToSrc.FindOrAdd(dst).Add(src);
	};

	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Sorting expressions"));
	//calculate connection chain
	for(auto curExpr: material->Expressions){
		auto inputs = curExpr->GetInputs();
		for(auto curInput: inputs){
			if (!curInput || !curInput->Expression)
				continue;
			registerConnection(curInput->Expression, curExpr);
		}
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("srcToDst connections: %d, dstToSrc connections: %d"), srcToDst.Num(), dstToSrc.Num());

	TSet<UMaterialExpression*> topLevel;
	//find top level ones
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Gathering top level nodes"));
	for(auto curExpr: material->Expressions){
		if (!srcToDst.Contains(curExpr))
			topLevel.Add(curExpr);
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d nodes found"), topLevel.Num());

	TMap<UMaterialExpression*, int> exprLevels;
	TQueue<UMaterialExpression*> unprocessed;
	TMap<int, TArray<UMaterialExpression*>> levelOrder;

	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Enqueueing top-level nodes"));
	for(auto curExpr: topLevel){
		unprocessed.Enqueue(curExpr);
	}

	int defaultParamLevel = 1024;
	int maxLevel = defaultParamLevel;//0;
	while(!unprocessed.IsEmpty()){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing items still in queue"));
		UMaterialExpression* curExpr = 0;
		if (!unprocessed.Dequeue(curExpr))
			break;
		if (!curExpr)
			continue;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current node: %x, %s"), curExpr, *curExpr->GetName());
		int curLevel = 0;
		if (!dstToSrc.Contains(curExpr))
			curLevel = defaultParamLevel;
		const auto foundLevel= exprLevels.Find(curExpr);
		if (foundLevel){
			curLevel = *foundLevel;
		}
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current level: %d"), curLevel);

		auto& curOrder = levelOrder.FindOrAdd(curLevel);
		curOrder.Add(curExpr);
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d items at current level"), curOrder.Num());

		const auto children = dstToSrc.Find(curExpr);
		if (!children)
			continue;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d children found"), children->Num());
		for(auto child: *children){
			if (!child)
				continue;

			auto childLevel = curLevel + 1;
			if (!dstToSrc.Contains(child))
				childLevel = defaultParamLevel;
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing child %s (%x)"), *child->GetName(), child);
			auto prevLevel = exprLevels.Find(child);
			if (prevLevel){
				if (*prevLevel >= childLevel){
					//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Child %s (%x) already registered on level %d"), *child->GetName(), child, *prevLevel);
					continue;
				}
				levelOrder.FindOrAdd(*prevLevel).Remove(child);
			}
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Child %s (%x) enqueued for processing"), *child->GetName(), child);
			unprocessed.Enqueue(child);
			exprLevels.Add(child, childLevel);
			if (childLevel > maxLevel)
				maxLevel = childLevel;
		}
	}

	TArray<int> usedLevels;
	auto maxNumItems = 0;
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Gathering used levels"));
	for(int i = 0; i <= maxLevel; i++){
		auto numItems = levelOrder.FindOrAdd(i).Num();
		if (numItems > 0)
			usedLevels.Add(i);
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Registered level %d"), i);
		if (numItems > maxNumItems)
			maxNumItems = numItems;
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d levels total"), usedLevels.Num());

	//Now, actual grid.

	auto numRows = maxNumItems;
	auto numColumns = usedLevels.Num();
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("numRows: %d; numColumns: %d"), numRows, numColumns);
	if ((numRows == 0) || (numColumns == 0)){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Nothing to do, returning"));
		return;
	}


#if 0
	int32 xSize = 128;
	int32 ySize = 256;
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Sorting material node tree"));
	for(int col = 0; col < usedLevels.Num(); col++){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing column %d out of %d"), col, usedLevels.Num());
		int levelIndex = usedLevels[col];
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current level %d"), levelIndex);
		int x = xSize * (-1 -col);//(col - numColumns);
		const auto &curLevel = levelOrder.FindOrAdd(levelIndex);
		int curNumRows = curLevel.Num();
		int yOffset = (maxNumItems - curNumRows) * ySize / 2;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Num rows: %d, yOffset: %d"), curNumRows, yOffset);
		for (int row = 0; row < curNumRows; row++){
			int y = yOffset + row * ySize;
			auto item = curLevel[row];
			if (!item)
				continue;
			item->MaterialExpressionEditorX = x;
			item->MaterialExpressionEditorY = y;
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("x: %d; y: %d for item %s(%x)"), x, y, *item->GetName(), item);
		}
	}
#endif
	int32 x = -256;
	const int32 xPadding = 32;
	const int32 yPadding = 32;
	const int32 yOutputSize = 8;
	const int32 yHeaderSize = 8;
	/*
	const int32 xItemSize = 128;
	const int32 yItemSize = 256;
	const int32 yMinSize = 32;
	*/

	auto getItemHeight = [&](UMaterialExpression *arg) -> auto{
		if (!arg)
			return 0;
		auto h = arg->GetHeight();
		auto numOuts = arg->Outputs.Num();
		auto outputSize = yHeaderSize + yOutputSize *numOuts;
		if (h < outputSize)
			h = outputSize;
		return h;
	};

	for(int col = 0; col < usedLevels.Num(); col++){
		int levelIndex = usedLevels[col];
		const auto &curLevel = levelOrder.FindOrAdd(levelIndex);
		int curNumRows = curLevel.Num();

		int xSize = 0;
		int ySize = 0;
		for (int row = 0; row < curNumRows; row++){
			auto item = curLevel[row];
			if (!item)
				continue;
			auto w = item->GetWidth();
			auto h = getItemHeight(item);//item->GetHeight();
			if (w > xSize)
				xSize = w;

			ySize += h;
			if (row > 0)
				ySize += yPadding;
		}

		//int ySize = yItemSize * curNumRows;
		int yOffset = -ySize/2;
		int y = yOffset;
		x -= xSize;
		for (int row = 0; row < curNumRows; row++){
			//int y = yOffset + row * ySize;
			auto item = curLevel[row];
			if (!item)
				continue;
			auto w = item->GetWidth();
			auto h = getItemHeight(item);//item->GetHeight();
			item->MaterialExpressionEditorX = x;
			item->MaterialExpressionEditorY = y;
			y += h;
			if (row)
				y += yPadding;
		}
		x -= xPadding;
	}
}

UMaterialExpression* makeTextureTransformNodes(UMaterial* material, 
	const FVector2D &scaleVec, const FVector2D& offsetVec, int coordIndex = 0, 
	const TCHAR* coordNodeName = 0, const TCHAR* coordScaleParamName = 0, const TCHAR* coordOffsetParamName = 0, 
	bool coordNodeOnly = false){

/*
	result.xy = src.xy * scale.xy + offset.xy.... however, due to .y coordinate it turns into

	(src.x * scale.x + offset.x, 1.0 - ((1.0 - src.y) * scale.y + offset.y)) -->
	(src.x * scale.x + offset.x, 1.0 - (scale.y - src.y * scale.y + offset.y)) -->
	(src.x * scale.x + offset.x, 1.0 - scale.y + src.y * scale.y - offset.y) -->
	(src.x * scale.x + offset.x, src.y * scale.y + 1.0 - scale.y - offset.y) -->
	(src.x * scale.x, src.y * scale.y) + (offset.x, 1.0 - scale.y - offset.y)
*/

	auto texCoord = createExpression<UMaterialExpressionTextureCoordinate>(material, coordNodeName);
	texCoord->CoordinateIndex = coordIndex;

	if (coordNodeOnly)
		return texCoord;

	auto uvScale = createVectorParameterExpression(material, scaleVec, coordScaleParamName);
	auto uvOffset = createVectorParameterExpression(material, offsetVec, coordOffsetParamName);

	auto uvScaleVec2 = createExpression<UMaterialExpressionAppendVector>(material);//(scale.x, scale.y)
	uvScale->ConnectExpression(&uvScaleVec2->A, 1);
	uvScale->ConnectExpression(&uvScaleVec2->B, 2);

	// (src.x * scale.x, src.y * scale.y)
	auto mul = createMulExpression(material, texCoord, uvScaleVec2, TEXT("(src.x * scale.x, src.y * scale.y)"));//

	//1.0 - scale.y
	auto subY = createExpression<UMaterialExpressionOneMinus>(material, TEXT("1.0 - scale.y"));
	uvScale->ConnectExpression(&subY->Input, 2);

	//1.0 - scale.y - offset.y
	auto subY2 = createExpression<UMaterialExpressionSubtract>(material, TEXT("1.0 - scale.y - offset.y"));
	subY2->A.Expression = subY;
	uvOffset->ConnectExpression(&subY2->B, 2);

	//(offset.x, 1.0 - scale.y - offset.y)
	auto offset2 = createAppendVectorExpression(material, 0, subY2, TEXT("(offset.x, 1.0 - scale.y - offset.y)"));
	uvOffset->ConnectExpression(&offset2->A, 1);

	//(src.x * scale.x, src.y * scale.y) + (offset.x, 1.0 - scale.y - offset.y)
	auto add = createAddExpression(material, mul, offset2, TEXT("(src.x * scale.x, src.y * scale.y) + (offset.x, 1.0 - scale.y - offset.y)"));

	return add;
}

/*
UMaterialExpression* makeTextureTransformNodes(UMaterial* material, 
	const FVector2D &scaleVec, const FVector2D& offsetVec, int coordIndex = 0, 
	const TCHAR* coordNodeName = 0, const TCHAR* coordScaleParamName = 0, const TCHAR* coordOffsetParamName = 0, 
	bool coordNodeOnly = false){

	auto texCoord = createExpression<UMaterialExpressionTextureCoordinate>(material, coordNodeName);
	texCoord->CoordinateIndex = coordIndex;

	if (coordNodeOnly)
		return texCoord;

	auto uvScale = createVectorParameterExpression(material, scaleVec, coordScaleParamName);
	auto uvOffset = createVectorParameterExpression(material, offsetVec, coordOffsetParamName);

	auto uvScaleVec2 = createExpression<UMaterialExpressionAppendVector>(material);
	auto uvOffsetVec2 = createExpression<UMaterialExpressionAppendVector>(material);

	uvScale->ConnectExpression(&uvScaleVec2->A, 1);
	uvScale->ConnectExpression(&uvScaleVec2->B, 2);

	uvOffset->ConnectExpression(&uvOffsetVec2->A, 1);
	uvOffset->ConnectExpression(&uvOffsetVec2->B, 2);

	auto add = createExpression<UMaterialExpressionAdd>(material);
	auto mul = createExpression<UMaterialExpressionMultiply>(material);
	add->A.Expression = mul;
	add->B.Expression = uvOffsetVec2;

	mul->A.Expression = texCoord;
	mul->B.Expression = uvScaleVec2;

	return add;
}
*/


void MaterialBuilder::processMainUv(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.mainTextureTransform)
		return;

	if (!fingerprint.albedoTex && !fingerprint.normalmapTex && !fingerprint.metallicTex 
		&& !fingerprint.specularTex && !fingerprint.occlusionTex && !fingerprint.parallaxTex 
		&& !fingerprint.detailMaskTex && !fingerprint.emissionTex)
		return;

	auto coordExpr = makeTextureTransformNodes(material, jsonMat.mainTextureScale, jsonMat.mainTextureOffset, 0, 
		TEXT("Main UV coords"), TEXT("Main UV scale"), TEXT("Main UV offset"));

	buildData.mainUv = coordExpr;
}

void MaterialBuilder::processParallax(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	//if (!fingerprint.
	if (!fingerprint.parallaxTex)
		return;

	auto parallaxTex = buildData.importer->getTexture(jsonMat.parallaxTex);
	auto parallaxTexExpr = createTextureExpression(material, parallaxTex, TEXT("Parallax"));
	if (buildData.mainUv){
		parallaxTexExpr->Coordinates.Expression = buildData.mainUv;
	}
	auto parallaxScaleExpr = createScalarParameterExpression(material, jsonMat.parallaxScale, TEXT("Parallax Scale"));

	//material->parallax
}

void MaterialBuilder::processDetailUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	//if (!fingerprint.
	if (!fingerprint.detailAlbedoTex && !fingerprint.detailNormalTex)
		return;

	auto texCoord = makeTextureTransformNodes(material, jsonMat.detailAlbedoScale, jsonMat.detailAlbedoOffset, jsonMat.secondaryUv, 
		TEXT("Detail UV coords"), TEXT("Detail UV scale"), TEXT("Detail UV offset"), !fingerprint.detailTextureTransform);

	buildData.detailUv = texCoord;
}

void MaterialBuilder::processAlbedo(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	UE_LOG(JsonLog, Log, TEXT("Creating albedo"));

	auto albedoColorExpr = createVectorParameterExpression(material, jsonMat.colorGammaCorrected, TEXT("Albedo Color"));
	buildData.albedoExpression = albedoColorExpr;
	buildData.albedoColorExpression = albedoColorExpr;

	//texture
	if (fingerprint.albedoTex){
		auto albedoTex = buildData.importer->getTexture(jsonMat.albedoTex);
		if (albedoTex){
			auto texExpr = createTextureExpression(material, albedoTex, TEXT("Albedo Texture"), false);

			buildData.albedoTexExpression = texExpr;
			if (buildData.mainUv){
				texExpr->Coordinates.Expression = buildData.mainUv;
			}
			auto mul = createExpression<UMaterialExpressionMultiply>(material);
			mul->A.Expression = texExpr;
			mul->B.Expression = albedoColorExpr;
			
			buildData.albedoExpression = mul;
		}
	}

	//detail
	if (fingerprint.detailAlbedoTex){
		auto detailAlbedoTex = buildData.importer->getTexture(jsonMat.detailAlbedoTex);
		if (detailAlbedoTex){
			auto texExpr = createTextureExpression(material, detailAlbedoTex, TEXT("Detail Map(Albedo"), false);
			buildData.albedoDetailTexExpression = texExpr;
			if (buildData.detailUv){
				texExpr->Coordinates.Expression = buildData.detailUv;
			}

			UMaterialExpression *detailData = texExpr;
			if (buildData.detailMaskExpression){
				auto detailLerp = createExpression<UMaterialExpressionLinearInterpolate>(material);
				auto constWhite = createExpression<UMaterialExpressionConstant4Vector>(material);
				constWhite->Constant = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);//FLinearColor::White;
				detailLerp->A.Expression = constWhite;
				buildData.detailMaskExpression->ConnectExpression(&detailLerp->Alpha, 4);
				detailLerp->B.Expression = detailData;

				detailData = detailLerp;
			}

			auto mul = createExpression<UMaterialExpressionMultiply>(material);
			mul->A.Expression = detailData;
			mul->B.Expression = buildData.albedoExpression;

			auto mulx2 = createMulExpression(material, mul, 0);
			mulx2->ConstB = 2.0f;

			buildData.albedoExpression = mulx2;
		}
	}

	material->BaseColor.Expression = buildData.albedoExpression;
}

void MaterialBuilder::processNormalMap(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (fingerprint.normalmapTex){
		auto normalMapTex = buildData.importer->getTexture(jsonMat.normalMapTex);
		auto normTexExpr = createTextureExpression(material, normalMapTex, TEXT("Normal Map(main)"), true);
		if (buildData.mainUv){
			normTexExpr->Coordinates.Expression = buildData.mainUv;
		}
		buildData.normalExpression = normTexExpr;
		buildData.normalTexExpression = normTexExpr;

		if (fingerprint.normalMapIntensity){
			auto bumpScaleParam = createScalarParameterExpression(
				material, jsonMat.bumpScale, TEXT("Bump Scale (Normal intensity)"));
			auto scale = makeNormalMapScaler(material, normTexExpr, bumpScaleParam);
			buildData.normalExpression = scale;
		}
	}

	if (fingerprint.detailNormalTex){
		auto detailNormalMapTex = buildData.importer->getTexture(jsonMat.detailNormalMapTex);
		auto detNormTexExpr = createTextureExpression(material, detailNormalMapTex, TEXT("Normal Map(detail)"), true);
		buildData.detailNormalTexExpression = detNormTexExpr;
		buildData.detailNormalExpression = detNormTexExpr;
		if (buildData.detailUv){
			detNormTexExpr->Coordinates.Expression = buildData.detailUv;
		}

		if (fingerprint.detailNormalMapScale){
			auto detailNormScaleParam = createScalarParameterExpression(
				material, jsonMat.bumpScale, TEXT("Detail Normal Scale (DetailNormal intensity)"));
			auto detScale = makeNormalMapScaler(material, detNormTexExpr, detailNormScaleParam);
			buildData.detailNormalExpression  = detScale;
		}

		if (!buildData.normalExpression){
			buildData.normalExpression = buildData.detailNormalExpression;
		}
		else{
			auto expr = makeNormalBlend(material, buildData.normalExpression, buildData.detailNormalExpression);
			buildData.normalExpression = expr;
		}
	}

	if (buildData.normalExpression){
		material->Normal.Expression = buildData.normalExpression;
	}
}

void MaterialBuilder::processEmissive(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.emissionEnabled)
		return;

	auto emissiveColor = createVectorParameterExpression(material, jsonMat.emissionColor, TEXT("Emissive color"));
	UMaterialExpression *emissiveExpr = emissiveColor;

	UTexture *emissiveTex = buildData.importer->getTexture(jsonMat.emissionTex);
	if (emissiveTex){
		auto emissiveTexExpr = createTextureExpression(material, emissiveTex, TEXT("Emissive Texture"));
		if (buildData.mainUv)
			emissiveTexExpr->Coordinates.Expression = buildData.mainUv;
		auto mul = createExpression<UMaterialExpressionMultiply>(material);
		mul->A.Expression = emissiveTexExpr;
		mul->B.Expression = emissiveColor;

		emissiveExpr = mul;
	}

	material->EmissiveColor.Expression = emissiveExpr;
}

void MaterialBuilder::processDetailMask(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.detailMaskTex || !fingerprint.hasDetailMaps())
		return;

	auto detailTex = buildData.importer->getTexture(jsonMat.detailMaskTex);

	auto detailTexNode = createTextureExpression(material, detailTex, TEXT("Detail texture"), false);
	if (buildData.mainUv){
		detailTexNode->Coordinates.Expression = buildData.mainUv;
	}

	buildData.detailMaskExpression = detailTexNode;
}

void MaterialBuilder::processOcclusion(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.occlusionTex)
		return;

	auto occlusionTex = buildData.importer->getTexture(jsonMat.occlusionTex);
	auto occlusionTexExpr = createTextureExpression(material, occlusionTex, TEXT("Occlusion texture"));
	UMaterialExpression *occlusionExpr = occlusionTexExpr;
	if (fingerprint.occlusionIntensity){
		auto occlusionIntensityParam = createScalarParameterExpression(material, jsonMat.occlusionStrength, TEXT("Occlusion intensity"));

		auto lerpNode = createExpression<UMaterialExpressionLinearInterpolate>(material);
		lerpNode->A.Expression = occlusionExpr;
		lerpNode->Alpha.Expression = occlusionIntensityParam;
		lerpNode->ConstB = 1.0f;

		occlusionExpr = lerpNode;
	}

	if (!occlusionExpr)
		return;

	material->AmbientOcclusion.Expression = occlusionExpr;
}

void MaterialBuilder::processMetallic(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (fingerprint.specularModel)
		return;

	UMaterialExpression *metallicExpr = nullptr;
	if (fingerprint.metallicTex){
		auto metallicTex = buildData.importer->getTexture(jsonMat.metallicTex);
		auto texExpr = createTextureExpression(material, metallicTex, TEXT("Metallic (texture)"));
		if (buildData.mainUv)
			texExpr->Coordinates.Expression = buildData.mainUv;
		buildData.metallicTexExpression = texExpr;
		metallicExpr = texExpr;
	}

	if (!metallicExpr){
		auto metalParam = createScalarParameterExpression(material, jsonMat.metallic, TEXT("Metallic"));
		metallicExpr = metalParam;
	}

	material->Metallic.Expression = metallicExpr;
}

void MaterialBuilder::processSpecular(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.specularModel)
		return;

	//UMaterialExpression *specExpr = nullptr;

	//TODO... remove color if specular is white?
	//Actually, specular color texture can't be tinted in unity. Which is odd.
	if (fingerprint.specularTex){
		auto specTex = buildData.importer->getTexture(jsonMat.specularTex);
		auto specTexNode = createTextureExpression(material, specTex, TEXT("Specular (texture)"));
		if (buildData.mainUv){
			specTexNode->Coordinates.Expression = buildData.mainUv;
		}

		/*
		auto mul = createExpression<UMaterialExpressionMultiply>(material);
		mul->A.Expression = specTexNode;
		mul->B.Expression = specExpr;
		*/

		buildData.specularTexExpression = specTexNode;
		buildData.specularExpression = specTexNode;//mul;
	}
	else{
		auto specColor = createVectorParameterExpression(material, jsonMat.specularColor, TEXT("Specular (color)"));
		buildData.specularColorExpression = specColor;
		buildData.specularExpression = specColor;
	}

	/*
	TODO: The whole approximation thing
	*/
	{
		auto rMask = createComponentMask(material, buildData.specularExpression, true, false, false, false);
		auto gMask = createComponentMask(material, buildData.specularExpression, false, true, false, false);
		auto bMask = createComponentMask(material, buildData.specularExpression, false, false, true, false);

		auto max1 = createExpression<UMaterialExpressionMax>(material);
		max1->A.Expression = rMask;
		max1->B.Expression = gMask;
		auto max2 = createExpression<UMaterialExpressionMax>(material);
		max2->A.Expression = max1;
		max2->B.Expression = bMask;

		auto metal = createMulExpression(material, max2, max2);
		auto lerp = createExpression<UMaterialExpressionLinearInterpolate>(material);

		lerp->Alpha.Expression = metal;
		lerp->A.Expression = buildData.albedoExpression;
		lerp->B.Expression = buildData.specularExpression;

		material->BaseColor.Expression = lerp;
		material->Metallic.Expression = metal;
	}

	//material->Specular.Expression = specExpr;
}

void MaterialBuilder::processRoughness(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	//Well, unity went ahead and added roughness-based shader, apparently. Sigh. I'll need to look into this later.

	auto constRough = createScalarParameterExpression(material, 1.0f - jsonMat.smoothness, TEXT("Roughness"));
	UMaterialExpression *roughExpr = constRough;

	UMaterialExpression *smoothSource = fingerprint.altSmoothnessTexture ? buildData.albedoTexExpression: buildData.smoothTexSource;

	//UMaterialExpression *roughExpression = nullptr;
	if (smoothSource){
		auto smoothMask = createComponentMask(material, smoothSource, false, false, false, true);
		auto converter = createExpression<UMaterialExpressionOneMinus>(material);
		converter->Input.Expression = smoothMask;

		auto lerp = createExpression<UMaterialExpressionLinearInterpolate>(material);
		lerp->Alpha.Expression = converter;
		lerp->A.Expression = constRough;
		lerp->ConstB = 1.0f;

		roughExpr = lerp;
	}

	material->Roughness.Expression = roughExpr;
}

#ifndef MATBUILDER_OLDGEN

void MaterialBuilder::buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){

	//sets up tex coordinate nodes...
	processMainUv(material, jsonMat, fingerprint, buildData);

	//this is a stub
	//processParallax(material, jsonMat, fingerprint, buildData);

	//detail coordinates, if necessary.
	processDetailUv(material, jsonMat, fingerprint, buildData);
	//this one creates detail mask
	processDetailMask(material, jsonMat, fingerprint, buildData);
	//albedo and albedo detail
	processAlbedo(material, jsonMat, fingerprint, buildData);
	//normalmap and normalmap detail
	processNormalMap(material, jsonMat, fingerprint, buildData);
	//specular and specular detail
	processSpecular(material, jsonMat, fingerprint, buildData);
	//occlsion and occlusion slider
	processOcclusion(material, jsonMat, fingerprint, buildData);
	//emissive color and texture
	processEmissive(material, jsonMat, fingerprint, buildData);
	//metallic color and texture
	processMetallic(material, jsonMat, fingerprint, buildData);
	//roughness/smoothness
	processRoughness(material, jsonMat, fingerprint, buildData);
	//opacity
	processOpacity(material, jsonMat, fingerprint, buildData);
	//sort this as a grid.

	//arrangeNodesGrid(material, jsonMat, fingerprint, buildData);
	arrangeNodesTree(material);//, jsonMat, fingerprint, buildData);
}

#endif

void MaterialBuilder::processOpacity(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	bool translucent = false;
	if (jsonMat.isTransparentQueue()){
		material->BlendMode = BLEND_Translucent;
		translucent = true;
	}
	if (jsonMat.isAlphaTestQueue())
		material->BlendMode = BLEND_Masked;
	if (jsonMat.isGeomQueue())
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (jsonMat.isTransparentQueue() || jsonMat.isAlphaTestQueue()) && !jsonMat.isGeomQueue();
	if (!needsOpacity)
		return;

	if (translucent){
		material->TranslucencyLightingMode = TLM_SurfacePerPixelLighting;//TLM_Surface;
	}

	auto &opacityTarget = jsonMat.isTransparentQueue() ? material->Opacity: material->OpacityMask;

	auto albedoTexExpression = buildData.albedoTexExpression;
	auto albedoColorExpression = buildData.albedoColorExpression;

	bool sourceNeedToSpecifyChannel = true;
	UMaterialExpression *opacitySource = nullptr;

	if (fingerprint.altSmoothnessTexture){
		albedoTexExpression = 0; //setting this flag effectively disables texture alpha as opacity source
	}
	
	//Detailmap alpha is not being used, as far as I can tell.
	if (albedoTexExpression && albedoColorExpression){
		auto opacityMul = createExpression<UMaterialExpressionMultiply>(material);
		albedoTexExpression->ConnectExpression(&opacityMul->A, 4);
		albedoColorExpression->ConnectExpression(&opacityMul->B, 4);
		opacitySource = opacityMul;
		sourceNeedToSpecifyChannel = false;
	}else if (albedoTexExpression != 0)
		opacitySource = albedoTexExpression;
	else if (albedoColorExpression != 0)
		opacitySource = albedoColorExpression;
	else{
		UE_LOG(JsonLog, Warning, TEXT("Could not find matchin opacity source in material %s"), *jsonMat.name);
		return;
	}

	//TODO: detail map?
	if (opacitySource){
		if (sourceNeedToSpecifyChannel)
			opacitySource->ConnectExpression(&opacityTarget, 4);
		else
			opacityTarget.Expression = opacitySource;
	}
}


UMaterial* MaterialBuilder::importMaterial(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId){
	MaterialFingerprint fingerprint(jsonMat);

	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		jsonMat.name, jsonMat.path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		importer->registerMaterialPath(jsonMat.id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//stuff
	MaterialBuildData buildData(matId, importer);
	buildMaterial(material, jsonMat, fingerprint, buildData);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();

		importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return material;
}

UMaterial* MaterialBuilder::importMaterial(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId){
	UE_LOG(JsonLog, Log, TEXT("Importing material %d"), matId);

	JsonMaterial jsonMat(obj);

	return importMaterial(jsonMat, importer, matId);
}

UMaterialExpression* createTerrainLayerCoords(UMaterial *material, const JsonTerrain &terr, const JsonTerrainData &terrData
	, const FVector2D& splatSize, const FVector2D& splatOffset, const FIntPoint &terrainVertSize,  const TCHAR* text = 0){
	auto landCoord = createExpression<UMaterialExpressionLandscapeLayerCoords>(material, text);

	//we'll need another mapping rotate/scale module here, most likely
	//Let's postpone for now.
	landCoord->MappingScale = 1.0f;//1 per one vert unit//100.0f;//this means that 
	landCoord->MappingPanU = 0.0f;//offset.X;
	landCoord->MappingPanV = 0.0f;//offset.Y;

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

	auto finalUvOffset = FVector2D(0.0f, 0.0f);
	finalUvOffset.Y = 1.0f - FMath::Frac(quadToFinalScale.Y);
	finalUvOffset.X += splatUvOffset.X;
	finalUvOffset.Y -= splatUvOffset.Y;

	auto downScaleUvConst = createExpression<UMaterialExpressionConstant4Vector>(material, TEXT("Terrain channel downscale"));
	downScaleUvConst->Constant.R = ueToQuadUv.X;
	downScaleUvConst->Constant.G = ueToQuadUv.Y;
	auto downScaleUvVal = createComponentMask(material, true, true, false, false);
	downScaleUvVal->Input.Expression = downScaleUvConst;

	auto scaleUvConst = createExpression<UMaterialExpressionConstant4Vector>(material, TEXT("Terrain channel scale2"));
	scaleUvConst->Constant.R = quadToFinalScale.X;
	scaleUvConst->Constant.G = quadToFinalScale.Y;
	auto scaleUvVal = createComponentMask(material, true, true, false, false);
	scaleUvVal->Input.Expression = scaleUvConst;
	 
	auto offsetUvConst = createExpression<UMaterialExpressionConstant4Vector>(material, TEXT("Terrain channel offset"));
	offsetUvConst->Constant.R = finalUvOffset.X;
	offsetUvConst->Constant.G = finalUvOffset.Y;
	auto offsetUvVal = createComponentMask(material, true, true, false, false);
	offsetUvVal ->Input.Expression = offsetUvConst;

	auto mul = createMulExpression(material, landCoord, downScaleUvVal);
	auto mul2 = createMulExpression(material, mul, scaleUvVal);
	auto add = createAddExpression(material, mul2, offsetUvVal);

	auto swapMaskX = createComponentMask(material, true, false, false, false);
	swapMaskX->Input.Expression = add;
	auto swapMaskY = createComponentMask(material, false, true, false, false);
	swapMaskY->Input.Expression = add;

	auto append = createAppendVectorExpression(material, swapMaskY, swapMaskX);

	return append;

	//return result;
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

void MaterialBuilder::buildTerrainMaterial(UMaterial *material, const JsonGameObject &gameObj, 
		const JsonTerrain &terr, const JsonTerrainData &terrData, 
		const FIntPoint &terrainVertSize, JsonImporter *importer){

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

	//no specular
	arrangeNodesTree(material);
}

UMaterial* MaterialBuilder::buildTerrainMaterial(const JsonGameObject &gameObj,
		const JsonTerrain &terr, const JsonTerrainData &terrainData, const FIntPoint &terrainVertSize, JsonImporter *importer){
	FString terrPath, terrFileName, terrExt;
	FPaths::Split(terrainData.exportPath, terrPath, terrFileName, terrExt);

	auto materialName = terrainData.name + TEXT("_Material");
	auto basePackageName = sanitizePackageName(materialName);
	auto importPath = importer->getProjectImportPath();
	auto materialPackagePath = buildPackagePath(basePackageName, 
		&terrPath, &importPath, &importer->getAssetCommonPath());

	UE_LOG(JsonLogTerrain, Log, TEXT("Creating material package for terrain \"%s\"(\"%s\") at \"%s\""), 
		*terrainData.name, *terrainData.exportPath, *materialPackagePath);

	auto materialPackage = CreatePackage(0, *materialPackagePath);

	/*
	//Nope. We need a factory.

	auto materialObj = NewObject<UMaterial>(materialPackage, UMaterial::StaticClass(), *sanitizeObjectName(*materialName), 
			RF_Standalone|RF_Public);
	*/

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* materialObj = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), 
		materialPackage, 
		FName(*basePackageName), 
		RF_Standalone|RF_Public,
		0, GWarn
	);

	buildTerrainMaterial(materialObj, gameObj, terr, terrainData, terrainVertSize, importer);

	if (materialObj){
		materialObj->PreEditChange(0);
		materialObj->PostEditChange();

		//importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(materialObj);
		materialPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return materialObj;
}


UMaterial* MaterialBuilder::createMaterial(const FString& name, const FString &path, JsonImporter *importer, 
		MaterialCallbackFunc newCallback, MaterialCallbackFunc existingCallback, MaterialCallbackFunc postEditCallback){
	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		name, path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		if (existingCallback)
			existingCallback(existingMaterial);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	if (newCallback)
		newCallback(material);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();
		
		if (postEditCallback)
			postEditCallback(material);
		//importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return material;
}
