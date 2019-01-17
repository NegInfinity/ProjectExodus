#include "JsonImportPrivatePCH.h"
#include "MaterialExpressionBuilder.h"

void MaterialExpressionBuilder::begin(UMaterial *mat){
	checkf(mat, TEXT("Material cannot be null"));
	checkf(material == nullptr, TEXT("Recursive begin() call in expressio nbuilder"));
	material = mat;
}

void MaterialExpressionBuilder::end(){
	checkf(material, TEXT("Mismatched end() call, material is null"));
	material = nullptr;
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(const FVector2D &vec, const TCHAR* inputName){
	return vecParam(vec.X, vec.Y, 0.0f, 1.0f, inputName);
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(const FVector &vec, const TCHAR* inputName){
	return vecParam(vec.X, vec.Y, vec.Z, 1.0f, inputName);
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(const FVector4 &vec, const TCHAR* inputName){
	return vecParam(vec.X, vec.Y, vec.Z, vec.W, inputName);
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(float x, float y, const TCHAR* inputName){
	return vecParam(x, y, 0.0f, 1.0f, inputName);
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(float x, float y, float z, const TCHAR* inputName){
	return vecParam(x, y, z, 1.0f, inputName);
}

UMaterialExpressionVectorParameter* MaterialExpressionBuilder::vecParam(float x, float y, float z, float w, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionVectorParameter>(inputName);

	result->DefaultValue.R = x;
	result->DefaultValue.G = y;
	result->DefaultValue.B = z;
	result->DefaultValue.A = w;
	if (inputName){
		result->SetParameterName(inputName);
		result->Desc = inputName;
	}
	return result;
}

UMaterialExpressionConstant2Vector* MaterialExpressionBuilder::constVec2(const FVector2D &vec, const TCHAR* inputName){
	return constVec2(vec.X, vec.Y, inputName);
}

UMaterialExpressionConstant2Vector* MaterialExpressionBuilder::constVec2(float x, float y, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionConstant2Vector>(inputName);
	result->R = x;
	result->G = y;
	return result;
}

UMaterialExpressionConstant3Vector* MaterialExpressionBuilder::constVec3(const FVector &vec, const TCHAR* inputName){
	return constVec3(vec.X, vec.Y, vec.Z, inputName);
}

UMaterialExpressionConstant3Vector* MaterialExpressionBuilder::constVec3(float x, float y, float z, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionConstant3Vector>(inputName);
	result->Constant.R = x;
	result->Constant.G = y;
	result->Constant.B = z;
	result->Constant.A = 1.0f;
	return result;
}

UMaterialExpressionConstant3Vector* MaterialExpressionBuilder::constVec3(const FLinearColor &c, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionConstant3Vector>(inputName);
	result->Constant = c;
	return result;
}

UMaterialExpressionConstant4Vector* MaterialExpressionBuilder::constVec4(const FVector4 &vec, const TCHAR* inputName){
	return constVec4(vec.X, vec.Y, vec.Z, vec.W, inputName);
}

UMaterialExpressionConstant4Vector* MaterialExpressionBuilder::constVec4(float x, float y, float z, float w, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionConstant4Vector>(inputName);
	result->Constant.R = x;
	result->Constant.G = y;
	result->Constant.B = z;
	result->Constant.A = w;
	return result;
}

UMaterialExpressionConstant4Vector* MaterialExpressionBuilder::constVec4(const FLinearColor &c, const TCHAR* inputName){
	auto result = expr<UMaterialExpressionConstant4Vector>(inputName);
	result->Constant = c;
	return result;
}

UMaterialExpressionConstant* MaterialExpressionBuilder::constScalar(float value, const TCHAR* name){
	auto result = expr<UMaterialExpressionConstant>(name);
	result->R = value;
	return result;
}

UMaterialExpressionComponentMask* MaterialExpressionBuilder::mask(UMaterialExpression *src, bool r, bool g, bool b, bool a, const TCHAR* name){
	auto result = expr<UMaterialExpressionComponentMask>(name);
	result->Input.Expression = src;
	result->R = r;
	result->G = g;
	result->B = b;
	result->A = a;
	return result;
}

UMaterialExpressionAdd* MaterialExpressionBuilder::add(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name){
	auto result = expr<UMaterialExpressionAdd>(name);
	result->A.Expression = arg1;
	result->ConstA = arg1Const;
	result->B.Expression = arg2;
	result->ConstB = arg2Const;
	return result;
}

UMaterialExpressionMultiply* MaterialExpressionBuilder::mul(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name){
	auto result = expr<UMaterialExpressionMultiply>(name);
	result->A.Expression = arg1;
	result->ConstA = arg1Const;
	result->B.Expression = arg2;
	result->ConstB = arg2Const;
	return result;
}

UMaterialExpressionSubtract* MaterialExpressionBuilder::sub(UMaterialExpression *arg1, float arg1Const, UMaterialExpression* arg2, float arg2Const, const TCHAR* name){
	auto result = expr<UMaterialExpressionSubtract>(name);
	result->A.Expression = arg1;
	result->ConstA = arg1Const;
	result->B.Expression = arg2;
	result->ConstB = arg2Const;
	return result;
}

UMaterialExpressionOneMinus* MaterialExpressionBuilder::oneMinus(UMaterialExpression *arg, const TCHAR* name){
	auto result = expr<UMaterialExpressionOneMinus>(name);
	result->Input.Expression = arg;
	return result;
}

UMaterialExpressionAppendVector* MaterialExpressionBuilder::append(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* name){
	auto result = expr<UMaterialExpressionAppendVector>(name);
	result->A.Expression = arg1;
	result->B.Expression = arg2;
	return result;
}

UMaterialExpressionTextureSample* MaterialExpressionBuilder::texSample(UTexture *tex, UMaterialExpression *uvs, bool normalMap, const TCHAR* name){
	auto result = expr<UMaterialExpressionTextureSample>(name);

	result->SamplerType = normalMap ? SAMPLERTYPE_Normal: SAMPLERTYPE_Color;
	result->Texture = tex;

	if (name){
		result->SetParameterName(name);
		result->Desc = name;
	}

	return result;
}

UMaterialExpressionDotProduct* MaterialExpressionBuilder::dot(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption){
	auto result = expr<UMaterialExpressionDotProduct>(caption);
	result->A.Expression = arg1;
	result->B.Expression = arg2;
	return result;
}

UMaterialExpressionWorldPosition* MaterialExpressionBuilder::worldPos(bool cameraSpace, bool offsets, const TCHAR* caption){
	auto result = expr<UMaterialExpressionWorldPosition>(caption);
	result->WorldPositionShaderOffset = cameraSpace ? 
		(offsets ? WPT_CameraRelative: WPT_CameraRelativeNoOffsets)
		: (offsets ? WPT_Default: WPT_ExcludeAllShaderOffsets);
	return result;
}

UMaterialExpressionTransform* MaterialExpressionBuilder::transform(UMaterialExpression *src, 
		EMaterialVectorCoordTransformSource srcSpace, EMaterialVectorCoordTransform dstSpace, const TCHAR* caption){
	auto result = expr<UMaterialExpressionTransform>(caption);
	result->Input.Expression = src;
	result->TransformSourceType = srcSpace;
	result->TransformType = dstSpace;
	return result;
}

UMaterialExpressionTransformPosition* MaterialExpressionBuilder::transformPos(UMaterialExpression *src, 
		EMaterialPositionTransformSource srcSpace, EMaterialPositionTransformSource dstSpace, const TCHAR* caption){
	auto result = expr<UMaterialExpressionTransformPosition>(caption);
	result->Input.Expression = src;
	result->TransformSourceType = srcSpace;
	result->TransformType = dstSpace;
	return result;
}

UMaterialExpressionSquareRoot* MaterialExpressionBuilder::dot(UMaterialExpression *arg, const TCHAR* caption){
	auto result = expr<UMaterialExpressionSquareRoot>(caption);
	result->Input.Expression = arg;
	return result;
}

/*
UMaterialExpressionSaturate* MaterialExpressionBuilder::saturate(UMaterialExpression *arg, const TCHAR* caption){
	auto result = expr<UMaterialExpressionSaturate>(caption);
	result->Input.Expression = arg;
	return result;
}
*/

UMaterialExpressionClamp* MaterialExpressionBuilder::clamp(UMaterialExpression *arg, UMaterialExpression *argMin, UMaterialExpression *argMax, const TCHAR* caption){
	auto result = expr<UMaterialExpressionClamp>(caption);
	result->Input.Expression = arg;
	result->Min.Expression = argMin;
	result->Max.Expression = argMax;
	return result;
}

UMaterialExpressionMax* MaterialExpressionBuilder::max(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption){
	auto result = expr<UMaterialExpressionMax>(caption);
	result->A.Expression = arg1;
	result->B.Expression = arg2;
	return result;
}

UMaterialExpressionSquareRoot* MaterialExpressionBuilder::squareRoot(UMaterialExpression *arg, const TCHAR *caption){
	auto result = expr<UMaterialExpressionSquareRoot>(caption);
	result->Input.Expression = arg;
	return result;
}

UMaterialExpressionSquareRoot* MaterialExpressionBuilder::sqrt(UMaterialExpression *arg, const TCHAR *caption){
	return squareRoot(arg, caption);
}

UMaterialExpression* MaterialExpressionBuilder::distance(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption){
	auto diff = sub(arg1, arg2);
	auto diffDot = dot(diff, diff);
	auto l = sqrt(diffDot, caption);
	return l;
}

/*
UMaterialExpressionDistance* MaterialExpressionBuilder::distance(UMaterialExpression *arg1, UMaterialExpression *arg2, const TCHAR* caption){
	auto result = expr<UMaterialExpressionDistance>(caption);
	result->A.Expression = arg1;
	result->B.Expression = arg2;
	return result;
}
*/

/*
UMaterialExpressionVertexInterpolator* MaterialExpressionBuilder::vertexInterpolator(UMaterialExpression *arg, EMaterialValueType interpolationType){
	auto result = expr<UMaterialExpressionVertexInterpolator>();
	result->Input.Expression = arg;
	result->InterpolatedType = interpolationType;
	return result;
}
*/

UMaterialExpressionNoise* MaterialExpressionBuilder::noise(UMaterialExpression *cords, ENoiseFunction noiseFunc, bool tiled, bool turbulence){
	auto result = expr<UMaterialExpressionNoise>();
	result->NoiseFunction = noiseFunc;
	result->bTiling = tiled;
	result->bTurbulence = turbulence;
	return result;
}
