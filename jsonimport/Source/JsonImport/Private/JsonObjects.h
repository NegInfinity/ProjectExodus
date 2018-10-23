#pragma once

#include "JsonTypes.h"
#include "JsonLog.h"

#define JSON_GET_AUTO_PARAM2(obj, varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_AUTO_PARAM(obj, name, op) auto name = op(obj, #name); logValue(#name, name);

#define JSON_GET_PARAM2(obj, varName, paramName, op) varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_PARAM(obj, name, op) name = op(obj, #name); logValue(#name, name);

class AActor;
using IdActorMap = TMap<int, AActor*>;
using IdSet = TSet<int>;

/*
This one exists mostly to deal with the fact htat IDs are unique within SCENE, 
and within each scene they start from zero.

I kinda wonder if I should work towards ensureing ids being globally unique, but then again...
not much point when I can just use scoped dictionaries.
*/
class ImportWorkData{
public:
	IdNameMap objectFolderPaths;
	IdActorMap objectActors;
	UWorld *world;
	bool editorMode;
	bool storeActors;

	TArray<AActor*> rootActors;
	TArray<AActor*> childActors;
	TArray<AActor*> allActors;

	void registerActor(AActor* actor, AActor *parent);
	//void addRootActor(AActor* actor);

	ImportWorkData(UWorld *world_, bool editorMode_, bool storeActors_ = false)
	:world(world_), editorMode(editorMode_), storeActors(storeActors_){
	}

	void clear(){
		objectFolderPaths.Empty();
		objectActors.Empty();
	}
};

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

class JsonPrefabData{
public:
	FString name;
	FString path;
	FString guid;
	FString prefabType;

	TArray<JsonGameObject> objects;

	void load(JsonObjPtr jsonData);
	JsonPrefabData() = default;
	JsonPrefabData(JsonObjPtr jsonData);
};

class JsonBounds{
public:
	FVector center;
	FVector size;
	void load(JsonObjPtr jsonData);
	JsonBounds() = default;
	JsonBounds(JsonObjPtr jsonData);
};

class JsonTerrain{
public:
	bool castShadows;
	float detailObjectDensity;
	float detailObjectDistance;
	bool drawHeightmap;
	bool drawTreesAndFoliage;
		
	bool renderHeightmap;
	bool renderTrees;
	bool renderDetails;
		
	float heightmapPixelError;
		
	float legacyShininess;
	//Color legacySpecular;
	FLinearColor legacySpecular;
	int lightmapIndex;
	FVector4 lightmapScaleOffet;

	int materialTemplateIndex = -1;
	FString materialType;		
		
	FVector patchBoundsMultiplier;
	bool preserveTreePrototypeLayers;
	int realtimeLightmapIndex;
	FVector4 realtimeLightmapScaleOffset;
		
	int terrainDataId = -1;
		
	float treeBillboardDistance;
	float treeCrossFadeLength;
		
	float treeDistance;
	float treeLodBiasMultiplier;
		
	int treeMaximumFullLODCount;

	JsonTerrain() = default;
	JsonTerrain(JsonObjPtr data);
	void load(JsonObjPtr data);
};

class JsonTerrainDetailPrototype{
public:
	int textureId;
	int detailPrefabId;
	int detailPrefabObjectId;

	float bendFactor;
	FLinearColor dryColor;
	FLinearColor healthyColor;
	float minWidth;
	float minHeight;
	float maxWidth;
	float maxHeight;
	float noiseSpread;
	FString renderMode;
	bool usePrototypeMesh;

	JsonTerrainDetailPrototype() = default;
	JsonTerrainDetailPrototype(JsonObjPtr data);
	void load(JsonObjPtr data);
};



namespace JsonObjects{
	void logValue(const FString &msg, const IntArray &arr);
	void logValue(const FString &msg, const FloatArray &arr);
	void logValue(const FString &msg, const bool val);
	void logValue(const FString &msg, const FVector2D &val);
	void logValue(const FString &msg, const FVector &val);
	void logValue(const FString &msg, const FQuat &val);
	void logValue(const FString &msg, const FMatrix &val);
	void logValue(const FString &msg, const int val);
	void logValue(const FString &msg, const float val);
	void logValue(const FString &msg, const FString &val);
	void logValue(const FString &msg, const FLinearColor &val);

	template <typename T>T* createActor(UWorld *world, FTransform transform, bool editorMode, const TCHAR* logName = 0){
		T* result = 0;
		if (editorMode){
			result = 	Cast<T>(GEditor->AddActor(world->GetCurrentLevel(),
				T::StaticClass(), transform));
		}
		else{
			result = world->SpawnActor<T>();
		}
		if (!result){
			if (logName){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn actor %s"), logName);
			}
			else{
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn templated actor"));
			}
		}
		else{
			auto moveResult = result->SetActorTransform(transform, false, nullptr, ETeleportType::ResetPhysics);
			logValue("Actor move result: ", moveResult);
		}
		return result;
	}

	template <typename T>T* createActor(ImportWorkData& workData, FTransform transform, const TCHAR* logName = 0){
		return createActor<T>(workData.world, transform, workData.editorMode, logName);
	}

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

	FVector unityToUe(const FVector& arg);
	FMatrix unityWorldToUe(const FMatrix &unityMatrix);
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
	
	void getJsonValue(FLinearColor& outValue, JsonObjPtr data, const char*name);
	void getJsonValue(FMatrix& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector2D& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector4& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FQuat& outValue, JsonObjPtr data, const char *name);

	void getJsonValue(int& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(bool& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(float &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(FString &outValue, JsonObjPtr data, const char* name);
	//JsonObjPtr getObject(JsonObjPtr data, const char* name);

	FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity);
	FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D());
	FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector());
	FVector4 getVector4(JsonObjPtr data, const char* name, const FVector4 &defaultVal = FVector4());
	FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat());
}
