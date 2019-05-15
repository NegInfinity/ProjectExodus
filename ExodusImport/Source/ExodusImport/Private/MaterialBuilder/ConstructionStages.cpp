#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"

#include "MaterialTools.h"
#include "JsonImporter.h"
#include "JsonObjects/utilities.h"
#include "UnrealUtilities.h"

//#define MATBUILDER_OLDGEN

using namespace MaterialTools;
using namespace UnrealUtilities;

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
	//arrangeNodesTree(material);
	arrangeMaterialNodesAsTree(material);
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

