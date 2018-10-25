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
#include "Materials/MaterialExpressionOneMinus.h"

#include "MaterialTools.h"

using namespace MaterialTools;

void MaterialBuilder::arrangeNodes(UMaterial* material, const JsonMaterial &jsonMat, 
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

void MaterialBuilder::processMainUv(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.mainTextureTransform)
		return;

	if (!fingerprint.albedoTex && !fingerprint.normalmapTex && !fingerprint.metallicTex 
		&& !fingerprint.specularTex && !fingerprint.occlusionTex && !fingerprint.parallaxTex 
		&& !fingerprint.detailMaskTex && !fingerprint.emissionTex)
		return;

	auto texCoord = createExpression<UMaterialExpressionTextureCoordinate>(material, TEXT("Main UV coords"));
	auto uvScale = createVectorParameterExpression(material, jsonMat.mainTextureScale, TEXT("Main UV scale"));
	auto uvOffset = createVectorParameterExpression(material, jsonMat.mainTextureOffset, TEXT("Main UV offset"));

	auto add = createExpression<UMaterialExpressionAdd>(material);
	auto mul = createExpression<UMaterialExpressionMultiply>(material);
	add->A.Expression = mul;
	add->B.Expression = uvOffset;

	mul->A.Expression = texCoord;
	mul->B.Expression = uvScale;

	buildData.mainUv = mul;
}

void MaterialBuilder::processDetailUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	//if (!fingerprint.
	if (!fingerprint.detailAlbedoTex && !fingerprint.detailNormalTex)
		return;

	auto texCoord = createExpression<UMaterialExpressionTextureCoordinate>(material, TEXT("Detail UV coords"));
	texCoord->CoordinateIndex = (int32)fingerprint.secondaryUv;

	if (!fingerprint.detailTextureTransform){
		buildData.detailUv = texCoord;
		return;
	}

	auto uvScale = createVectorParameterExpression(material, jsonMat.mainTextureScale, TEXT("Detail UV scale"));
	auto uvOffset = createVectorParameterExpression(material, jsonMat.mainTextureOffset, TEXT("Detail UV offset"));

	auto add = createExpression<UMaterialExpressionAdd>(material);
	auto mul = createExpression<UMaterialExpressionMultiply>(material);
	add->A.Expression = mul;
	add->B.Expression = uvOffset;

	mul->A.Expression = texCoord;
	mul->B.Expression = uvScale;

	buildData.detailUv = mul;
}

