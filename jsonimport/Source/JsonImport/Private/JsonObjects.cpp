#include "JsonImportPrivatePCH.h"
#include "JsonObjects.h"
#include "JsonLog.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

using namespace JsonObjects;

void JsonObjects::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *warnName);
	}
}

void JsonObjects::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *name);
	}
}

void JsonObjects::logValue(const FString &msg, const IntArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: IntArray[%d]{...}"), *msg, arr.Num());
}
	
void JsonObjects::logValue(const FString &msg, const FloatArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: FloatArray[%d]{...}"), *msg, arr.Num());
}

void JsonObjects::logValue(const FString &msg, const bool val){
	FString str = val ? TEXT("true"): TEXT("false");
	//const char *str = val ? "true": "false";
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *str);
}

FVector JsonObjects::unityToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

void JsonObjects::logValue(const FString &msg, const FVector2D &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f"), *msg, val.X, val.Y);
}

void JsonObjects::logValue(const FString &msg, const FVector &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f"), *msg, val.X, val.Y, val.Z);
}

void JsonObjects::logValue(const FString &msg, const FQuat &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f, w %f"), *msg, val.X, val.Y, val.Z, val.W);
}

void JsonObjects::logValue(const FString &msg, const FMatrix &val){
	UE_LOG(JsonLog, Log, TEXT("%s:"), *msg);
	for(int i = 0; i < 4; i++)
		UE_LOG(JsonLog, Log, TEXT("    %f %f %f %f"), val.M[i][0], val.M[i][1], val.M[i][2], val.M[i][3]);
}

void JsonObjects::logValue(const FString &msg, const int val){
	UE_LOG(JsonLog, Log, TEXT("%s: %d"), *msg, val);
}

void JsonObjects::logValue(const FString &msg, const float val){
	UE_LOG(JsonLog, Log, TEXT("%s: %f"), *msg, val);
}

void JsonObjects::logValue(const FString &msg, const FString &val){
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *val);
}

void JsonObjects::logValue(const FString &msg, const FLinearColor &val){
	UE_LOG(JsonLog, Log, TEXT("%s: r %f, g %f, b %f, a %f"), *msg, val.R, val.G, val.B, val.A);
}

int32 JsonObjects::getInt(JsonObjPtr data, const char* name){
	return data->GetIntegerField(FString(name));
}

bool JsonObjects::getBool(JsonObjPtr data, const char* name){
	return data->GetBoolField(FString(name));
}

float JsonObjects::getFloat(JsonObjPtr data, const char* name){
	return data->GetNumberField(FString(name));
}

FString JsonObjects::getString(JsonObjPtr data, const char* name){
	return data->GetStringField(FString(name));
}

JsonObjPtr JsonObjects::getObject(JsonObjPtr data, const char* name){
	if (!data || !data->HasField(name))
		return 0;
	auto result = data->GetObjectField(FString(name));
	return result;
	//if (!result || !result->IsvV
}

FLinearColor JsonObjects::getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
	FLinearColor result = defaultVal;

	auto colorObj = getObject(data, name);
	if (!colorObj.IsValid())
			return result;
	result.A = getFloat(colorObj, "a");
	result.R = getFloat(colorObj, "r");
	result.G = getFloat(colorObj, "g");
	result.B = getFloat(colorObj, "b");

	return result;
}

FMatrix JsonObjects::getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal){
	FMatrix result = defaultVal;
	auto matObj = getObject(data, name);
	if (!matObj.IsValid())
			return result;

	result.M[0][0] = getFloat(matObj, "e00");
	result.M[0][1] = getFloat(matObj, "e10");
	result.M[0][2] = getFloat(matObj, "e20");
	result.M[0][3] = getFloat(matObj, "e30");
	result.M[1][0] = getFloat(matObj, "e01");
	result.M[1][1] = getFloat(matObj, "e11");
	result.M[1][2] = getFloat(matObj, "e21");
	result.M[1][3] = getFloat(matObj, "e31");
	result.M[2][0] = getFloat(matObj, "e02");
	result.M[2][1] = getFloat(matObj, "e12");
	result.M[2][2] = getFloat(matObj, "e22");
	result.M[2][3] = getFloat(matObj, "e32");
	result.M[3][0] = getFloat(matObj, "e03");
	result.M[3][1] = getFloat(matObj, "e13");
	result.M[3][2] = getFloat(matObj, "e23");
	result.M[3][3] = getFloat(matObj, "e33");
		
	return result;
}

FVector4 JsonObjects::getVector4(JsonObjPtr data, const char* name, const FVector4 &defaultVal){
	FVector4 result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");
	result.W = getFloat(vecObj, "W");

	return result;
}

FVector2D JsonObjects::getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal){
	FVector2D result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");

	return result;
}

FVector JsonObjects::getVector(JsonObjPtr data, const char* name, const FVector &defaultVal){
	FVector result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");

	return result;
}

FLinearColor JsonObjects::getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
	FLinearColor result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.R = getFloat(vecObj, "r");
	result.G = getFloat(vecObj, "g");
	result.B = getFloat(vecObj, "b");
	result.A = getFloat(vecObj, "a");

	return result;
}

