#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"

#include "MaterialBuilder.h"
#include "JsonObjects/JsonTerrainData.h"
#include "JsonObjects/JsonSkeleton.h"
#include "JsonObjects.h"
#include "ImportWorkData.h"

class UMaterialExpression;
class UMaterialExpressionParameter;
class UMaterialExpressionVectorParameter;
class UMaterialExpressionConstant;
class UMaterialExpressionTextureSample;
class UTexture;
class UTextureCube;
class UMaterial;
class ALandscape;
class ULandscapeLayerInfoObject;
class ULandscapeGrassType;
class APointLight;
class ASpotLight;
class ADirectionalLight;
class UPointLightComponent;
class USpotLightComponent;
class UDirectionalLightComponent;
class USphereReflectionCaptureComponent;
class UBoxReflectionCaptureComponent;
class UReflectionCaptureComponent;
class ULightComponent;
class USkeleton;
class USkeletalMesh;
class UAnimSequence;
class UBoxComponent;
class USphereComponent;
class UCapsuleComponent;
class UPrimitiveComponent;

class JsonImporter{
protected:
	FString assetRootPath;//TODO: rename to srcAssetRootPath. Points to json file folder.
	FString sourceExternDataPath;
	FString assetCommonPath;
	FString sourceBaseName;
	IdNameMap meshIdMap;
	IdNameMap skinMeshIdMap;
	IdNameMap texIdMap;
	IdNameMap cubeIdMap;
	IdNameMap matMasterIdMap;
	IdNameMap matInstIdMap;
	JsonExternResourceList externResources;

	TArray<JsonMaterial> jsonMaterials;
	TMap<JsonId, JsonSkeleton> jsonSkeletons;
	IdNameMap skeletonIdMap;

	AnimClipPathMap animClipPaths;//UAnimationSequence

	//TMap<JsonId, Json
	//IdNameMap animatorControllerIdMap;
	//IdNameMap animationClipIdMap;

	/*
	For now let's just load them all
	*/
	TMap<JsonId, JsonAnimationClip> animationClipMap;
	TMap<JsonId, JsonAnimatorController> animatorControllerMap;

	TMap<JsonId, JsonTerrainData> terrainDataMap;

	//This data should be reset between scenes. Otherwise thingsb ecome bad.
	IdSet emissiveMaterials;
	MaterialBuilder materialBuilder;

	//using ImportedObjectCallback = std::function<void(const ImportedObject &obj)>;

