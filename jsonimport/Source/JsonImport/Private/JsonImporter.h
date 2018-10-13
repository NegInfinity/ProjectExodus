#pragma once

using FStringArray = TArray<FString>;
using JsonObjPtr = TSharedPtr<FJsonObject>;
using JsonValPtr = TSharedPtr<FJsonValue>;
using JsonReaderRef = TSharedRef<TJsonReader<>>;
using JsonObjPtrs = TArray<JsonObjPtr>;
using JsonValPtrs = TArray<JsonValPtr>;
using IdNameMap = TMap<int, FString>;

class AActor;
using IdActorMap = TMap<int, AActor*>;
using IdSet = TSet<int>;

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
	IdNameMap objectFolderPaths;
	IdActorMap objectActors;
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
public:
	UTexture* loadTexture(int32 id);
	UMaterial* loadMaterial(int32 id);

	static void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName);
	static void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name);
	static void logValue(const FString &msg, const bool val);
	static FVector unityToUe(const FVector& arg);
	static void logValue(const FString &msg, const FVector2D &val);
	static void logValue(const FString &msg, const FVector &val);
	static void logValue(const FString &msg, const FQuat &val);
	static void logValue(const FString &msg, const FMatrix &val);
	static void logValue(const FString &msg, const int val);
	static void logValue(const FString &msg, const float val);
	static void logValue(const FString &msg, const FString &val);
	static void logValue(const FString &msg, const FLinearColor &val);
	static int32 getInt(JsonObjPtr data, const char* name);
	static bool getBool(JsonObjPtr data, const char* name);
	static float getFloat(JsonObjPtr data, const char* name);
	static FString getString(JsonObjPtr data, const char* name);
	static JsonObjPtr getObject(JsonObjPtr data, const char* name);

	static TArray<float> toFloatArray(const JsonValPtrs &inData){
		TArray<float> result;
		for(auto cur: inData){
			double val = 0.0;
			if (cur.IsValid()){
				cur->TryGetNumber(val);
			}
				//val = cur->AsNumber();
			result.Add(val);
		}
		return result;
	}
	
	static TArray<float> toFloatArray(const JsonValPtrs* inData){
		TArray<float> result;
		if (inData){
			for(auto cur: *inData){
				double val = 0.0;
				if (cur.IsValid()){
					cur->TryGetNumber(val);
				}
					//val = cur->AsNumber();
				result.Add(val);
			}
		}
		return result;
	}
	
	static TArray<int32> toIntArray(const JsonValPtrs &inData){
		TArray<int32> result;
		for(auto cur: inData){
			int32 val;
			if (cur.IsValid()){
				cur->TryGetNumber(val);
			}
			result.Add(val);
		}
		return result;
	}
	
	static FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	static FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity);
	static FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D());
	static FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector());
	static FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	static FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat());

	void importScene(const FString& path);
	void importProject(const FString& path);

	void importResources(JsonObjPtr jsonNode);
	void loadTextures(const JsonValPtrs* textures);
	void loadMaterials(const JsonValPtrs* materials);
	void loadMeshes(const JsonValPtrs* meshes);
	void loadObjects(const JsonValPtrs* objects);

	void setupAssetPaths(const FString &jsonFilename);

	JsonObjPtr loadJsonFromFile(const FString &filename);

	void importTexture(JsonObjPtr obj, const FString &rootPath);
	void importMaterial(JsonObjPtr obj, int32 matId);
	void importMesh(JsonObjPtr obj, int32 meshId);
	void importObject(JsonObjPtr obj, int32 objId);

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
		UE_LOG(JsonLog, Log, TEXT("Object name: %s, filename: %s, extension: %s"), *objName, *objInFileName, *extension);

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

