#pragma once

#include "JsonTypes.h"

class JsonBlendShapeFrame{
public:
	int index;
	float weight;
	FloatArray deltaVerts;
	FloatArray deltaTangents;
	FloatArray deltaNormals;

	FVector getDeltaVert(int index) const;
	FVector getDeltaTangent(int index) const;
	FVector getDeltaNormal(int index) const;

	void load(JsonObjPtr data);
	JsonBlendShapeFrame(JsonObjPtr data){
		load(data);
	}
	JsonBlendShapeFrame() = default;
};

class JsonBlendShape{
public:
	FString name;
	int index = -1;
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
	ResId id;
	//int id = -1;
	FString name;
	FString uniqueName;
	bool convexCollider = false;
	bool triangleCollider = false;
	FString path;
	IntArray materials;
	bool readable;
	int32 vertexCount;
	//LinearColorArray colors;
	ByteArray colors;
	FloatArray verts;
	FloatArray normals;
	FloatArray tangents;//well... looks like directly importing tangetns might be less hassle, after all.

	//I wonder if I should've made this an array
	FloatArray uv0;
	FloatArray uv1;
	FloatArray uv2;
	FloatArray uv3;
	FloatArray uv4;
	FloatArray uv5;
	FloatArray uv6;
	FloatArray uv7;

	bool hasColors() const{
		return (colors.Num() > 0);
	}

	bool hasNormals() const{
		return (normals.Num() > 0);
	}

	bool hasTangents() const{
		return (tangents.Num() > 0);
	}

	const FVector getVertex(int index) const;
	const FVector getNormal(int index) const;

	int getNumTexCoords() const{
		const int maxNumCoords = 8;
		const FloatArray* coords[maxNumCoords] = {
			&uv0, &uv1, &uv2, &uv3, &uv4, &uv5, &uv6, &uv7
		};
		for(int i = 0; i < maxNumCoords; i++){
			if (coords[i]->Num() == 0)
				return i;
		}
		return maxNumCoords;
	}

	FloatArray boneWeights;
	IntArray boneIndexes;
	int defaultSkeletonId = -1;
	StringArray defaultBoneNames;

	bool hasBones() const{
		return boneIndexes.Num() > 0;
	}

	FString defaultMeshNodeName;
	FMatrix defaultMeshNodeMatrix;//relative to skeleton root

	int blendShapeCount = 0;
	TArray<JsonBlendShape> blendShapes;
	TArray<FMatrix> bindPoses;
	TArray<FMatrix> inverseBindPoses;

	int32 subMeshCount = 0;
	TArray<JsonSubMesh> subMeshes;

	FString makeUnrealMeshName() const;

	bool hasBoneWeights() const{
		return (boneWeights.Num() > 0) || (boneIndexes.Num() > 0);
	}

	bool hasBlendShapes() const{
		return blendShapes.Num() > 0;
	}

	JsonMesh() = default;
	void load(JsonObjPtr data);
	JsonMesh(JsonObjPtr data){
		load(data);
	}
};