	static void registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg);

	void setupPointLightComponent(UPointLightComponent *pointLight, const JsonLight &jsonLight);
	void setupSpotLightComponent(USpotLightComponent *spotLight, const JsonLight &jsonLight);
	void setupDirLightComponent(ULightComponent *dirLight, const JsonLight &jsonLight);

	//UWorld* importSceneObjectsAsWorld(const TArray<JsonGameObject> &sceneObjects, const FString &sceneName, const FString &scenePath);
	UWorld* importSceneObjectsAsWorld(const JsonScene &scene, const FString &sceneNameOverride, const FString &scenePathOverride);

	static ImportedObject createBlankActor(ImportWorkData &workData, const JsonGameObject &gameObj);

	UBoxComponent *createBoxCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider) const;
	USphereComponent *createSphereCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider) const;
	UCapsuleComponent *createCapsuleCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider) const;
	UStaticMeshComponent *createMeshCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider) const;

	void makeComponentVisibleInEditor(USceneComponent *comp) const;

	UPrimitiveComponent* processCollider(ImportWorkData &workData, const JsonGameObject &gameObj, UObject *ownerPtr, const JsonCollider &collider);
	//void processColliders(ImportWorkData &workData, const JsonGameObject &gameObj, UObject *ownerPtr, TArray<UPrimitiveComponent*> &createdObjects);

	/*
	Alright....

	This function processes mesh and colliders for the game object, and if either of them are present, it returns ImportedObject that is meant to serve as a root object 
	for everything else found on this object.

	The function is a headache.
	*/
	ImportedObject processMeshAndColliders(ImportWorkData &workData, const JsonGameObject &gameObj, int objId, ImportedObject *parentObject, const FString &folderPath);

	ImportedObject processReflectionProbe(ImportWorkData &workData, const JsonGameObject &gameObj,
		const JsonReflectionProbe &probe, int32 objId, ImportedObject *parentObject, const FString &folderPath);
	void processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, int32 objId, ImportedObject *parentObject, const FString &folderPath,
		ImportedObjectArray *createdObjects);

	ImportedObject processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &light, ImportedObject *parentObject, const FString& folderPath);
	void processLights(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath,
		ImportedObjectArray *createdObjects);

	void setupCommonColliderSettings(const ImportWorkData &workData, UPrimitiveComponent *dstCollider, const JsonGameObject &jsonGameObj, const JsonCollider &collider) const;
	bool configureStaticMeshComponent(UStaticMeshComponent *meshComp, const JsonGameObject &gameObj, bool configForRender, const JsonCollider *collider) const;
	ImportedObject processStaticMesh(ImportWorkData &workData, const JsonGameObject &gameObj, int objId, ImportedObject *parentObject, const FString& folderPath, const JsonCollider *collider);

	ImportedObject processTerrain(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonTerrain &jsonTerrain, ImportedObject *parentObject, 
		const FString& folderPath);
	void processTerrains(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString& folderPath, 
		ImportedObjectArray *createdObjects);

	void processSkinMeshes(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString &folderPath,
		ImportedObjectArray *createdObjects);
	ImportedObject processSkinRenderer(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonSkinRenderer &skinRend, ImportedObject *parentObject, const FString &folderPath);

	void processAnimator(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonAnimator &jsonAnimator,
		ImportedObject *parentObject, const FString &folderPath);
	void processAnimators(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedObject *parentObject, const FString &folderPath);

	UWorld* importScene(const JsonScene &scene, bool createWorld);

	void importPrefab(const JsonPrefabData& prefab);
	void importPrefabs(const StringArray &prefabs);

	void importTerrainData(JsonObjPtr jsonData, JsonId terrainId, const FString &rootPath);
	void loadTerrains(const StringArray &terrains);

	/*
	ULandscapeGrassType* createGrassType(ImportWorkData &workData, const JsonGameObject &jsonGameObj, const JsonTerrainData &terrainData, 
		int layerIndex, const FString &terrainDataPth);*/

	void registerMaterialInstancePath(int32 id, FString path);
	void registerMasterMaterialPath(int32 id, FString path);

	void importStaticMesh(const JsonMesh &jsonMesh, int32 meshId);
	void importSkeletalMesh(const JsonMesh &jsonMesh, int32 meshId);

	void loadAnimatorsDebug(const StringArray &animatorPaths);
	void loadAnimClipsDebug(const StringArray &animClipPaths);

	static void setObjectHierarchy(const ImportedObject &object, ImportedObject *parentObject, const FString& folderPath, ImportWorkData &workData, const JsonGameObject &gameObj);

	//static void setActorHierarchy(AActor *actor, AActor *parentObject, const FString& folderPath, ImportWorkData &workData, const JsonGameObject &gameObj);

	//UWorld* importScene(const JsonScene &scene, bool createWorld) const;

	//JsonAnimatorController loadAnimatorController(int id) const;

	void processDelayedAnimators(const TArray<JsonGameObject> &objects, ImportWorkData &workData);
	void processDelayedAnimator(JsonId skelId, JsonId controllerId);

	template<typename T> bool loadIndexedExternResource(T& outObj, int index, const StringArray &resPaths) const{
		if ((index < 0 ) || (index >= resPaths.Num())){
			UE_LOG(JsonLog, Warning, TEXT("Could not load indexed extern resource %d"), index);
			return false;
		}

		const auto &resPath = resPaths[index];
		auto data = loadExternResourceFromFile(resPath);
		if (!data.IsValid()){
			UE_LOG(JsonLog, Warning, TEXT("Could not load indexed extern resource %d (%s)"), index, *resPath);
			return false;
		}

		outObj.load(data);
		return true;
	}
