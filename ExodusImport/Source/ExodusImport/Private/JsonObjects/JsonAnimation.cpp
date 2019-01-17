#include "JsonImportPrivatePCH.h"
#include "JsonAnimation.h"
#include"macros.h"
#include "UnrealUtilities.h"

using namespace JsonObjects;
using namespace UnrealUtilities;

//#define JSON_ENABLE_VALUE_LOGGING

/*
Alright. 

Using JSON_GET_VAR in those classes creates MASSIVE memory allocations. Gigabytes of RAM.
I think it goes into Unreal log. 

So, I'm disabling those.
*/
void JsonTransform::load(JsonObjPtr data){
	JSON_GET_VAR(data, x);
	JSON_GET_VAR(data, y);
	JSON_GET_VAR(data, z);
	JSON_GET_VAR(data, pos);
}

void JsonTransformKey::load(JsonObjPtr data){
	JSON_GET_VAR_NOLOG(data, time);
	JSON_GET_VAR_NOLOG(data, frame);

	getJsonObj(data, local, "local");
	getJsonObj(data, world, "world");
}

void JsonAnimationMatrixCurve::load(JsonObjPtr data){
	JSON_GET_VAR(data, objectName);
	JSON_GET_VAR(data, objectPath);

	getJsonObjArray(data, keys, "keys");
}

void JsonKeyframe::load(JsonObjPtr data){
	JSON_GET_VAR(data, time);
	JSON_GET_VAR(data, value);
	JSON_GET_VAR(data, weightedMode);
	JSON_GET_VAR(data, inTangent);
	JSON_GET_VAR(data, inWeight);
	JSON_GET_VAR(data, outTangent);
	JSON_GET_VAR(data, outWeight);
}

void JsonAnimationCurve::load(JsonObjPtr data){
	JSON_GET_VAR(data, length);
	JSON_GET_VAR(data, preWrapMode);
	JSON_GET_VAR(data, postWrapMode);

	getJsonObjArray(data, keys, "keys");
}

void JsonAnimationEvent::load(JsonObjPtr data){
	JSON_GET_VAR(data, time);
	JSON_GET_VAR(data, floatParameter);
	JSON_GET_VAR(data, intParameter);
	JSON_GET_VAR(data, stringParameter);
	JSON_GET_VAR(data, objectReferenceParameter);
	JSON_GET_VAR(data, isFiredByAnimator);
	JSON_GET_VAR(data, isFiredByLegacy);
}

void JsonAnimatorControllerParameter::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, animType);
	JSON_GET_VAR(data, defaultBool);
	JSON_GET_VAR(data, defaultFloat);
}

void JsonAnimatorController::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, id);

	getJsonObjArray(data, parameters, "parameters");
	animationIds = getIntArray(data, "animationIds");
}

void JsonAnimator::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, skeletonId);

	skinMeshIds = getIntArray(data, "skinMeshIds");

	JSON_GET_VAR(data, animatorControllerId);
	JSON_GET_VAR(data, applyRootMotion);
	JSON_GET_VAR(data, cullingMode);
	JSON_GET_VAR(data, hasRootMotion);
	JSON_GET_VAR(data, hasTransformHierarchy);
	JSON_GET_VAR(data, humanScale);
	JSON_GET_VAR(data, isHuman);
	JSON_GET_VAR(data, layerCount);
	JSON_GET_VAR(data, layersAffectMassCenter);
	JSON_GET_VAR(data, linearVelocityBlending);
	JSON_GET_VAR(data, speed);

	getJsonObjArray(data, humanBones, "humanBones");
}

void JsonEditorCurveBinding::load(JsonObjPtr data){
	JSON_GET_VAR(data, propertyName);
	JSON_GET_VAR(data, isDiscreteCurve);
	JSON_GET_VAR(data, isPPtrCurve);
	JSON_GET_VAR(data, path);

	getJsonObjArray(data, curves, "curves");
}

void JsonAnimationClip::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, frameRate);
	JSON_GET_VAR(data, empty);
	JSON_GET_VAR(data, isLooping);
	JSON_GET_VAR(data, legacy);
	JSON_GET_VAR(data, length);

	getJsonObj(data, localBounds, "localBounds");
	JSON_GET_VAR(data, wrapMode);

	getJsonObjArray(data, animEvents, "animEvents");
	getJsonObjArray(data, objBindings, "objBindings");
	getJsonObjArray(data, floatBindings, "floatBindings");
	getJsonObjArray(data, matrixCurves, "matrixCurves");
}

void JsonHumanBone::load(JsonObjPtr data){
	JSON_GET_VAR(data, boneName);
	JSON_GET_VAR(data, objectName);
	JSON_GET_VAR(data, objectPath);
	JSON_GET_VAR(data, objInstanceId);
	JSON_GET_VAR(data, transfInstanceId);
}

/*
void JsonEditorCurveBinding::load(JsonObjPtr data){
	JSON_GET_VAR(data, propertyName);
	JSON_GET_VAR(data, isDiscreteCurve);
	JSON_GET_VAR(data, isPPtrCurve);
	JSON_GET_VAR(data, path);

	getJsonObjArray(data, curves, "curves");
}
*/

FMatrix JsonTransform::getUnityTransform() const{
	auto result = FMatrix::Identity;
	auto x1 = x;
	auto y1 = y;
	auto z1 = z;
	auto pos1 = pos;
	result.SetAxes(&x1, &y1, &z1, &pos1); //Why aren't those pointers const?
	return result;
}

FMatrix JsonTransform::getUnrealTransform() const{
	return unityWorldToUe(getUnityTransform());
}
