#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"

#include "MaterialBuilder.h"
#include "JsonObjects/JsonTerrainData.h"

class UMaterialExpression;
class UMaterialExpressionParameter;
class UMaterialExpressionVectorParameter;
class UMaterialExpressionConstant;
class UMaterialExpressionTextureSample;
class UTexture;
class UMaterial;
class ALandscape;
class ULandscapeLayerInfoObject;
class ULandscapeGrassType;

class JsonImporter{
protected:
	FString assetRootPath;//TODO: rename to srcAssetRootPath. Points to json file folder.
	FString assetCommonPath;
	FString sourceBaseName;
	IdNameMap meshIdMap;
	IdNameMap texIdMap;
	IdNameMap matIdMap;
	IdNameMap matInstIdMap;

	TMap<JsonId, JsonTerrainData> terrainDataMap;

	//This data should be reset between scenes. Otherwise thingsb ecome bad.
	//IdNameMap objectFolderPaths;
	//IdActorMap objectActors;
	IdSet emissiveMaterials;
	MaterialBuilder materialBuilder;

	ALandscape* createDefaultLandscape(ImportWorkData &workData, const JsonGameObject &jsonGameObj);
	ALandscape* createDefaultLandscape(UWorld *world);

	bool saveSceneObjectsAsWorld(const JsonValPtrs *sceneObjects, const FString &sceneName, const FString &scenePath);
	void processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, int32 objId, AActor *parentActor, const FString &folderPath);
	void processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &light, AActor *parentActor, const FString& folderPath);
	void processLights(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath);
	void processMesh(ImportWorkData &workData, const JsonGameObject &gameObj, int objId, AActor *parentActor, const FString& folderPath);

	/*
	void processFoliageActors(ImportWorkData &workData, ALandscape *landscape, const JsonGameObject &jsonGameObj, 
		const JsonTerrain &jsonTerrain, const JsonTerrainData &terrData, 
		AActor *parentActor, const FString& folderPath);*/
	void processTerrain(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonTerrain &jsonTerrain, AActor *parentActor, const FString& folderPath);
	void processTerrains(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath);

	ULandscapeLayerInfoObject* createTerrainLayerInfo(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		const JsonTerrainData &terrData,int layerIndex, bool detailLayer, const FString &terrainDataPath);

	//UMaterial *createTerrainMaterial(ImportWorkData &workData, const JsonGameObject &jsonGameObj, const JsonTerrainData &terrData, const JsonConvertedTerrain &convTerrain);
	UWorld *createWorldForScene(const FString &sceneName, const FString &scenePath);
	bool saveLoadedWorld(UWorld *world, const FString &sceneName, const FString &sceneAssetPath);
	void importScene(JsonObjPtr sceneData, bool createWorld);

	void importPrefabs(const JsonValPtrs *prefabs);
	void importPrefab(const JsonPrefabData& prefab);

	void importTerrainData(JsonObjPtr jsonData, JsonId terrainId, const FString &rootPath);
	void loadTerrains(const JsonValPtrs* terrains);

	ULandscapeGrassType* createGrassType(ImportWorkData &workData, const JsonGameObject &jsonGameObj, const JsonTerrainData &terrainData, 
		int layerIndex, const FString &terrainDataPth);
