#pragma once

#include "JsonTypes.h"

class JsonBlendShapeFrame{
public:
	int index;
	float weight;
	FloatArray deltaVerts;
	FloatArray deltaTangents;
	FloatArray deltaNormals;

	void load(JsonObjPtr data);
	JsonBlendShapeFrame(JsonObjPtr data){
		load(data);
	}
	JsonBlendShapeFrame() = default;
};

class JsonBlendShape{
public:
	FString name;
	int index=  -1;
	int numFrames;
	TArray<JsonBlendShapeFrame> frames;

	void load(JsonObjPtr data);
	JsonBlendShape(JsonObjPtr data){
		load(data);
	}
	JsonBlendShape() = default;
};

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
	//LinearColorArray colors;
	ByteArray colors;
	FloatArray verts;
	FloatArray normals;
	FloatArray uv0;
	FloatArray uv1;
	FloatArray uv2;
	FloatArray uv3;
	FloatArray uv4;
	FloatArray uv5;
	FloatArray uv6;
	FloatArray uv7;

	FloatArray boneWeights;
	IntArray boneIndexes;
	int blendShapeCount = 0;
	TArray<JsonBlendShape> blendShapes;
	TArray<FMatrix> bindPoses;
	TArray<FMatrix> inverseBindPoses;

	int32 subMeshCount = 0;
	TArray<JsonSubMesh> subMeshes;

	JsonMesh() = default;
	void load(JsonObjPtr data);
	JsonMesh(JsonObjPtr data){
		load(data);
	}
};
