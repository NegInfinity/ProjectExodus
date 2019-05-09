#include "JsonImportPrivatePCH.h"
#include "JsonMesh.h"
#include "macros.h"
#include "loggers.h"

//#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonSubMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, triangles);
}

void JsonMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, uniqueName);

	JSON_GET_VAR(data, convexCollider);
	JSON_GET_VAR(data, triangleCollider);

	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, materials);
	JSON_GET_VAR(data, readable);
	JSON_GET_VAR(data, vertexCount);
	colors = getByteArray(data, "colors", true);
	logValue(TEXT("colors: "), colors);
	
	verts = getFloatArray(data, "verts", false);
	logValue(TEXT("verts: "), verts);
	normals = getFloatArray(data, "normals", true);
	logValue(TEXT("normals: "), normals);

	tangents = getFloatArray(data, "tangents", true);
	logValue(TEXT("tangents: "), tangents);
	uv0 = getFloatArray(data, "uv0", true);
	logValue(TEXT("uv0: "), uv0);
	uv1 = getFloatArray(data, "uv1", true);
	logValue(TEXT("uv1: "), uv1);
	uv2 = getFloatArray(data, "uv2", true);
	logValue(TEXT("uv2: "), uv2);
	uv3 = getFloatArray(data, "uv3", true);
	logValue(TEXT("uv3: "), uv3);
	uv4 = getFloatArray(data, "uv4", true);
	logValue(TEXT("uv4: "), uv4);
	uv5 = getFloatArray(data, "uv5", true);
	logValue(TEXT("uv5: "), uv5);
	uv6 = getFloatArray(data, "uv6", true);
	logValue(TEXT("uv6: "), uv6);
	uv7 = getFloatArray(data, "uv7", true);
	logValue(TEXT("uv7: "), uv7);

	boneWeights = getFloatArray(data, "boneWeights", true);
	logValue(TEXT("boneWeights: "), boneWeights);

	boneIndexes = getIntArray(data, "boneIndexes", true);
	logValue(TEXT("boneIndexes: "), boneIndexes);

	JSON_GET_VAR(data, defaultSkeletonId);
	defaultBoneNames = getStringArray(data, "defaultBoneNames", true);
	logValue(TEXT("defaultBoneNames: "), boneIndexes);

	JSON_GET_VAR_NOLOG(data, defaultMeshNodeName);
	defaultMeshNodeMatrix = FMatrix::Identity;
	JSON_GET_VAR_NOLOG(data, defaultMeshNodeMatrix);

	JSON_GET_VAR(data, blendShapeCount);
	getJsonObjArray(data, blendShapes, "blendShapes", blendShapeCount == 0);

	bool needsSkinWeights = (boneWeights.Num() != 0)||(boneIndexes.Num() != 0);
	bindPoses = getMatrixArray(data, "bindPoses", !needsSkinWeights);
	inverseBindPoses = getMatrixArray(data, "inverseBindPoses", !needsSkinWeights);

	JSON_GET_VAR(data, subMeshCount);
	getJsonObjArray(data, subMeshes, "subMeshes");
}

void JsonBlendShapeFrame::load(JsonObjPtr data){
	JSON_GET_VAR(data, index);
	JSON_GET_VAR(data, weight);
	JSON_GET_VAR(data, deltaVerts);
	JSON_GET_VAR(data, deltaTangents);
	JSON_GET_VAR(data, deltaNormals);
}

void JsonBlendShape::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, index);
	JSON_GET_VAR(data, numFrames);

	getJsonObjArray(data, frames, "frames");
}

FString JsonMesh::makeUnrealMeshName() const{
	auto pathBaseName = FPaths::GetBaseFilename(path);
	FString result;
	FString baseName = uniqueName.IsEmpty() ? name: uniqueName;

	if (!pathBaseName.IsEmpty()){
		//Well, I've managed to create a level with two meshes named "cube". So...
		result = FString::Printf(TEXT("%s_%s_%d"), *pathBaseName, *baseName, id.toIndex());
	}
	else{
		result = FString::Printf(TEXT("%s_%d"), *baseName, id.toIndex());
	}
	return result;
}

const FVector JsonMesh::getVertex(int index) const{
	return getIdxVector3(verts, index);
}

const FVector JsonMesh::getNormal(int index) const{
	if (!normals.Num())
		return FVector::ZeroVector;
	return getIdxVector3(normals, index);
}

FVector JsonBlendShapeFrame::getDeltaVert(int vertIdx) const{
	return getIdxVector3(deltaVerts, vertIdx);
}

FVector JsonBlendShapeFrame::getDeltaTangent(int vertIdx) const{
	return getIdxVector3(deltaTangents, vertIdx);
}

FVector JsonBlendShapeFrame::getDeltaNormal(int vertIdx) const{
	return getIdxVector3(deltaNormals, vertIdx);
}