FQuat JsonObjects::getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal){
	FQuat result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");
	result.W = getFloat(vecObj, "W");

	return result;
}

void JsonReflectionProbe::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, backgroundColor, getColor);
	JSON_GET_PARAM(jsonData, blendDistance, getFloat);
	JSON_GET_PARAM(jsonData, boxProjection, getBool);
	JSON_GET_PARAM(jsonData, center, getVector);
	JSON_GET_PARAM(jsonData, size, getVector);
	JSON_GET_PARAM(jsonData, clearType, getString);
	JSON_GET_PARAM(jsonData, cullingMask, getInt);
	JSON_GET_PARAM(jsonData, hdr, getBool);
	JSON_GET_PARAM(jsonData, intensity, getFloat);
	JSON_GET_PARAM(jsonData, nearClipPlane, getFloat);
	JSON_GET_PARAM(jsonData, farClipPlane, getFloat);
	JSON_GET_PARAM(jsonData, resolution, getInt);
	JSON_GET_PARAM(jsonData, mode, getString);
	JSON_GET_PARAM(jsonData, refreshMode, getString);
}

JsonReflectionProbe::JsonReflectionProbe(JsonObjPtr jsonData){
	load(jsonData);
}

JsonGameObject::JsonGameObject(JsonObjPtr jsonData){
	load(jsonData);
}

void JsonLight::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, range, getFloat);
	JSON_GET_PARAM(jsonData, spotAngle, getFloat);
	JSON_GET_PARAM2(jsonData, lightType, type, getString)
	JSON_GET_PARAM(jsonData, shadowStrength, getFloat);
	JSON_GET_PARAM(jsonData, intensity, getFloat);
	JSON_GET_PARAM(jsonData, bounceIntensity, getFloat);
	JSON_GET_PARAM(jsonData, color, getColor);
	JSON_GET_PARAM(jsonData, renderMode, getString);
	JSON_GET_PARAM(jsonData, shadows, getString);
	castsShadows = shadows != "Off";
}

JsonLight::JsonLight(JsonObjPtr jsonData){
	load(jsonData);
}

IntArray JsonObjects::getIntArray(JsonObjPtr jsonObj, const char *name){
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toIntArray(*arrValues);
	return IntArray();
}

void JsonRenderer::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, lightmapIndex, getInt);
	JSON_GET_PARAM(jsonData, shadowCastingMode, getString);
	JSON_GET_PARAM(jsonData, receiveShadows, getBool);

	JSON_GET_PARAM(jsonData, materials, getIntArray);
}

void JsonGameObject::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, name, getString);
	JSON_GET_PARAM(jsonData, id, getInt);
	JSON_GET_PARAM(jsonData, instanceId, getInt);
	JSON_GET_PARAM(jsonData, localPosition, getVector);
	JSON_GET_PARAM(jsonData, localRotation, getQuat);
	JSON_GET_PARAM(jsonData, localScale, getVector);
	JSON_GET_PARAM(jsonData, worldMatrix, getMatrix);
	JSON_GET_PARAM(jsonData, localMatrix, getMatrix);
	JSON_GET_PARAM2(jsonData, parentId, parent, getInt);
	JSON_GET_PARAM2(jsonData, meshId, mesh, getInt);

	JSON_GET_PARAM(jsonData, isStatic, getBool);
	JSON_GET_PARAM(jsonData, lightMapStatic, getBool);
	JSON_GET_PARAM(jsonData, navigationStatic, getBool);
	JSON_GET_PARAM(jsonData, occluderStatic, getBool);
	JSON_GET_PARAM(jsonData, occludeeStatic, getBool);

	JSON_GET_PARAM(jsonData, nameClash, getBool);
	JSON_GET_PARAM(jsonData, uniqueName, getString);

	JSON_GET_PARAM(jsonData, prefabRootId, getInt);
	JSON_GET_PARAM(jsonData, prefabObjectId, getInt);
	JSON_GET_PARAM(jsonData, prefabInstance, getBool);
	JSON_GET_PARAM(jsonData, prefabModelInstance, getBool);
	JSON_GET_PARAM(jsonData, prefabType, getString);

	renderers.Empty();
	lights.Empty();
	probes.Empty();

	getJsonObjArray(jsonData, lights, "light");
	getJsonObjArray(jsonData, renderers, "renderer");
	getJsonObjArray(jsonData, probes, "reflectionProbes");

	if (nameClash && (uniqueName.Len() > 0)){
		UE_LOG(JsonLog, Warning, TEXT("Name clash detected on object %d: %s. Renaming to %s"), 
			id, *name, *uniqueName);		
		ueName = uniqueName;
	}
	else
		ueName = name;

	ueWorldMatrix = unityWorldToUe(worldMatrix);
}

FMatrix JsonObjects::unityWorldToUe(const FMatrix &unityWorld){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = unityWorld.GetOrigin();
	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);
	FMatrix ueMatrix;
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);
	return ueMatrix;
}