void MaterialBuilder::processAlbedo(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	UE_LOG(JsonLog, Log, TEXT("Creating albedo"));

	auto albedoColorExpr = createVectorParameterExpression(material, jsonMat.emissionColorGammaCorrected, TEXT("Albedo Color"));
	buildData.albedoExpression = albedoColorExpr;
	buildData.albedoColorExpression = albedoColorExpr;

	//texture
	if (fingerprint.albedoTex){
		albedoTex = buildData.importer->getTexture(jsonMat.albedoTex);
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
		detailAlbedoTex = buildData.importer->getTexture(jsonMat.detailAlbedoTex);
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
				constWhite->Constant = FLinearColor::White;
				detailLerp->A.Expression = constWhite;
				buildData.detailMaskExpression->ConnectExpression(&detailLerp->Alpha, 4);
				detailLerp->B.Expression = detailData;

				detailData = detailLerp;
			}

			auto mul = createExpression<UMaterialExpressionMultiply>(material);
			mul->A.Expression = detailData;//texExpr;
			mul->B.Expression = buildData.albedoExpression;

			buildData.albedoExpression = mul;
		}
	}

	material->BaseColor.Expression = buildData.albedoExpression;

	/*
	UMaterialExpressionTextureSample *albedoTexExpression = 0;
	UMaterialExpressionVectorParameter *albedoColorExpression = 0;

	UTexture *mainTexture = buildData.importer->getTexture(jsonMat.mainTexture);
	auto albedoSource = createMaterialInputMultiply(material, mainTexture, &jsonMat.colorGammaCorrected, material->BaseColor, 
		TEXT("Albedo(Texture)"), TEXT("Albedo(Color)"), &albedoTexExpression, &albedoColorExpression);

	*/
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
	}

	if (fingerprint.detailNormalTex){
		auto detailNormalMapTex = buildData.importer->getTexture(jsonMat.detailNormalMapTex);
		auto detNormTexExpr = createTextureExpression(material, detailNormalMapTex, TEXT("Normal Map(detail)"), true);
		buildData.detailNormalTexExpression = detNormTexExpr;
		if (buildData.detailUv){
			detNormTexExpr->Coordinates.Expression = buildData.detailUv;
		}

		if (buildData.normalExpression){
			auto lerp = createExpression<UMaterialExpressionLinearInterpolate>(material);
			lerp->A.Expression = buildData.normalExpression;
			if (buildData.detailMaskExpression){
				auto mul = createExpression<UMaterialExpressionMultiply>(material);
				auto constHalf = createExpression<UMaterialExpressionConstant>(material);
				constHalf->R = 0.5f;
				mul->B.Expression = constHalf;
				buildData.detailMaskExpression->ConnectExpression(&mul->A.Expression, 4);
				lerp->Alpha = mul;
			}
			else{
				lerp->ConstAlpha = 0.5f;
			}
			lerp->B.Expression = detNormTexExpr;

			auto norm = createExpression<UMaterialExpressionNormalize>(material);
			norm->VectorInput.Expression = lerp;

			buildData.normalExpression = norm;
		}
		else{
			buildData.normalExpression = detNormTexExpr;
		}
	}

	if (fingerprint.normalMapIntensity && buildData.normalExpression){
		auto flatNormal = createExpression<UMaterialExpressionConstant3Vector>(material, TEXT("Flat normal"));
		flatNormal->Constant = FLinearColor(0.0f, 0.0f, 1.0f);

		auto constParam = createExpression<UMaterialExpressionConstant>(material, TEXT("Bump scale"));
		/*
			WARNING:
			This is NOT the formula used by unity. Unity multiplies *.xy by bumpScale, and then computes z based on pythagora's theorem.
		*/
		constParam->R = jsonMat.bumpScale;

		auto flattener = createExpression<UMaterialExpressionLinearInterpolate>(material, TEXT("Flatten normal"));
		flattener->A.Expression = flatNormal;
		flattener->B.Expression = buildData.normalExpression;
		flattener->Alpha.Expression = constParam;

		buildData.normalExpression = flattener;
	}

	if (buildData.normalExpression){
		material->Normal.Expression = buildData.normalExpression;
	}

	/*
	if (jsonMat.useNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Creating normal map"));

		auto normalMapTex = buildData.importer->getTexture(jsonMat.normalMapTex);
		createMaterialInput(material, normalMapTex, nullptr, material->Normal, true, TEXT("Normal"));
	}
	*/
}

void MaterialBuilder::processEmissive(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.emissionEnabled)
		return;

	auto emissiveColor = createVectorParameterExpression(material, jsonMat.emissionColor, TEXT("Emissive color");
	UMaterialExpression *emissiveExpr = nullptr;

	UTexture *emissiveTex = buildData.importer->getTexture(jsonMat.emissionTex);
	if (emissiveTex){
		auto texExpr = createTextureExpression(material, emissiveTex, TEXT("Emissive TExture"));
		if (buildData.mainUv)
			texExpr->Coordinates.Expression = buildData.mainUv;
		auto mul = createExpression<UMaterialExpressionMultiply>(material);
		mul->A.Expression = emissiveTex;
		mul->B.Expression = emissiveColor;

		emissiveExpr = mul;
	}

	material->EmissiveColor.Expression = emissiveExpr;
}

