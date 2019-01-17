#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"
#include <functional>

class UStaticMesh;
class USkeletalMesh;
class UMaterial;
class UMaterialInterface;
class JsonImporter;

class MeshBuilder{
public:
	void setupStaticMesh(UStaticMesh *mesh, const JsonMesh &jsonMesh, std::function<void(TArray<FStaticMaterial> &meshMaterials)> materialSetup);
	void generateBillboardMesh(UStaticMesh *staticMesh, UMaterialInterface *billboardMaterial);
	MeshBuilder() = default;
protected:
};

