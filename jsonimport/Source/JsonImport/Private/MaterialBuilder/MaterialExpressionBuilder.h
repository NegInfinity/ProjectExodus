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
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionSquareRoot.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionTransform.h"
#include "Materials/MaterialExpressionTransformPosition.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionVertexInterpolator.h"
#include "Materials/MaterialExpressionNoise.h"

#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerSwitch.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"

class MaterialExpressionBuilder{
protected:
	UMaterial* material = nullptr;
public:
	MaterialExpressionBuilder() = default;
	MaterialExpressionBuilder(UMaterial *mat){
		begin(mat);
	}
	void begin(UMaterial *mat);
	void end();

	UMaterialExpressionVectorParameter* vecParam(const FVector2D &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionVectorParameter* vecParam(const FVector &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionVectorParameter* vecParam(const FVector4 &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionVectorParameter* vecParam(float x, float y, const TCHAR* inputName = nullptr);
	UMaterialExpressionVectorParameter* vecParam(float x, float y, float z, const TCHAR* inputName = nullptr);
	UMaterialExpressionVectorParameter* vecParam(float x, float y, float z, float w, const TCHAR* inputName = nullptr);

	UMaterialExpressionConstant2Vector* constVec2(const FVector2D &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant2Vector* constVec2(float x, float y, const TCHAR* inputName = nullptr);

	UMaterialExpressionConstant3Vector* constVec3(const FVector &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant3Vector* constVec3(float x, float y, float z, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant3Vector* constVec3(const FLinearColor &c, const TCHAR* inputName = nullptr);

	UMaterialExpressionConstant4Vector* constVec4(const FVector4 &vec, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant4Vector* constVec4(float x, float y, float z, float w, const TCHAR* inputName = nullptr);
	UMaterialExpressionConstant4Vector* constVec4(const FLinearColor &c, const TCHAR* inputName = nullptr);

	UMaterialExpressionComponentMask* mask(UMaterialExpression *src, bool r, bool g, bool b, bool a, const TCHAR* name = nullptr);
	UMaterialExpressionConstant* constScalar(float value, const TCHAR* name = nullptr);

	UMaterialExpressionAdd* add(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name = nullptr);
	UMaterialExpressionMultiply* mul(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name = nullptr);
	UMaterialExpressionSubtract* sub(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name = nullptr);

	UMaterialExpressionOneMinus* oneMinus(UMaterialExpression *arg, const TCHAR* name = nullptr);
	UMaterialExpressionAppendVector* append(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* name = nullptr);
	UMaterialExpressionTextureSample* texSample(UTexture *tex, UMaterialExpression *uvs, bool normalMap = false, const TCHAR* name = nullptr);

	UMaterialExpressionDotProduct* dot(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption = nullptr);
	UMaterialExpressionWorldPosition* worldPos(bool cameraSpace = false, bool offsets = false, const TCHAR* caption = nullptr);

	UMaterialExpressionTransform* transform(UMaterialExpression *src, 
		EMaterialVectorCoordTransformSource srcSpace, EMaterialVectorCoordTransform dstSpace, const TCHAR* caption = nullptr);
	UMaterialExpressionTransformPosition* transformPos(UMaterialExpression *src, 
		EMaterialPositionTransformSource srcSpace, EMaterialPositionTransformSource dstSpace, const TCHAR* caption = nullptr);

	UMaterialExpressionSquareRoot* dot(UMaterialExpression *arg, const TCHAR* caption = nullptr);
	//UMaterialExpressionSaturate* saturate(UMaterialExpression *arg, const TCHAR* caption = nullptr);
	UMaterialExpressionClamp* clamp(UMaterialExpression *arg, UMaterialExpression *argMin, UMaterialExpression *argMax, const TCHAR* caption);
	UMaterialExpressionMax* max(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption = nullptr);

	UMaterialExpressionSquareRoot* squareRoot(UMaterialExpression *arg, const TCHAR *caption = nullptr);
	UMaterialExpressionSquareRoot* sqrt(UMaterialExpression *arg, const TCHAR *caption = nullptr);
	UMaterialExpression* distance(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption = nullptr);
	//UMaterialExpressionDistance* distance(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption = nullptr);

	//UMaterialExpressionVertexInterpolator *vertexInterpolator(UMaterialExpression *arg, EMaterialValueType interpolationType = MCT_Float);

	UMaterialExpressionNoise* noise(UMaterialExpression *cords, ENoiseFunction noiseFunc = NOISEFUNCTION_GradientTex, bool tiled = false, bool turbulence = false);

#define EXPRBUILD_INLINE_OPS2(op, Type)  Type* op(UMaterialExpression *arg1, UMaterialExpression* arg2, const TCHAR* name = nullptr) \
		{return op(arg1, 0.0f, arg2, 0.0f, name);}  \
	Type* op(UMaterialExpression *arg1, float arg2, const TCHAR* name = nullptr) {return op(arg1, 0.0f, nullptr, arg2, name);}  \
	Type* op(float arg1, UMaterialExpression* arg2, const TCHAR* name = nullptr) {return op(nullptr, arg1, arg2, 0.0f, name);}  \
	Type* op(float arg1, float arg2, const TCHAR* name = nullptr) \
		{return op(nullptr, arg1, nullptr, arg2, name);}  

	EXPRBUILD_INLINE_OPS2(add, UMaterialExpressionAdd)
	EXPRBUILD_INLINE_OPS2(sub, UMaterialExpressionSubtract)
	EXPRBUILD_INLINE_OPS2(mul, UMaterialExpressionMultiply)

	template<typename Exp> Exp* expr(const TCHAR* name = nullptr){
		checkf(material != nullptr, TEXT("Expression builder used without material. Call begin() first"));
		Exp* result = NewObject<Exp>(material);
		material->Expressions.Add(result);
		if (name){
			result->Desc = FString(name);
		}
		return result;
	}
};

