#pragma once

#include "JsonTypes.h"
#include "JsonObjects/JsonBounds.h"

class JsonTransform{
public:
	FVector x;
	FVector y;
	FVector z;
	FVector pos;

	FMatrix getUnityTransform() const;
	FMatrix getUnrealTransform() const;
	void load(JsonObjPtr data);
	JsonTransform() = default;
	JsonTransform(JsonObjPtr data){
		load(data);
	}
};

class JsonTransformKey{
public:
	float time;
	int frame;
	JsonTransform local;
	JsonTransform world;

	void load(JsonObjPtr data);
	JsonTransformKey() = default;
	JsonTransformKey(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimationMatrixCurve{
public:
	FString objectName;
	FString objectPath;
	TArray<JsonTransformKey> keys;

	void load(JsonObjPtr data);
	JsonAnimationMatrixCurve() = default;
	JsonAnimationMatrixCurve(JsonObjPtr data){
		load(data);
	}
};

class JsonKeyframe{
public:
	float time;
	float value;
	int weightedMode;
	float inTangent;
	float inWeight;
	float outTangent;
	float outWeight;

	void load(JsonObjPtr data);
	JsonKeyframe() = default;
	JsonKeyframe(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimationCurve{
public:
	int length = 0;
	FString preWrapMode;
	FString postWrapMode;
	TArray<JsonKeyframe> keys;

	void load(JsonObjPtr data);
	JsonAnimationCurve() = default;
	JsonAnimationCurve(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimationEvent{
public:
	float time;
	float floatParameter;
	int intParameter;
	FString stringParameter;
	FString objectReferenceParameter;
	bool isFiredByAnimator;
	bool isFiredByLegacy;

	JsonAnimationEvent() = default;
	void load(JsonObjPtr data);
	JsonAnimationEvent(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimatorControllerParameter{
public:
	FString name;
	FString animType;
	bool defaultBool;
	int defaultInt;
	float defaultFloat;

	void load(JsonObjPtr data);
	JsonAnimatorControllerParameter() = default;
	JsonAnimatorControllerParameter(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimatorController{
public:
	FString name;
	FString path;
	int id = -1;
	TArray<JsonAnimatorControllerParameter> parameters;
	IntArray animationIds;

	void load(JsonObjPtr data);
	JsonAnimatorController() = default;
	JsonAnimatorController(JsonObjPtr data){
		load(data);
	}
};

class JsonHumanBone{
public:
	FString boneName;
	FString objectName;
	FString objectPath;
	int objInstanceId = -1;
	int transfInstanceId = -1;

	JsonHumanBone() = default;
	void load(JsonObjPtr data);
	JsonHumanBone(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimator{
public:
	FString name;
	int skeletonId = -1;
	IntArray skinMeshIds;
	int animatorControllerId = -1;

	bool applyRootMotion;
	FString cullingMode;
	bool hasRootMotion;
	bool hasTransformHierarchy;
	float humanScale;
	bool isHuman;
	int layerCount;
	bool layersAffectMassCenter;
	bool linearVelocityBlending;
	float speed;
	bool stabilizeFeet;

	TArray<JsonHumanBone> humanBones;

	void load(JsonObjPtr data);
	JsonAnimator() = default;
	JsonAnimator(JsonObjPtr data){
		load(data);
	}
};

class JsonEditorCurveBinding{
public:
	FString propertyName;
	bool isDiscreteCurve;
	bool isPPtrCurve;
	FString path;
	TArray<JsonAnimationCurve> curves;
	//TArrau>KspmAmo,atopmCirve? cirves; Well, this is new
	void load(JsonObjPtr data);
	JsonEditorCurveBinding() = default;
	JsonEditorCurveBinding(JsonObjPtr data){
		load(data);
	}
};

class JsonAnimationClip{
public:
	FString name;
	int id  = -1;

	float frameRate;
	bool empty;
	bool humanMotion;
	bool isLooping;
	bool legacy;
	float length;
	JsonBounds localBounds;
	FString wrapMode;

	TArray<JsonAnimationEvent> animEvents;

	TArray<JsonEditorCurveBinding> objBindings;
	TArray<JsonEditorCurveBinding> floatBindings;

	TArray<JsonAnimationMatrixCurve> matrixCurves;

	void load(JsonObjPtr data);
	JsonAnimationClip() = default;
	JsonAnimationClip(JsonObjPtr data){
		load(data);
	}
};