IntArray JsonGameObject::getFirstMaterials() const{
	for(int i = 0; i < renderers.Num(); i++){
		const auto &curRend = renderers[i];
		if (curRend.hasMaterials())
			return curRend.materials;
	}
	UE_LOG(JsonLog, Warning, TEXT("Materials not found on renderer, returning blank array"));
	return IntArray();
}

void JsonPrefabData::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, name, getString);
	JSON_GET_PARAM(jsonData, path, getString);
	JSON_GET_PARAM(jsonData, guid, getString);
	JSON_GET_PARAM(jsonData, prefabType, getString);

	objects.Empty();
	getJsonObjArray(jsonData, objects, "objects");
	//objects
}

JsonPrefabData::JsonPrefabData(JsonObjPtr jsonData){
	load(jsonData);
}

/*
void ImportWorkData::addRootActor(AActor* actor){
	if (!storeActors)
		return;
	rootActors.Add(actor);
}
*/

void ImportWorkData::registerActor(AActor* actor, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootActors.Add(actor);
	else
		childActors.Add(actor);
	allActors.Add(actor);
}

void JsonBounds::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, center, getVector);
	JSON_GET_PARAM(jsonData, size, getVector);
}

JsonBounds::JsonBounds(JsonObjPtr jsonData){
	load(jsonData);
}

JsonTerrain::JsonTerrain(JsonObjPtr data){
	load(data);
}

void JsonTerrain::load(JsonObjPtr data){
	JSON_GET_PARAM(data, castShadows, getBool);
	JSON_GET_PARAM(data, detailObjectDensity, getFloat);
	JSON_GET_PARAM(data, detailObjectDistance, getFloat);
	JSON_GET_PARAM(data, drawHeightmap, getBool);
	JSON_GET_PARAM(data, drawTreesAndFoliage, getBool);
		
	JSON_GET_PARAM(data, renderHeightmap, getBool);
	JSON_GET_PARAM(data, renderTrees, getBool);
	JSON_GET_PARAM(data, renderDetails, getBool);
		
	JSON_GET_PARAM(data, heightmapPixelError, getFloat);
		
	JSON_GET_PARAM(data, legacyShininess, getFloat);
	//Color legacySpecular;
	JSON_GET_PARAM(data, legacySpecular, getColor);
	JSON_GET_PARAM(data, lightmapIndex, getInt);
	JSON_GET_PARAM(data, lightmapScaleOffet, getVector4);

	JSON_GET_PARAM(data, materialTemplateIndex, getInt);
	JSON_GET_PARAM(data, materialType, getString);
		
	JSON_GET_PARAM(data, patchBoundsMultiplier, getVector);
	JSON_GET_PARAM(data, preserveTreePrototypeLayers,  getBool);
	JSON_GET_PARAM(data, realtimeLightmapIndex, getInt);
	JSON_GET_PARAM(data, realtimeLightmapScaleOffset, getVector4);
		
	JSON_GET_PARAM(data, terrainDataId, getInt);
		
	JSON_GET_PARAM(data, treeBillboardDistance, getFloat);
	JSON_GET_PARAM(data, treeCrossFadeLength, getFloat);
		
	JSON_GET_PARAM(data, treeDistance, getFloat);
	JSON_GET_PARAM(data, treeLodBiasMultiplier, getFloat);
		
	JSON_GET_PARAM(data, treeMaximumFullLODCount, getInt);
}

JsonTerrainDetailPrototype::JsonTerrainDetailPrototype(JsonObjPtr data){
	load(data);
}

void JsonTerrainDetailPrototype::load(JsonObjPtr data){
}

void JsonObjects::getJsonValue(FLinearColor& outValue, JsonObjPtr data, const char*name){
	outValue = getLinearColor(data, name);
}

void JsonObjects::getJsonValue(FMatrix& outValue, JsonObjPtr data, const char *name){
	outValue = getMatrix(data, name);
}

void JsonObjects::getJsonValue(FVector2D& outValue, JsonObjPtr data, const char *name){
	outValue = getVector2(data, name);
}

void JsonObjects::getJsonValue(FVector& outValue, JsonObjPtr data, const char *name){
	outValue = getVector(data, name);
}

void JsonObjects::getJsonValue(FVector4& outValue, JsonObjPtr data, const char *name){
	outValue = getVector4(data, name);
}

void JsonObjects::getJsonValue(FQuat& outValue, JsonObjPtr data, const char *name){
	outValue = getQuat(data, name);
}

void JsonObjects::getJsonValue(int& outValue, JsonObjPtr data, const char* name){
	outValue = getInt(data, name);
}

void JsonObjects::getJsonValue(bool& outValue, JsonObjPtr data, const char* name){
	outValue = getBool(data, name);
}

void JsonObjects::getJsonValue(float &outValue, JsonObjPtr data, const char* name){
	outValue = getFloat(data, name);
}

void JsonObjects::getJsonValue(FString &outValue, JsonObjPtr data, const char* name){
	outValue = getString(data, name);
}