void MaterialBuilder::processDetailMask(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (!fingerprint.detailMaskTex || !fingerprint.hasDetailMaps())
		return;

	auto detailTex = buildData.importer->getTexture(detailMaskTex);

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
		if (metallicTex){
			auto texExpr = createTextureExpression(material, metallicTex, TEXT("Metallic (texture)"));
			if (buildData.mainUv)
				texExpr->Coordinates.Expression = buildData.mainUv;
			buildData.metallicTexExpression = texExpr;
			metallicExpr = texExpr;
		}
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

	UMaterialExpression *specExpr = nullptr;

	auto specColor = createVectorParameterExpression(material, jsonMat.specularColor, TEXT("Specular (color)"));
	specExpr = specColor;
	//TODO... remove color if specular is white?

	if (fingerprint.specularTex){
		auto specTex = buildData.importer->getTexture(jsonMat.specularTex);
		auto specTexNode = createTextureExpression(material, specTex, TEXT("Specular (texture)"));
		buildData.specularTexExpression = specTexNode;

		auto mul = createExpression<UMaterialExpressionMultiply>(material);
		mul->A.Expression = specTexNode;
		mul->B.Expression = specExpr;

		specExpr = mul;
	}

	/*
	TODO: The whole approximation thing
	*/
	material->Specular.Expression = specExpr;
}

void MaterialBuilder::processRoughness(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	//Well, unity went ahead and added roughness-based shader, apparently. Sigh. I'll need to look into this later.

	UMaterialExpresion *smoothSource = fingerprint.altSmoothnessTexture ? buildData.albedoTexExpression: buildData.smoothTexSource;

	//UMaterialExpression *roughExpression = nullptr;
	if (!smoothSource){
		auto constRough = createScalarParameterExpression(material, 1.0f - jsonMat.smoothness, TEXT("Roughness"));
		material->Roughness.Expression = constRough;
		return;
	}

	auto *converter = createExpression<UMaterialExpressionOneMinus>(material);
	converter->ConnectExpression(&converter->Input, 4);
	material->Roughness.Expression = converter;
}

