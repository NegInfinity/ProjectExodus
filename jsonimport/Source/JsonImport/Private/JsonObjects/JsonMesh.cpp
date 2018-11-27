#include "JsonImportPrivatePCH.h"
#include "JsonMesh.h"
#include "macros.h"

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
	JSON_GET_VAR(data, colors);
	
	JSON_GET_VAR(data, verts);
	JSON_GET_VAR(data, normals);
	JSON_GET_VAR(data, uv0);
	JSON_GET_VAR(data, uv1);
	JSON_GET_VAR(data, uv2);
	JSON_GET_VAR(data, uv3);
	JSON_GET_VAR(data, uv4);
	JSON_GET_VAR(data, uv5);
	JSON_GET_VAR(data, uv6);
	JSON_GET_VAR(data, uv7);

	JSON_GET_VAR(data, boneWeights);
	JSON_GET_VAR(data, boneIndexes);
	JSON_GET_VAR(data, blendShapeCount);

	getJsonObjArray(data, blendShapes, "blendShapes");
	bindPoses = getMatrixArray(data, "bindPoses");
	inverseBindPoses = getMatrixArray(data, "inverseBindPoses");
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