public:
	UAnimSequence* getAnimSequence(AnimClipIdKey key) const;
	void registerAnimSequence(AnimClipIdKey key, UAnimSequence *sequence);

	USkeleton* getSkeletonObject(int32 id) const;
	void registerSkeleton(int32 id, USkeleton *skel);

	JsonMesh loadJsonMesh(int32 id) const;
	const JsonMaterial* getJsonMaterial(int32 id) const;

	JsonAnimationClip loadAnimationClip(JsonId id) const;
	JsonAnimatorController loadAnimationController(JsonId id) const;

	void registerEmissiveMaterial(int32 id);
	const FString& getAssetRootPath() const{
		return assetRootPath;
	}
	const FString& getAssetCommonPath() const{
		return assetCommonPath;
	}

	UTextureCube* getCubemap(int32 id) const;
	UTextureCube* loadCubemap(int32 id) const;
	void importCubemap(JsonObjPtr data, const FString &rootPath);

	//UMaterialInstanceConstant* getMaterialInstance(int32 id) const;
	const JsonSkeleton* getSkeleton(int32 id) const;

	UTexture *getTexture(int32 id) const;
	UTexture* loadTexture(int32 id) const;
	UMaterialInstanceConstant* loadMaterialInstance(int32 id) const;
	UMaterial* loadMasterMaterial(int32 id) const;

	UMaterialInterface* loadMaterialInterface(int32 id) const;

	FString getMeshPath(JsonId id) const;
	UStaticMesh *loadStaticMeshById(JsonId id) const;
	USkeletalMesh *loadSkeletalMeshById(JsonId id) const;

	void importProject(const FString& path);

	void importResources(const JsonExternResourceList &resources);
	void loadCubemaps(const StringArray &cubemaps);
	void loadTextures(const StringArray & textures);
	void loadMaterials(const StringArray &materials);
	void loadSkeletons(const StringArray &materials);
	void loadMeshes(const StringArray &meshes);

	void loadObjects(const TArray<JsonGameObject> &objects, ImportWorkData &importData);

	void setupAssetPaths(const FString &jsonFilename);

	JsonObjPtr loadExternResourceFromFile(const FString &filename) const;

	void importTexture(JsonObjPtr obj, const FString &rootPath);

	void importTexture(const JsonTexture &tex, const FString &rootPath);

	void importMesh(JsonObjPtr obj, int32 meshId);
	void importMesh(const JsonMesh &jsonMesh, int32 meshId);
	void importObject(const JsonGameObject &jsonGameObj , int32 objId, ImportWorkData &importData);

	static int findMatchingLength(const FString& arg1, const FString& arg2);
	FString findCommonPath(const JsonValPtrs* resources) const;
	FString findCommonPath(const StringArray &resources) const;
	FString getProjectImportPath() const;

	/*
	Ugh, this function again. I need to replace it with more compact version
	*/
	template<typename T> UPackage* createPackage(const FString &name, 
			const FString &srcFilePath, const FString &targetRootPath, 
			const FString &objNameSuffix, FString *outPackageName, 
			FString *outObjName, T** outExistingObj) const{

		FString objDir = FPaths::GetPath(srcFilePath);

		auto objSuffixName = ObjectTools::SanitizeObjectName(name + TEXT("_") + objNameSuffix);
		auto objName = ObjectTools::SanitizeObjectName(name);
		auto objInFileName = FPaths::Combine(*targetRootPath, *srcFilePath);
		UE_LOG(JsonLog, Log, TEXT("Creating package. Object name: %s, filename: %s"), *objName, *objInFileName);

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

//void setActorHierarchy(AActor *actor, AActor *parentActor, const FString& folderPath, ImportWorkData &workData, const JsonGameObject &gameObj);
