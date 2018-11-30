#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"
#include <functional>

#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"

class UStaticMesh;
class USkeletalMesh;
class UMaterial;
class UMaterialInterface;
class JsonImporter;

class MeshBuilder{
public:
	void setupStaticMesh(UStaticMesh *mesh, const JsonMesh &jsonMesh, std::function<void(TArray<FStaticMaterial> &meshMaterials)> materialSetup);
	void setupSkeleton(USkeleton *skeleton, const JsonSkeleton &jsonSkel);
	void setupSkeletalMesh(USkeletalMesh *mesh, const JsonMesh &jsonMesh, const JsonImporter *importer, std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup);
	void generateBillboardMesh(UStaticMesh *staticMesh, UMaterialInterface *billboardMaterial);
	MeshBuilder() = default;

protected:
	static void processTangent(int originalIndex, const FloatArray &normFloats, const FloatArray &tangentFloats, bool hasNormals, bool hasTangents,
		std::function<void(const FVector&)> normCallback, //Receives normal
		std::function<void(const FVector&, const FVector&)> tanCallback //Receives U and V tangents. U, V. In this order.
	);
};