#pragma once

#include "JsonTypes.h"

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
#include "Materials/MaterialExpressionStaticSwitch.h"
#include "Materials/MaterialExpressionStaticBool.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"

#include "Materials/MaterialExpressionConstant2Vector.h"
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

#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerSwitch.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"

namespace MaterialTools{
	UMaterialExpression* createMaterialInput(UMaterial *material, UTexture *texture, 
		const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, const TCHAR* paramName, UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);

	UMaterialExpression* createMaterialInputMultiply(UMaterial *material, UTexture *texture, 
		const FLinearColor *matColor, FExpressionInput &matInput, 
		const TCHAR* texParamName, const TCHAR* vecParamName,
		UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);

	UMaterialExpression* createMaterialSingleInput(UMaterial *material, float value, FExpressionInput &matInput, const TCHAR* inputName);
	UMaterialExpressionTextureSample *createTextureExpression(UMaterial *material, UTexture *texture, const TCHAR* inputName, bool normalMap = false);
	UMaterialExpressionVectorParameter *createVectorParameterExpression(UMaterial *material, FLinearColor color, const TCHAR* inputName);

	UMaterialExpressionScalarParameter *createScalarParameterExpression(UMaterial *material, float val, const TCHAR* inputName);

	UMaterialExpressionVectorParameter *createVectorParameterExpression(UMaterial *material, const FVector4 &vec, const TCHAR* inputName);
	UMaterialExpressionVectorParameter *createVectorParameterExpression(UMaterial *material, const FVector &vec, const TCHAR* inputName);
	UMaterialExpressionVectorParameter *createVectorParameterExpression(UMaterial *material, const FVector2D &vec, const TCHAR* inputName);

	UMaterialExpressionConstant2Vector *createConstVec2Expression(UMaterial *material, const FVector2D &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant3Vector *createConstVec3Expression(UMaterial *material, const FVector &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant4Vector *createConstVec4Expression(UMaterial *material, const FVector4 &vec, const TCHAR* inputName = nullptr);

	UMaterialExpressionConstant2Vector *createConstVec2Expression(UMaterial *material, float x, float y, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant3Vector *createConstVec3Expression(UMaterial *material, float x, float y, float z, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant4Vector *createConstVec4Expression(UMaterial *material, float x, float y, float z, float w, const TCHAR* inputName = nullptr);

	UMaterialExpressionConstant* createConstantExpression(UMaterial *material, float value, const TCHAR* constantName);
	UMaterialExpressionComponentMask* createComponentMask(UMaterial *material, bool r, bool g, bool b, bool a, const TCHAR* name = 0);
	UMaterialExpressionComponentMask* createComponentMask(UMaterial *material, UMaterialExpression* src, bool r, bool g, bool b, bool a, const TCHAR* name = 0);


	UMaterialExpressionAdd* createAddExpression(UMaterial *material, UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR *name = 0);
	UMaterialExpressionMultiply* createMulExpression(UMaterial *material, UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR *name = 0);

	//normalize(Vec3(n1.xy + n2.xy, n1.z*n2.z));
	UMaterialExpression* makeNormalBlend(UMaterial* material, UMaterialExpression *norm1, UMaterialExpression *norm2);
	//normal.xy *= bumpScale;
	//normal.z = sqrt(1.0 - saturate(dot(normal.xy, normal.xy)));
	UMaterialExpression* makeNormalMapScaler(UMaterial* material, UMaterialExpression *normalTex, UMaterialExpression* scaleFactor);
	UMaterialExpressionAppendVector* createAppendVectorExpression(UMaterial* material, UMaterialExpression *argA = 0, UMaterialExpression *argB = 0, const TCHAR* name = 0);

	//UMaterialExpressionLandscapeLayerBlend* createLayerExpression(UMaterial* material, 

	void arrangeMaterialNodesAsTree(UMaterial* material);
	void arrangeMaterialNodesAsGrid(UMaterial* materia);

	template<typename Exp> Exp* createExpression(UMaterial *material, const TCHAR* name = 0){
		Exp* result = NewObject<Exp>(material);
		material->Expressions.Add(result);
		if (name){
			result->Desc = FString(name);
		}
		return result;
	}

	template<typename T> auto makeFactoryRootGuard(){
		auto factory = NewObject<T>();
		factory ->AddToRoot();
		return TSharedPtr<T>(
			factory, 
			[](T* p){
				if (p) 
					p->RemoveFromRoot();
			}
		);
	}
}