public:
	void registerMaterialInstancePath(int32 id, FString path);
	void registerMaterialPath(int32 id, FString path);
	void registerEmissiveMaterial(int32 id);
	const FString& getAssetRootPath() const{
		return assetRootPath;
	}
	const FString& getAssetCommonPath() const{
		return assetCommonPath;
	}

	UMaterialInstanceConstant* getMaterialInstance(int32 id) const;
	UTexture *getTexture(int32 id) const;
	UTexture* loadTexture(int32 id) const;
	UMaterial* loadMaterial(int32 id) const;

	FString getMeshPath(JsonId id) const;
	UStaticMesh *loadStaticMeshById(JsonId id) const;

	void importScene(const FString& path);
	void importProject(const FString& path);

	void importResources(JsonObjPtr jsonNode);
	void loadTextures(const JsonValPtrs* textures);
	void loadMaterials(const JsonValPtrs* materials);
	void loadMeshes(const JsonValPtrs* meshes);
	void loadObjects(const JsonValPtrs* objects, ImportWorkData &importData);

	void setupAssetPaths(const FString &jsonFilename);

	JsonObjPtr loadJsonFromFile(const FString &filename);

	void importTexture(JsonObjPtr obj, const FString &rootPath);
	void importMaterial(JsonObjPtr obj, int32 matId);
	void importMaterialInstance(JsonObjPtr obj, int32 matId);
	void importMesh(JsonObjPtr obj, int32 meshId);
	void importObject(JsonObjPtr obj, int32 objId, ImportWorkData &importData);
	void importObject(const JsonGameObject &jsonGameObj , int32 objId, ImportWorkData &importData);

	static int findMatchingLength(const FString& arg1, const FString& arg2);
	FString findCommonPath(const JsonValPtrs* resources);
	FString getProjectImportPath() const;

	/*
	Ugh, this function again. I need to replace it with more compact version
	*/
	template<typename T> UPackage* createPackage(const FString &name, 
			const FString &filePath, const FString &rootPath, 
			const FString &objNameSuffix, FString *outPackageName, 
			FString *outObjName, T** outExistingObj) const{

		FString extension = FPaths::GetExtension(filePath);
		FString objDir = FPaths::GetPath(filePath);

		auto objSuffixName = ObjectTools::SanitizeObjectName(name + TEXT("_") + objNameSuffix);
		auto objName = ObjectTools::SanitizeObjectName(name);
		auto objInFileName = FPaths::Combine(*rootPath, *filePath);
		UE_LOG(JsonLog, Log, TEXT("Creating package. Object name: %s, filename: %s, extension: %s"), *objName, *objInFileName, *extension);

		FString packageName;

		FString packageRoot = TEXT("/Game/Import");
		if (sourceBaseName.Len())
			packageRoot = packageRoot + TEXT("/") + sourceBaseName;
		const int maxObjDirLength = 64;

		if (objDir.Len() > 0){
			if (objDir.StartsWith(assetCommonPath)){
				UE_LOG(JsonLog, Log, TEXT("Reducing asset path: %s"), *objDir);
				objDir = FString(*objDir + assetCommonPath.Len());
				UE_LOG(JsonLog, Log, TEXT("Shortened path: %s"), *objDir);
			}

			packageName = FPaths::Combine(*packageRoot, *objDir, *objName);
		}
		else
			packageName = FPaths::Combine(*packageRoot, *objName);

		packageName = PackageTools::SanitizePackageName(packageName);
		UE_LOG(JsonLog, Log, TEXT("Sanitized package name : %s"), *packageName);

		if (outPackageName){
			*outPackageName = packageName;
		}

		if (outObjName){
			*outObjName = objSuffixName;
		}

		UPackage *package = 0;

		T*existingObj = 0;
		{
			FString objPath = packageName + TEXT(".") + objSuffixName;
			existingObj = Cast<T>(LoadObject<T>(0, *objPath));
		}

		if (!existingObj){
			FString objPath = packageName + TEXT(".") + objName;
			existingObj = Cast<T>(LoadObject<T>(0, *objPath));
		}

		if (existingObj){
			package = existingObj->GetOutermost();
		}
		else{
			UE_LOG(JsonLog, Log, TEXT("Creating package %s"), *packageName);
			package = CreatePackage(0, *packageName);
			UE_LOG(JsonLog, Log, TEXT("Package created"));
		}

		if (outExistingObj)
			*outExistingObj = existingObj;

		UE_LOG(JsonLog, Log, TEXT("Done creating package"));
		return package;
	}
};
