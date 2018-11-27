#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"
#include <functional>

class UStaticMesh;
class USkeletalMesh;
class UMaterial;
class UMaterialInterface;

class MeshBuilder{
public:
	void setupMesh(UStaticMesh *mesh, const JsonMesh &jsonMesh, std::function<void(TArray<FStaticMaterial> &meshMaterials)> materialSetup);
	void setupMesh(USkeletalMesh *mesh, const JsonMesh &jsonMesh, std::function<void(TArray<FStaticMaterial> &meshMaterials)> materialSetup);
	void generateBillboardMesh(UStaticMesh *staticMesh, UMaterialInterface *billboardMaterial);
	MeshBuilder() = default;
};