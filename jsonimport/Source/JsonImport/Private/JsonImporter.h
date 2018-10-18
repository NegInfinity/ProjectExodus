#pragma once

#include "JsonTypes.h"
#include "JsonObjects.h"

class UMaterialExpression;
class UMaterialExpressionParameter;
class UMaterialExpressionVectorParameter;
class UMaterialExpressionConstant;
class UMaterialExpressionTextureSample;
class UTexture;
class UMaterial;


class JsonImporter{
protected:
	FString assetRootPath;
	FString assetCommonPath;
	FString sourceBaseName;
	IdNameMap meshIdMap;
	IdNameMap texIdMap;
	IdNameMap matIdMap;
	//This data should be reset between scenes. Otherwise thingsb ecome bad.
	//IdNameMap objectFolderPaths;
	//IdActorMap objectActors;
	IdSet emissiveMaterials;

	UMaterialExpression* createMaterialInput(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, const TCHAR* paramName, UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);
	UMaterialExpression* createMaterialInputMultiply(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, 
		const TCHAR* texParamName, const TCHAR* vecParamName,
		UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);
	UMaterialExpression* createMaterialSingleInput(UMaterial *material, float value, FExpressionInput &matInput, const TCHAR* inputName);
	UMaterialExpressionTextureSample *createTextureExpression(UMaterial *material, int32 matTextureId, const TCHAR* inputName, bool normalMap = false);
	UMaterialExpressionVectorParameter *createVectorExpression(UMaterial *material, FLinearColor color, const TCHAR* inputName);
	UMaterialExpressionConstant* createConstantExpression(UMaterial *material, float value, const TCHAR* constantName);
	template<typename Exp> Exp* createExpression(UMaterial *material){
		Exp* result = NewObject<Exp>(material);
		material->Expressions.Add(result);
		return result;
	}

	bool saveSceneObjectsAsWorld(const JsonValPtrs *sceneObjects, const FString &sceneName, const FString &scenePath);
	void processReflectionProbes(ImportWorkData &workData, const JsonGameObject &gameObj, int32 objId, AActor *parentActor, const FString &folderPath);
	void processLight(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonLight &light, AActor *parentActor, const FString& folderPath);
	void processLights(ImportWorkData &workData, const JsonGameObject &gameObj, AActor *parentActor, const FString& folderPath);
	void processMesh(ImportWorkData &workData, const JsonGameObject &gameObj, int objId, AActor *parentActor, const FString& folderPath);

	UWorld *createWorldForScene(const FString &sceneName, const FString &scenePath);
	bool saveLoadedWorld(UWorld *world, const FString &sceneName, const FString &sceneAssetPath);
	void importScene(JsonObjPtr sceneData, bool createWorld);
public:
	UTexture* loadTexture(int32 id);
	UMaterial* loadMaterial(int32 id);

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
	void importMesh(JsonObjPtr obj, int32 meshId);
	void importObject(JsonObjPtr obj, int32 objId, ImportWorkData &importData);

	static int findMatchingLength(const FString& arg1, const FString& arg2);
	FString findCommonPath(const JsonValPtrs* resources);

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
