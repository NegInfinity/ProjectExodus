#pragma once

#include "JsonTypes.h"
#include "MaterialBuilder/MaterialFingerprint.h"

class MaterialBuildData{
public:
	JsonMaterialId matId = -1;
};

class JsonImporter;

class MaterialBuilder{
public:
	UMaterial *importMaterial(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId);
	UMaterial *importMaterial(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId);
	MaterialBuilder() = default;
protected:
	void buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, JsonImporter *importer, MaterialBuildData &buildData);
};

