#pragma once

#include "JsonTypes.h"

#define JSON_GET_AUTO_PARAM2(obj, varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_AUTO_PARAM(obj, name, op) auto name = op(obj, #name); logValue(#name, name);

#define JSON_GET_PARAM2(obj, varName, paramName, op) varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_PARAM(obj, name, op) name = op(obj, #name); logValue(#name, name);

class JsonReflectionProbe{
public:
	FLinearColor backgroundColor;
	float blendDistance;
	bool boxProjection;
	FVector center;
	FVector size;
	FString clearType;
	int32 cullingMask;
	bool hdr;
	float intensity;
	float nearClipPlane;
	float farClipPlane;
	int32 resolution;
	FString mode;
	FString refreshMode;

	void load(JsonObjPtr jsonData);
	JsonReflectionProbe() = default;
	JsonReflectionProbe(JsonObjPtr jsonData);
};

class JsonLight{
public:
	float range;
	float spotAngle;
	FString lightType;
	float shadowStrength;
	float intensity;
	float bounceIntensity;
	FLinearColor color;
	FString renderMode;
	FString shadows;
	bool castsShadows;

	void load(JsonObjPtr jsonData);
	JsonLight() = default;
	JsonLight(JsonObjPtr jsonData);
};

class JsonRenderer{
public:
	int lightmapIndex = -1;
	FString shadowCastingMode;
	//FVector4 lightmapScaleOffset;
	TArray<int32> materials;
	bool receiveShadows;

	bool hasMaterials() const{
		return materials.Num() > 0;
	}

	void load(JsonObjPtr jsonData);
	JsonRenderer() = default;
	JsonRenderer(JsonObjPtr jsonData){
		load(jsonData);
	}
};

class JsonGameObject{
public:
	FString name;
	int32 id;
	int32 instanceId;
	FVector localPosition;
	FQuat localRotation;
	FVector localScale;
	FMatrix worldMatrix;
	FMatrix localMatrix;
	int32 parentId;
	int32 meshId;
	bool isStatic;
	bool lightMapStatic;
	bool navigationStatic;
	bool occluderStatic;
	bool occludeeStatic;

	bool nameClash;
	FString uniqueName;

	TArray<JsonLight> lights;
	TArray<JsonReflectionProbe> probes;
	TArray<JsonRenderer> renderers;

	IntArray getFirstMaterials() const;

	bool hasMesh() const{
		return meshId >= 0;
	}

	bool hasLights() const{
		return lights.Num() > 0;
	}

	bool hasProbes() const{
		return probes.Num() > 0;
	}

	bool hasRenderers() const{
		return renderers.Num() > 0;
	}

	FMatrix ueWorldMatrix;
	FString ueName;

	void load(JsonObjPtr jsonData);
	JsonGameObject() = default;
	JsonGameObject(JsonObjPtr jsonData);
};


namespace JsonObjects{
	template<typename T> TArray<T> getJsonObjArray(JsonObjPtr jsonData, const char* name){
		TArray<T> result;
		getJsonObjArray<T>(jsonData, result, name);
		return result;
	}

	template<typename T> void getJsonObjArray(JsonObjPtr jsonData, TArray<T>& result, const char* name){
		auto arrayVal = jsonData->GetArrayField(name);

		if (arrayVal.Num() <= 0)
			return;
		
		for (int i = 0; i < arrayVal.Num(); i++){
			auto jsonVal = arrayVal[i];
			auto jsonObj = jsonVal->AsObject();
			if (!jsonObj.IsValid()){	
				UE_LOG(JsonLog, Warning, TEXT("Could not retrieve index %d from \"%s\""), i, name);
				continue;
			}
			result.Add(T(jsonObj));
		}
	}

	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName);
	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name);

	void logValue(const FString &msg, const IntArray &arr);
	void logValue(const FString &msg, const FloatArray &arr);
	void logValue(const FString &msg, const bool val);
	FVector unityToUe(const FVector& arg);
	FMatrix unityWorldToUe(const FMatrix &unityMatrix);
	void logValue(const FString &msg, const FVector2D &val);
	void logValue(const FString &msg, const FVector &val);
	void logValue(const FString &msg, const FQuat &val);
	void logValue(const FString &msg, const FMatrix &val);
	void logValue(const FString &msg, const int val);
	void logValue(const FString &msg, const float val);
	void logValue(const FString &msg, const FString &val);
	void logValue(const FString &msg, const FLinearColor &val);
	int32 getInt(JsonObjPtr data, const char* name);
	bool getBool(JsonObjPtr data, const char* name);
	float getFloat(JsonObjPtr data, const char* name);
	FString getString(JsonObjPtr data, const char* name);
	JsonObjPtr getObject(JsonObjPtr data, const char* name);

	IntArray getIntArray(JsonObjPtr, const char *name);

	TArray<float> toFloatArray(const JsonValPtrs &inData){
		TArray<float> result;
		for(auto cur: inData){
			double val = 0.0;
			if (cur.IsValid()){
				cur->TryGetNumber(val);
			}
				//val = cur->AsNumber();
			result.Add(val);
		}
		return result;
	}
	
	TArray<float> toFloatArray(const JsonValPtrs* inData){
		TArray<float> result;
		if (inData){
			for(auto cur: *inData){
				double val = 0.0;
				if (cur.IsValid()){
					cur->TryGetNumber(val);
				}
					//val = cur->AsNumber();
				result.Add(val);
			}
		}
		return result;
	}
	
	TArray<int32> toIntArray(const JsonValPtrs &inData){
		TArray<int32> result;
		for(auto cur: inData){
			int32 val;
			if (cur.IsValid()){
				cur->TryGetNumber(val);
			}
			result.Add(val);
		}
		return result;
	}
	
	FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity);
	FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D());
	FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector());
	FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat());
}
