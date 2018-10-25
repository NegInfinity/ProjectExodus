#include "JsonImportPrivatePCH.h"

#include "MaterialTools.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"

using namespace MaterialTools;

UMaterialExpressionConstant* MaterialTools::createConstantExpression(UMaterial *material, float value, const TCHAR* constantName){
	auto matConstant = NewObject<UMaterialExpressionConstant>(material);
	if (constantName){
		matConstant->SetParameterName(constantName);
		matConstant->Desc = constantName;
	}
	material->Expressions.Add(matConstant);
	matConstant->R = value;
	return matConstant;
}

UMaterialExpression* MaterialTools::createMaterialSingleInput(UMaterial *unrealMaterial, float value, FExpressionInput &matInput, const TCHAR* inputName){
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

UMaterialExpressionVectorParameter* MaterialTools::createVectorExpression(UMaterial *material, FLinearColor color, const TCHAR* inputName){
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

UMaterialExpressionTextureSample* MaterialTools::createTextureExpression(UMaterial *material, UTexture * unrealTex, const TCHAR* inputName, bool normalMap){
	UMaterialExpressionTextureSample *result = 0;
	UE_LOG(JsonLog, Log, TEXT("Creating texture sample expression"));

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

UMaterialExpression* MaterialTools::createMaterialInputMultiply(UMaterial *material, UTexture *texture, 
		const FLinearColor *matColor, FExpressionInput &matInput, 
		const TCHAR* texParamName, const TCHAR* vecParamName,
		UMaterialExpressionTextureSample ** outTexNode,
		UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating multiply material input"));
	UMaterialExpressionTextureSample *texExp = 0;
	UMaterialExpressionVectorParameter *vecExp = 0;
	bool hasTex = texture != 0;//matTextureId >= 0;
	if (hasTex){
		texExp = createTextureExpression(material, texture, texParamName);
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

UMaterialExpression* MaterialTools::createMaterialInput(UMaterial *unrealMaterial, UTexture *texture, const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, 
		const TCHAR* paramName, UMaterialExpressionTextureSample **outTexNode, UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating material input"));
	//logValue(TEXT("texId: "), matTextureId);

	if (texture){
		UE_LOG(JsonLog, Log, TEXT("Texture found"));
		auto texExpression = NewObject<UMaterialExpressionTextureSample>(unrealMaterial);
		unrealMaterial->Expressions.Add(texExpression);
		matInput.Expression = texExpression;
		texExpression->Texture = texture;
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

