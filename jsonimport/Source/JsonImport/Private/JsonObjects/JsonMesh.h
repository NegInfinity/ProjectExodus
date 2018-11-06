#pragma once

#include "JsonTypes.h"

class JsonSubMesh{
public:
	IntArray triangles;
	JsonSubMesh() = default;
	void load(JsonObjPtr data);
	JsonSubMesh(JsonObjPtr data){
		load(data);
	}
};

class JsonMesh{
public:
	int id = -1;
	FString name;
	FString path;
	IntArray materials;
	bool readable;
	int32 vertexCount;
	LinearColorArray colors;
	FloatArray verts;
	FloatArray normals;
	FloatArray uv0;
	FloatArray uv1;
	FloatArray uv2;
	FloatArray uv3;

	int32 subMeshCount = 0;
	TArray<JsonSubMesh> subMeshes;

	JsonMesh() = default;
	void load(JsonObjPtr data);
	JsonMesh(JsonObjPtr data){
		load(data);
	}
};
