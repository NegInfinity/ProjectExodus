#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "DesktopPlatformModule.h"

UMaterialExpressionConstant* createConstantExpression(UMaterial *material, float value, const TCHAR* constantName){
	auto matConstant = NewObject<UMaterialExpressionConstant>(material);
	if (constantName){
		matConstant->SetParameterName(constantName);
		matConstant->Desc = constantName;
	}
	material->Expressions.Add(matConstant);
	matConstant->R = value;
	return matConstant;
}

UMaterialExpression* JsonImporter::createMaterialSingleInput(UMaterial *unrealMaterial, float value, FExpressionInput &matInput, const TCHAR* inputName){
	auto matConstant = NewObject<UMaterialExpressionConstant>(unrealMaterial);
	if (inputName){
		matConstant->SetParameterName(inputName);
		matConstant->Desc = inputName;
	}
	unrealMaterial->Expressions.Add(matConstant);
	matConstant->R = value;
	matInput.Expression = matConstant;
	return matConstant;
}

UMaterialExpressionVectorParameter* JsonImporter::createVectorExpression(UMaterial *material, FLinearColor color, const TCHAR* inputName){
	UMaterialExpressionVectorParameter* vecExpression = 
		NewObject<UMaterialExpressionVectorParameter>(material);
	material->Expressions.Add(vecExpression);

	//matInput.Expression = vecExpression;
	vecExpression->DefaultValue.R = color.R;
	vecExpression->DefaultValue.G = color.G;
	vecExpression->DefaultValue.B = color.B;
	vecExpression->DefaultValue.A = color.A;
	if (inputName){
		vecExpression->SetParameterName(inputName);
		vecExpression->Desc = inputName;
	}
	return vecExpression;
}

UMaterialExpressionTextureSample* JsonImporter::createTextureExpression(UMaterial *material, int32 matTextureId, const TCHAR* inputName, bool normalMap){
	UMaterialExpressionTextureSample *result = 0;
	UE_LOG(JsonLog, Log, TEXT("Creating texture sample expression"));

	logValue(TEXT("texId: "), matTextureId);
	auto unrealTex = loadTexture(matTextureId);

	if (!unrealTex){
		UE_LOG(JsonLog, Warning, TEXT("Texture not found"));
	}
	result = NewObject<UMaterialExpressionTextureSample>(material);
	result->SamplerType = normalMap ? SAMPLERTYPE_Normal: SAMPLERTYPE_Color;
	material->Expressions.Add(result);
	result->Texture = unrealTex;

	if (inputName){
		result->SetParameterName(inputName);
		result->Desc = inputName;
	}

	return result;
}

UMaterialExpression* JsonImporter::createMaterialInputMultiply(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, 
		const TCHAR* texParamName, const TCHAR* vecParamName,
		UMaterialExpressionTextureSample ** outTexNode,
		UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating multiply material input"));
	UMaterialExpressionTextureSample *texExp = 0;
	UMaterialExpressionVectorParameter *vecExp = 0;
	bool hasTex = matTextureId >= 0;
	if (hasTex){
		texExp = createTextureExpression(material, matTextureId, texParamName);
		if (outTexNode)
			*outTexNode = texExp;
	}
	if (matColor && ((*matColor != FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)) || !hasTex)){
		vecExp = createVectorExpression(material, *matColor, vecParamName);
		if (outVecParameter)
			*outVecParameter = vecExp;
	}

	UMaterialExpression* result = 0;
	if (vecExp && texExp){
		auto mulExp = createExpression<UMaterialExpressionMultiply>(material);
		mulExp->A.Expression = texExp;
		mulExp->B.Expression = vecExp;
		result = mulExp;
	}
	else if (vecExp){
		result = vecExp;
	}
	else if (texExp){
		result = texExp;
	}

	if (result){
		matInput.Expression = result;
	}

	return result;
}

UMaterialExpression* JsonImporter::createMaterialInput(UMaterial *unrealMaterial, int32 matTextureId, const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, 
		const TCHAR* paramName, UMaterialExpressionTextureSample **outTexNode, UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating material input"));
	logValue(TEXT("texId: "), matTextureId);
	auto unrealTex = loadTexture(matTextureId);

	if (unrealTex){
		UE_LOG(JsonLog, Log, TEXT("Texture found"));
		auto texExpression = NewObject<UMaterialExpressionTextureSample>(unrealMaterial);
		unrealMaterial->Expressions.Add(texExpression);
		matInput.Expression = texExpression;
		texExpression->Texture = unrealTex;
		texExpression->SamplerType = normalMap ? SAMPLERTYPE_Normal: SAMPLERTYPE_Color;
		if (outTexNode)
			*outTexNode = texExpression;
		if (paramName){
			texExpression->SetParameterName(paramName);
			texExpression->Desc = paramName;
		}
		return texExpression;
	}
	else{
		UE_LOG(JsonLog, Log, TEXT("Texture not found"));
		if (matColor){
			UE_LOG(JsonLog, Log, TEXT("Trying to create color node"));
			auto vecExpression = NewObject<UMaterialExpressionVectorParameter>(unrealMaterial);
			unrealMaterial->Expressions.Add(vecExpression);
			matInput.Expression = vecExpression;
			vecExpression->DefaultValue.R = matColor->R;
			vecExpression->DefaultValue.G = matColor->G;
			vecExpression->DefaultValue.B = matColor->B;
			vecExpression->DefaultValue.A = matColor->A;
			if (outVecParameter)
				*outVecParameter = vecExpression;
			if (paramName){
				vecExpression->SetParameterName(paramName);
				vecExpression->Desc = paramName;
			}
			return vecExpression;
		}
	}
	return nullptr;
}
