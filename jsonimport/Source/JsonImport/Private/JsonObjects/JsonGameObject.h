#pragma once
#include "JsonTypes.h"
#include "JsonLight.h"
#include "JsonRenderer.h"
#include "JsonReflectionProbe.h"

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

	int32 prefabRootId;
	int32 prefabObjectId;
	bool prefabInstance;
	bool prefabModelInstance;
	FString prefabType;

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

