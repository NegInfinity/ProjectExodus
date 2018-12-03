#include "JsonImportPrivatePCH.h"
#include "JsonMesh.h"
#include "macros.h"
#include "loggers.h"

using namespace JsonObjects;

void JsonSubMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, triangles);
}

void JsonMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, materials);
	JSON_GET_VAR(data, readable);
	JSON_GET_VAR(data, vertexCount);
	//TArray<FLinearColor> colors;
	//JSON_GET_VAR(data, colors);
	colors = getByteArray(data, "colors", true);
	logValue(TEXT("colors: "), colors);
	
	//JSON_GET_VAR(data, verts);
	verts = getFloatArray(data, "verts", false);
	logValue(TEXT("verts: "), verts);
	//JSON_GET_VAR(data, normals);
	normals = getFloatArray(data, "normals", true);
	logValue(TEXT("normals: "), normals);

	tangents = getFloatArray(data, "tangents", true);
	logValue(TEXT("tangents: "), tangents);
	//JSON_GET_VAR(data, uv0);
	//JSON_GET_VAR(data, uv1);
	//JSON_GET_VAR(data, uv2);
	//JSON_GET_VAR(data, uv3);
	//JSON_GET_VAR(data, uv4);
	//JSON_GET_VAR(data, uv5);
	//JSON_GET_VAR(data, uv6);
	//JSON_GET_VAR(data, uv7);
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

	//JSON_GET_VAR(data, boneWeights);
	boneWeights = getFloatArray(data, "boneWeights", true);
	logValue(TEXT("boneWeights: "), boneWeights);
	//JSON_GET_VAR(data, boneIndexes);
	boneIndexes = getIntArray(data, "boneIndexes", true);
	logValue(TEXT("boneIndexes: "), boneIndexes);

	JSON_GET_VAR(data, defaultSkeletonId);
	defaultBoneNames = getStringArray(data, "defaultBoneNames", true);
	logValue(TEXT("defaultBoneNames: "), boneIndexes);

	JSON_GET_VAR(data, blendShapeCount);
	getJsonObjArray(data, blendShapes, "blendShapes", blendShapeCount == 0);

	bool needsSkinWeights = (boneWeights.Num() != 0)||(boneIndexes.Num() != 0);
	bindPoses = getMatrixArray(data, "bindPoses", !needsSkinWeights);
	inverseBindPoses = getMatrixArray(data, "inverseBindPoses", !needsSkinWeights);
	//TArray<FMatrix> bindPoses;
	//TArray<FMatrix> inverseBindPoses;

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
	if (!pathBaseName.IsEmpty()){
		//Well, I've managed to create a level with two meshes named "cube". So...
		result = FString::Printf(TEXT("%s_%s_%d"), *pathBaseName, *name, id);
	}
	else{
		result = FString::Printf(TEXT("%s_%d"), *name, id);
	}
	return result;
}