void MaterialBuilder::buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){

	processMainUv(material, jsonMat, fingerprint, buildData);
	processDetailUv(material, jsonMat, fingerprint, buildData);

	processDetailMask(material, jsonMat, fingerprint, buildData);
	processAlbedo(material, jsonMat, fingerprint, buildData);

	/*
	logValue("Transparent queue", jsonMat.isTransparentQueue());
	logValue("Alpha test queue", jsonMat.isAlphaTestQueue());
	logValue("Geom queue", jsonMat.isGeomQueue());
	*/

#if 0
	//albedo
	UMaterialExpressionTextureSample *albedoTexExpression = 0;
	UMaterialExpressionVectorParameter *albedoColorExpression = 0;

	UTexture *mainTexture = buildData.importer->getTexture(jsonMat.mainTexture);
	auto albedoSource = createMaterialInputMultiply(material, mainTexture, &jsonMat.colorGammaCorrected, material->BaseColor, 
		TEXT("Albedo(Texture)"), TEXT("Albedo(Color)"), &albedoTexExpression, &albedoColorExpression);
#endif		
	processNormalMap(material, jsonMat, fingerprint, buildData);

#if 0
	if (jsonMat.useNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Creating normal map"));

		auto normalMapTex = buildData.importer->getTexture(jsonMat.normalMapTex);
		createMaterialInput(material, normalMapTex, nullptr, material->Normal, true, TEXT("Normal"));
	}
#endif

#if 0
	UE_LOG(JsonLog, Log, TEXT("Creating specular"));//TODO: connect specular alpha to smoothness

	UMaterialExpressionTextureSample *specTexExpression = 0;

	if (jsonMat.hasSpecular){
		auto specularTex = buildData.importer->getTexture(jsonMat.specularTex);
		createMaterialInput(material, specularTex, &jsonMat.specularColorGammaCorrected, material->Specular, false, 
			TEXT("Specular Color"), &specTexExpression);
	}
	else{
		// ?? Not sure if this is correct
		//material->Specular.Expression = albedoSource;
		//Nope, this is not correct. 
	}
#endif
	processSpecular(material, jsonMat, fingerprint, buildData);

#if 0
	auto occlusionTex = buildData.importer->getTexture(jsonMat.occlusionTex);
	createMaterialInput(material, occlusionTex, nullptr, material->AmbientOcclusion, false, TEXT("Ambient Occlusion"));
#endif
	processOcclusion(material, jsonMat, fingerprint, buildData);

	processEmissive(material, jsonMat, fingerprint, buildData);

#if 0
	if (jsonMat.hasEmission){
		UE_LOG(JsonLog, Log, TEXT("Creating emissive"));

		UMaterialExpressionTextureSample *emissiveTexExp = 0;

		auto emissionTex = buildData.importer->getTexture(jsonMat.emissionTex);

		createMaterialInputMultiply(material, emissionTex, jsonMat.hasEmissionColor ? &jsonMat.emissionColor: 0, material->EmissiveColor, 
			TEXT("Emission Texture"), TEXT("Emission Color"));
		material->bUseEmissiveForDynamicAreaLighting = true;
		buildData.importer->registerEmissiveMaterial(buildData.matId);
	}
#endif

	processMetallic(material, jsonMat, fingerprint, buildData);
#if 0 
	//if (useMetallic){
	if (jsonMat.hasMetallic){
		UE_LOG(JsonLog, Log, TEXT("Creating metallic value"));
		createMaterialSingleInput(material, jsonMat.metallic, material->Metallic, TEXT("Metallic"));
	}
#endif

	processRoughness(material, jsonMat, fingerprint, buildData);

#if 0
	UE_LOG(JsonLog, Log, TEXT("hasMetallic: %d; hasSpecular: %d"), (int)(jsonMat.hasSpecular), (int)jsonMat.hasMetallic);
	UE_LOG(JsonLog, Log, TEXT("specularMode:%d"), (int)(jsonMat.useSpecular));
	UE_LOG(JsonLog, Log, TEXT("specTex exiss:%d"), (int)(specTexExpression != nullptr));
	if (specTexExpression)
		UE_LOG(JsonLog, Log, TEXT("num outputs: %d"), specTexExpression->Outputs.Num());

	//if (useSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	//useSpecular is false? the heck..
	//if ((specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	if (jsonMat.hasSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
		auto mulNode = NewObject<UMaterialExpressionMultiply>(material);
		material->Expressions.Add(mulNode);
		auto addNode = NewObject<UMaterialExpressionAdd>(material);
		material->Expressions.Add(addNode);
		addNode->ConstA = 1.0f;
		addNode->B.Expression = mulNode;
		mulNode->ConstA = -1.0f;

		specTexExpression->ConnectExpression(&mulNode->B, 4);
		material->Roughness.Expression = addNode;
	}
	else{
		createMaterialSingleInput(material, 1.0f - jsonMat.smoothness, material->Roughness, TEXT("Roughness"));
	}
#endif

	processOpacity(material, jsonMat, fingerprint, buildData);

	arrangeNodes(material, jsonMat, fingerprint, buildData);
}

void MaterialBuilder::processOpacity(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	if (jsonMat.isTransparentQueue())
		material->BlendMode = BLEND_Translucent;
	if (jsonMat.isAlphaTestQueue())
		material->BlendMode = BLEND_Masked;
	if (jsonMat.isGeomQueue())
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (jsonMat.isTransparentQueue() || jsonMat.isAlphaTestQueue()) && !jsonMat.isGeomQueue();
	if (!needsOpacity)
		return;

	auto &opacityTarget = jsonMat.isTransparentQueue() ? material->Opacity: material->OpacityMask;

	auto albedoTexExpression = buildData.albedoTexExpression;
	auto albedoColorExpression = buildData.albedoColorExpression;

	bool sourceNeedToSpecifyChannel = true;
	UMaterialExpression *opacitySource = nullptr;

	//TODO: Detailmap?
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
			opacitySource->ConnectExpression(&opacityTarget.Expression, 4);
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
