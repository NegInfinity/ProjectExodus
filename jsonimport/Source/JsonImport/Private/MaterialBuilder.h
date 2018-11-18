#pragma once

#include "JsonTypes.h"
#include "MaterialBuilder/MaterialFingerprint.h"
#include "JsonObjects/JsonGameObject.h"
#include "JsonObjects/JsonTerrainData.h"
#include "JsonObjects/JsonTerrain.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include <functional>

class JsonImporter;
class UMaterialExpression;

DECLARE_LOG_CATEGORY_EXTERN(JsonLogMatNodeSort, Log, All);

class MaterialBuildData{
public:
	JsonMaterialId matId = -1;
	JsonImporter *importer = 0;

	UMaterialExpression *mainUv = nullptr;
	UMaterialExpression *detailUv = nullptr;

	UMaterialExpression *albedoTexExpression = nullptr;
	UMaterialExpression *albedoColorExpression = nullptr;
	UMaterialExpression *albedoDetailTexExpression = nullptr;

	UMaterialExpression *normalTexExpression = nullptr;
	UMaterialExpression *normalExpression = nullptr;
	UMaterialExpression *detailNormalTexExpression = nullptr;
	UMaterialExpression *detailNormalExpression = nullptr;

	UMaterialExpression *detailMaskExpression = nullptr;

	UMaterialExpression *metallicTexExpression = nullptr;
	UMaterialExpression *specularTexExpression = nullptr;
	UMaterialExpression *specularColorExpression = nullptr;
	UMaterialExpression *smoothTexSource= nullptr;

	UMaterialExpression *albedoExpression = nullptr;
	UMaterialExpression *specularExpression = nullptr;
	UMaterialExpression *metallicExpression = nullptr;
	UMaterialExpression *emissiveExpression = nullptr;

	MaterialBuildData(JsonMaterialId matId_, JsonImporter *importer_)
	:matId(matId_), importer(importer_){
	}
};

class TerrainBuilder;
class JsonTerrainDetailPrototype;

class MaterialBuilder{
public:
	UMaterial* loadDefaultMaterial();

	UMaterial *importMasterMaterial(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId);
	UMaterial *importMasterMaterial(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId);

	UMaterialInstanceConstant* importMaterialInstance(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId);
	UMaterialInstanceConstant* importMaterialInstance(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId);

	using MaterialCallbackFunc = std::function<void(UMaterial*)>;

	UMaterial *createMaterial(const FString& name, const FString &path, JsonImporter *importer, 
		MaterialCallbackFunc newCallback = MaterialCallbackFunc(), MaterialCallbackFunc existingCallback = MaterialCallbackFunc(), 
		MaterialCallbackFunc postEditCallback = MaterialCallbackFunc());

	UMaterial *createTerrainMaterial(const TerrainBuilder *terrainBuilder,
		const FIntPoint &terrainVertSize, const FString &terrainDataPath);

	UMaterial *createBillboardMaterial(const JsonTerrainDetailPrototype * detailPrototype, int layerIndex, const TerrainBuilder *terrainBuilder, const FString &terrainDataPath);
	UMaterialInstanceConstant* createBillboardMatInstance(const JsonTerrainDetailPrototype * detailPrototype, int layerIndex, const TerrainBuilder *terrainBuilder, const FString &terrainDataPath);

	MaterialBuilder() = default;
protected:
	void setScalarParam(UMaterialInstanceConstant *matInst, const char *paramName, float val) const;	
	void setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FVector2D val) const;
	void setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FVector val) const;
	void setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FLinearColor val) const;
	void setTexParam(UMaterialInstanceConstant *matInst, const char *paramName, int32 texId, const JsonImporter *importer) const;
	void setTexParam(UMaterialInstanceConstant *matInst, const char *paramName, UTexture *tex) const;
	bool setStaticSwitch(FStaticParameterSet &paramSet, const char *switchName, bool newValue) const;
	bool setTexParams(UMaterialInstanceConstant *matInst,  FStaticParameterSet &paramSet, int32 texId, 
		const char *switchName, const char *texParamName, const JsonImporter *importer) const;

	void setupMaterialInstance(UMaterialInstanceConstant *matInst, const JsonMaterial &jsonMat, JsonImporter *importer, JsonMaterialId matId);
	void  setupBillboardMatInstance(UMaterialInstanceConstant *result, const JsonTerrainDetailPrototype *detailPrototype, 
		int layerIndex, const TerrainBuilder *terrainBuilder);

	void createBillboardTransformNodes(UMaterial *material);
	void fillBillboardMaterial(UMaterial* material, const JsonTerrainDetailPrototype * detailPrototype, int layerIndex, const TerrainBuilder *terrainBuilder);
	void buildTerrainMaterial(UMaterial *material, const TerrainBuilder *terrainBuilder, const FIntPoint &terrainVertSize, const FString &terrainDataPath);

	void buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void arrangeNodesGrid(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void arrangeNodesTree(UMaterial* material /*, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData*/);
	void processOpacity(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processMainUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processDetailUv(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);

	void processAlbedo(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processNormalMap(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processEmissive(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processOcclusion(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processMetallic(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processSpecular(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processDetailMask(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processRoughness(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
	void processParallax(UMaterial* material, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData);
};
