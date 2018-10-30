#include "JsonImportPrivatePCH.h"

#include "MaterialTools.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionComponentMask.h"

using namespace MaterialTools;

//normal.xy *= bumpScale;
//normal.z = sqrt(1.0 - saturate(dot(normal.xy, normal.xy)));
UMaterialExpression* MaterialTools::makeNormalMapScaler(UMaterial* material, UMaterialExpression *normalTex, UMaterialExpression* scaleFactor){
	auto normSource = normalTex;
	auto normMask = createExpression<UMaterialExpressionComponentMask>(material);
	normMask->Input.Expression = normSource;
	normMask->R = normMask->G = true;
	normMask->B = normMask->A = false;

	auto mulXy = createExpression<UMaterialExpressionMultiply>(material);
	//auto dotExp = createExpression<UMaterialExpressionDot

	mulXy->A.Expression = normMask;
	mulXy->B.Expression = scaleFactor;

	auto dotExp = createExpression<UMaterialExpressionDotProduct>(material);
	dotExp->A.Expression = mulXy;
	dotExp->B.Expression = mulXy;

	/*
	auto satExp = createExpression<UMaterialExpressionSaturate>(material);
	satExp->Input.Expression = dotExp; //This doesn't work?
	*/
	auto satExp = createExpression<UMaterialExpressionClamp>(material);
	satExp->Input.Expression = dotExp;
	satExp->MinDefault = 0.0f;
	satExp->MaxDefault = 1.0f;

	auto oneMinus = createExpression<UMaterialExpressionOneMinus>(material);
	oneMinus->Input.Expression = satExp;

	auto sqrtExp = createExpression<UMaterialExpressionSquareRoot>(material);
	sqrtExp->Input.Expression = oneMinus;

	auto finAppend = createExpression<UMaterialExpressionAppendVector>(material);
	finAppend->A.Expression = mulXy;
	finAppend->B.Expression = sqrtExp;

	return finAppend;
}

//normalize(Vec3(n1.xy + n2.xy, n1.z*n2.z));
UMaterialExpression* MaterialTools::makeNormalBlend(UMaterial* material, UMaterialExpression *norm1, UMaterialExpression *norm2){
	auto n1xy = createComponentMask(material, norm1, true, true, false, false);
	auto n2xy = createComponentMask(material, norm2, true, true, false, false);
	auto n1z = createComponentMask(material, norm1, false, false, true, false);
	auto n2z = createComponentMask(material, norm2, false, false, true, false);

	auto xyAdd = createExpression<UMaterialExpressionAdd>(material);
	auto zMul = createExpression<UMaterialExpressionMultiply>(material);
	xyAdd->A.Expression = n1xy;
	xyAdd->B.Expression = n2xy;

	zMul->A.Expression = n1z;
	zMul->B.Expression = n2z;

	auto combine = createExpression<UMaterialExpressionAppendVector>(material);
	combine->A.Expression = xyAdd;
	combine->B.Expression = zMul;

	auto norm = createExpression<UMaterialExpressionNormalize>(material);
	norm->VectorInput.Expression = combine;

	return norm;
}

UMaterialExpressionAdd* MaterialTools::createAddExpression(UMaterial *material, UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR *name){
	auto result = createExpression<UMaterialExpressionAdd>(material, name);
	if (arg1){
		result->A.Expression= arg1;
	}
	if (arg2){
		result->B.Expression = arg2;
	}
	return result;
}

UMaterialExpressionMultiply* MaterialTools::createMulExpression(UMaterial *material, UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR *name){
	auto result = createExpression<UMaterialExpressionMultiply>(material, name);
	if (arg1){
		result->A.Expression= arg1;
	}
	if (arg2){
		result->B.Expression = arg2;
	}
	return result;
}

UMaterialExpressionComponentMask* MaterialTools::createComponentMask(UMaterial *material, UMaterialExpression* src, bool r, bool g, bool b, bool a, const TCHAR* name){
	auto result = createComponentMask(material, r, g, b, a, name);
	if (src)
		result->Input.Expression = src;
	return result;
}

UMaterialExpressionComponentMask* MaterialTools::createComponentMask(UMaterial *material, bool r, bool g, bool b, bool a, const TCHAR* name){
	auto result = createExpression<UMaterialExpressionComponentMask>(material, name);
	result->R = r;// ? 1: 0;
	result->G = g;// ? 1: 0;
	result->B = b;// ? 1: 0;
	result->A = a;// ? 1: 0;
	return result;
}


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

UMaterialExpressionVectorParameter* MaterialTools::createVectorParameterExpression(UMaterial *material, FLinearColor color, const TCHAR* inputName){
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

UMaterialExpressionVectorParameter* MaterialTools::createVectorParameterExpression(UMaterial *material, const FVector4 &vec, const TCHAR* inputName){
	UMaterialExpressionVectorParameter* vecExpression = 
		NewObject<UMaterialExpressionVectorParameter>(material);
	material->Expressions.Add(vecExpression);

	//matInput.Expression = vecExpression;
	vecExpression->DefaultValue.R = vec.X;
	vecExpression->DefaultValue.G = vec.Y;
	vecExpression->DefaultValue.B = vec.Z;
	vecExpression->DefaultValue.A = vec.W;
	if (inputName){
		vecExpression->SetParameterName(inputName);
		vecExpression->Desc = inputName;
	}
	return vecExpression;
}

UMaterialExpressionScalarParameter* MaterialTools::createScalarParameterExpression(UMaterial *material, float val, const TCHAR* inputName){
	auto result = createExpression<UMaterialExpressionScalarParameter>(material, inputName);

	result->DefaultValue = val;

	if (inputName){
		result->ParameterName = inputName;
	}
	return result;
}

UMaterialExpressionVectorParameter* MaterialTools::createVectorParameterExpression(UMaterial *material, const FVector &vec, const TCHAR* inputName){
	return createVectorParameterExpression(material, FVector4(vec, 1.0f), inputName);
}

UMaterialExpressionVectorParameter* MaterialTools::createVectorParameterExpression(UMaterial *material, const FVector2D &vec, const TCHAR* inputName){
	return createVectorParameterExpression(material, FVector4(vec.X, vec.Y, 0.0f, 1.0f), inputName);
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
		vecExp = createVectorParameterExpression(material, *matColor, vecParamName);
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

UMaterialExpressionAppendVector* MaterialTools::createAppendVectorExpression(UMaterial* material, UMaterialExpression *argA, UMaterialExpression *argB, const TCHAR* name){
	auto* result = createExpression<UMaterialExpressionAppendVector>(material, name);
	if (argA){
		result->A.Expression = argA;
	}
	if (argB){
		result->B.Expression = argB;
	}
	return result;
}

