// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "JsonImportStyle.h"
#include "JsonImportCommands.h"

#include "LevelEditor.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"

#include "RawMesh.h"

#include "DesktopPlatformModule.h"

static const FName JsonImportTabName("JsonImport");

#define LOCTEXT_NAMESPACE "FJsonImportModule"

DEFINE_LOG_CATEGORY(JsonLog);

using FStringArray = TArray<FString>;
using JsonObjPtr = TSharedPtr<FJsonObject>;
using JsonValPtr = TSharedPtr<FJsonValue>;
using JsonReaderRef = TSharedRef<TJsonReader<>>;
using JsonObjPtrs = TArray<JsonObjPtr>;
using JsonValPtrs = TArray<JsonValPtr>;
using IdNameMap = TMap<int, FString>;
using IdSet = TSet<int>;

class JsonImporter{
protected:
	FString assetRootPath;
	FString assetCommonPath;
	FString sourceBaseName;
	IdNameMap meshIdMap;
	IdNameMap texIdMap;
	IdNameMap matIdMap;
	IdNameMap objectFolderPaths;
	IdSet emissiveMaterials;

	UMaterialExpression* createMaterialInput(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);
	UMaterialExpression* createMaterialInputMultiply(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, UMaterialExpressionTextureSample ** outTexNode = 0,
		UMaterialExpressionVectorParameter **outVecParameter = 0);
	UMaterialExpression* createMaterialSingleInput(UMaterial *material, float value, FExpressionInput &matInput);
	UMaterialExpressionTextureSample *createTextureExpression(UMaterial *material, int32 matTextureId, bool normalMap = false);
	UMaterialExpressionVectorParameter *createVectorExpression(UMaterial *material, FLinearColor color);
	UMaterialExpressionConstant* createConstantExpression(UMaterial *material, float value);
	template<typename Exp> Exp* createExpression(UMaterial *material){
		Exp* result = NewObject<Exp>(material);
		material->Expressions.Add(result);
		return result;
	}
public:
	UTexture* loadTexture(int32 id);
	UMaterial* loadMaterial(int32 id);

	static void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName){
		if (!data->TryGetArrayField(name, valPtrs)){
			UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *warnName);
		}
	}
	static void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name){
		if (!data->TryGetArrayField(name, valPtrs)){
			UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *name);
		}
	}

	static void logValue(const FString &msg, const bool val){
		FString str = val ? TEXT("true"): TEXT("false");
		//const char *str = val ? "true": "false";
		UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *str);
	}

	/*
	static void logValue(const FString& msg, const MatTexture& tex){
		UE_LOG(JsonLog, Log, TEXT("%s: id %d, file %s"), *msg, tex.id, *tex.name);
	}
	*/

	static FVector unityToUe(const FVector& arg){
		return FVector(arg.Z, arg.X, arg.Y);
	}

	static void logValue(const FString &msg, const FVector2D &val){
		UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f"), *msg, val.X, val.Y);
	}

	static void logValue(const FString &msg, const FVector &val){
		UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f"), *msg, val.X, val.Y, val.Z);
	}

	static void logValue(const FString &msg, const FQuat &val){
		UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f, w %f"), *msg, val.X, val.Y, val.Z, val.W);
	}

	static void logValue(const FString &msg, const FMatrix &val){
		UE_LOG(JsonLog, Log, TEXT("%s:"), *msg);
		for(int i = 0; i < 4; i++)
			UE_LOG(JsonLog, Log, TEXT("    %f %f %f %f"), val.M[i][0], val.M[i][1], val.M[i][2], val.M[i][3]);
	}

	static void logValue(const FString &msg, const int val){
		UE_LOG(JsonLog, Log, TEXT("%s: %d"), *msg, val);
	}

	static void logValue(const FString &msg, const float val){
		UE_LOG(JsonLog, Log, TEXT("%s: %f"), *msg, val);
	}

	static void logValue(const FString &msg, const FString &val){
		UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *val);
	}

	static void logValue(const FString &msg, const FLinearColor &val){
		UE_LOG(JsonLog, Log, TEXT("%s: r %f, g %f, b %f, a %f"), *msg, val.R, val.G, val.B, val.A);
	}

	static int32 getInt(JsonObjPtr data, const char* name){
		return data->GetIntegerField(FString(name));
	}

	static bool getBool(JsonObjPtr data, const char* name){
		return data->GetBoolField(FString(name));
	}

	static float getFloat(JsonObjPtr data, const char* name){
		return data->GetNumberField(FString(name));
	}

	static FString getString(JsonObjPtr data, const char* name){
		return data->GetStringField(FString(name));
	}

	static JsonObjPtr getObject(JsonObjPtr data, const char* name){
		return data->GetObjectField(FString(name));
	}

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
	
	static FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor()){
		FLinearColor result = defaultVal;

		auto colorObj = getObject(data, name);
		if (!colorObj.IsValid())
				return result;
		result.A = getFloat(colorObj, "a");
		result.R = getFloat(colorObj, "r");
		result.G = getFloat(colorObj, "g");
		result.B = getFloat(colorObj, "b");

		return result;
	}

	/*
	static bool getBool(JsonObjPtr data, const char* name, const bool defaultVal = false){
		return getString(data, name) == FString("true");
	}
	*/

	static FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity){
		FMatrix result = defaultVal;
		auto matObj = getObject(data, name);
		if (!matObj.IsValid())
				return result;

		result.M[0][0] = getFloat(matObj, "e00");
		result.M[0][1] = getFloat(matObj, "e10");
		result.M[0][2] = getFloat(matObj, "e20");
		result.M[0][3] = getFloat(matObj, "e30");
		result.M[1][0] = getFloat(matObj, "e01");
		result.M[1][1] = getFloat(matObj, "e11");
		result.M[1][2] = getFloat(matObj, "e21");
		result.M[1][3] = getFloat(matObj, "e31");
		result.M[2][0] = getFloat(matObj, "e02");
		result.M[2][1] = getFloat(matObj, "e12");
		result.M[2][2] = getFloat(matObj, "e22");
		result.M[2][3] = getFloat(matObj, "e32");
		result.M[3][0] = getFloat(matObj, "e03");
		result.M[3][1] = getFloat(matObj, "e13");
		result.M[3][2] = getFloat(matObj, "e23");
		result.M[3][3] = getFloat(matObj, "e33");
		
		return result;
	}

	static FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D()){
		FVector2D result = defaultVal;

		auto vecObj = getObject(data, name);
		if (!vecObj.IsValid())
				return result;
		result.X = getFloat(vecObj, "X");
		result.Y = getFloat(vecObj, "Y");

		return result;
	}

	static FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector()){
		FVector result = defaultVal;

		auto vecObj = getObject(data, name);
		if (!vecObj.IsValid())
				return result;
		result.X = getFloat(vecObj, "X");
		result.Y = getFloat(vecObj, "Y");
		result.Z = getFloat(vecObj, "Z");

		return result;
	}

	static FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor()){
		FLinearColor result = defaultVal;

		auto vecObj = getObject(data, name);
		if (!vecObj.IsValid())
				return result;
		result.R = getFloat(vecObj, "r");
		result.G = getFloat(vecObj, "g");
		result.B = getFloat(vecObj, "b");
		result.A = getFloat(vecObj, "a");

		return result;
	}

	static FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat()){
		FQuat result = defaultVal;

		auto vecObj = getObject(data, name);
		if (!vecObj.IsValid())
				return result;
		result.X = getFloat(vecObj, "X");
		result.Y = getFloat(vecObj, "Y");
		result.Z = getFloat(vecObj, "Z");
		result.W = getFloat(vecObj, "W");

		return result;
	}

	void importScene(const FString& path);

	void importTexture(JsonObjPtr obj, const FString &rootPath);
	void importMaterial(JsonObjPtr obj, int32 matId);
	void importMesh(JsonObjPtr obj, int32 meshId);
	void importObject(JsonObjPtr obj, int32 objId);

	static int findMatchingLength(const FString& arg1, const FString& arg2){
		int result = 0;
		for(; (result < arg1.Len()) && (result < arg2.Len()); result++){
			if (arg1[result] != arg2[result])
				break;
		} 
		return result;
	}

	FString findCommonPath(const JsonValPtrs* resources){
		FString result;
		if (!resources)
			return result;
		UE_LOG(JsonLog, Log, TEXT("Listing files"));
		bool first = true;
		int minCommonLength = 0;

		for(auto cur: *resources){
			auto resPath = cur->AsString();
			UE_LOG(JsonLog, Log, TEXT("Resource %s is found"), *resPath);
			auto curPath = FPaths::GetPath(resPath);
			auto curLen = curPath.Len();
			auto ext = FPaths::GetExtension(resPath);
			if (!ext.Len()){
				UE_LOG(JsonLog, Warning, TEXT("Invalid ext for candidate %s"), *resPath);
				continue;
			}
			if (first){
				result = curPath;
				minCommonLength = curLen;
				first = false;
				continue;
			}

			auto commonLen = findMatchingLength(curPath, result);
			if (commonLen < minCommonLength){
				minCommonLength = commonLen;
				result = FString(minCommonLength, *curPath);
			}
			if (commonLen == 0)
				return FString();
		}
		return result;
	}

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

UMaterialExpressionConstant* createConstantExpression(UMaterial *material, float value){
	auto matConstant = NewObject<UMaterialExpressionConstant>(material);
	material->Expressions.Add(matConstant);
	matConstant->R = value;
	return matConstant;
}

UMaterialExpression* JsonImporter::createMaterialSingleInput(UMaterial *unrealMaterial, float value, FExpressionInput &matInput){
	auto matConstant = NewObject<UMaterialExpressionConstant>(unrealMaterial);
	unrealMaterial->Expressions.Add(matConstant);
	matConstant->R = value;
	matInput.Expression = matConstant;
	return matConstant;
}

UMaterialExpressionVectorParameter* JsonImporter::createVectorExpression(UMaterial *material, FLinearColor color){
	UMaterialExpressionVectorParameter* vecExpression = 
		NewObject<UMaterialExpressionVectorParameter>(material);
	material->Expressions.Add(vecExpression);

	//matInput.Expression = vecExpression;
	vecExpression->DefaultValue.R = color.R;
	vecExpression->DefaultValue.G = color.G;
	vecExpression->DefaultValue.B = color.B;
	vecExpression->DefaultValue.A = color.A;
	return vecExpression;
}

UMaterialExpressionTextureSample* JsonImporter::createTextureExpression(UMaterial *material, int32 matTextureId, bool normalMap){
	UMaterialExpressionTextureSample *result = 0;
	UE_LOG(JsonLog, Log, TEXT("Creating texture sample expression"));

	logValue(TEXT("texId: "), matTextureId);
	auto unrealTex = loadTexture(matTextureId);

	if (!unrealTex){
		UE_LOG(JsonLog, Warning, TEXT("Texture not found"));
	}
	result = NewObject<UMaterialExpressionTextureSample>(material);
	result->SamplerType = normalMap ? SAMPLERTYPE_Normal: SAMPLERTYPE_Color;
	material->Expressions.Add(result);
	result->Texture = unrealTex;

	return result;
}

UMaterialExpression* JsonImporter::createMaterialInputMultiply(UMaterial *material, int32 matTextureId, 
		const FLinearColor *matColor, FExpressionInput &matInput, UMaterialExpressionTextureSample ** outTexNode,
		UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating multiply material input"));
	UMaterialExpressionTextureSample *texExp = 0;
	UMaterialExpressionVectorParameter *vecExp = 0;
	bool hasTex = matTextureId >= 0;
	if (hasTex){
		texExp = createTextureExpression(material, matTextureId);
		if (outTexNode)
			*outTexNode = texExp;
	}
	if (matColor && ((*matColor != FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)) || !hasTex)){
		vecExp = createVectorExpression(material, *matColor);
		if (outVecParameter)
			*outVecParameter = vecExp;
	}

	UMaterialExpression* result = 0;
	if (vecExp && texExp){
		auto mulExp = createExpression<UMaterialExpressionMultiply>(material);
		mulExp->A.Expression = texExp;
		mulExp->B.Expression = vecExp;
		result = mulExp;
	}
	else if (vecExp){
		result = vecExp;
	}
	else if (texExp){
		result = texExp;
	}

	if (result){
		matInput.Expression = result;
	}

	return result;
}

UMaterialExpression* JsonImporter::createMaterialInput(UMaterial *unrealMaterial, int32 matTextureId, const FLinearColor *matColor, FExpressionInput &matInput, bool normalMap, 
		UMaterialExpressionTextureSample **outTexNode, UMaterialExpressionVectorParameter **outVecParameter){
	UE_LOG(JsonLog, Log, TEXT("Creating material input"));
	logValue(TEXT("texId: "), matTextureId);
	auto unrealTex = loadTexture(matTextureId);

	if (unrealTex){
		UE_LOG(JsonLog, Log, TEXT("Texture found"));
		auto texExpression = NewObject<UMaterialExpressionTextureSample>(unrealMaterial);
		unrealMaterial->Expressions.Add(texExpression);
		matInput.Expression = texExpression;
		texExpression->Texture = unrealTex;
		texExpression->SamplerType = normalMap ? SAMPLERTYPE_Normal: SAMPLERTYPE_Color;
		if (outTexNode)
			*outTexNode = texExpression;
		return texExpression;
	}
	else{
		UE_LOG(JsonLog, Log, TEXT("Texture not found"));
		if (matColor){
			UE_LOG(JsonLog, Log, TEXT("Trying to create color node"));
			auto vecExpression = NewObject<UMaterialExpressionVectorParameter>(unrealMaterial);
			unrealMaterial->Expressions.Add(vecExpression);
			matInput.Expression = vecExpression;
			vecExpression->DefaultValue.R = matColor->R;
			vecExpression->DefaultValue.G = matColor->G;
			vecExpression->DefaultValue.B = matColor->B;
			vecExpression->DefaultValue.A = matColor->A;
			if (outVecParameter)
				*outVecParameter = vecExpression;
			return vecExpression;
		}
	}
	return nullptr;
}

UMaterial* JsonImporter::loadMaterial(int32 id){
	UE_LOG(JsonLog, Log, TEXT("Looking for material %d"), id);
	if (id < 0){
		UE_LOG(JsonLog, Log, TEXT("Invalid id %d"), id);
		return 0;
	}

	if (!matIdMap.Contains(id)){
		UE_LOG(JsonLog, Log, TEXT("Id %d is not in the map"), id);
		return 0;
	}

	auto matPath = matIdMap[id];
	UMaterial *mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), 0, *matPath));
	UE_LOG(JsonLog, Log, TEXT("Material located"));
	return mat;
}


UTexture* JsonImporter::loadTexture(int32 id){
	UE_LOG(JsonLog, Log, TEXT("Looking for tex %d"), id);
	if (id < 0){
		UE_LOG(JsonLog, Log, TEXT("Invalid id %d"), id);
		return 0;
	}
	if (!texIdMap.Contains(id)){
		UE_LOG(JsonLog, Log, TEXT("Id %d is not in the map"), id);
		return 0;
	}
	auto texPath = texIdMap[id];
	UTexture *texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), 0, *texPath));
	UE_LOG(JsonLog, Log, TEXT("Texture located"));
	return texture;
}

void JsonImporter::importObject(JsonObjPtr obj, int32 objId){
	UE_LOG(JsonLog, Log, TEXT("Importing object %d"), objId);

#define GETPARAM2(varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(name, getString)
	GETPARAM(id, getInt)
	GETPARAM(instanceId, getInt)
	GETPARAM(localPosition, getVector)
	GETPARAM(localRotation, getQuat)
	GETPARAM(localScale, getVector)
	GETPARAM(worldMatrix, getMatrix)
	GETPARAM(localMatrix, getMatrix)
	GETPARAM2(parentId, parent, getInt)
	GETPARAM2(meshId, mesh, getInt)

	GETPARAM(isStatic, getBool)
	GETPARAM(lightMapStatic, getBool)
	GETPARAM(navigationStatic, getBool)
	GETPARAM(occluderStatic, getBool)
	GETPARAM(occludeeStatic, getBool)
	//renderer
	//light
#undef GETPARAM
#undef GETPARAM
	auto rendererArray = obj->GetArrayField("renderer");
	auto lightArray = obj->GetArrayField("light");

	FString folderPath;
	FString curFolderPath = name;
	if (parentId >= 0){
		const FString* found = objectFolderPaths.Find(parentId);
		if (found){
			folderPath = *found;
			curFolderPath = folderPath + "/" + name;
		}
		else{
			UE_LOG(JsonLog, Log, TEXT("Object parent not found, folder path may be invalid"));
		}
	}
	UE_LOG(JsonLog, Log, TEXT("Folder path for object: %d: %s"), id, *folderPath);
	objectFolderPaths.Add(id, curFolderPath);

	bool hasRenderer = false, hasLight = false;
	bool receiveShadows = true;
	FString shadowCastingMode;

	TArray<int32> materials;
	//FVector4 lightmapScaleOffset
	if (rendererArray.Num() > 0){
		auto rendVal = rendererArray[0];
		auto rendObj = rendVal->AsObject();
		if (rendObj.IsValid()){
			hasRenderer = true;
			receiveShadows = getBool(rendObj, "receiveShadows");
			shadowCastingMode = getString(rendObj, "shadowCastingMode");
			const JsonValPtrs* matValues = 0;
			loadArray(rendObj, matValues, "materials");
			if (matValues)
				materials = toIntArray(*matValues);
		}
	}

	float lightRange = 0.0f;
	float lightSpotAngle = 0.0f;
	bool lightCastShadow = false;
	FString lightType;
	float lightShadowStrength = 0.0f;
	float lightIntensity = 0.0f;
	FLinearColor lightColor;
	float lightBounceIntensity = 0.0f;
	FString lightRenderMode;
	FString lightShadows;

	if (lightArray.Num() > 0){
		auto lightVal= lightArray[0];
		auto lightObj = lightVal->AsObject();
		if (lightObj.IsValid()){
			hasLight = true;
			lightRange = getFloat(lightObj, "range");
			lightSpotAngle = getFloat(lightObj, "spotAngle");
			lightType = getString(lightObj, "type");
			lightShadowStrength= getFloat(lightObj, "shadowStrength");
			lightIntensity = getFloat(lightObj, "intensity");
			lightBounceIntensity = getFloat(lightObj, "bounceIntensity");
			lightColor = getColor(lightObj, "color");
			lightRenderMode = getString(lightObj, "renderMode");
			lightShadows = getString(lightObj, "shadows");
			lightCastShadow = lightShadows != "Off";
		}
	}

	FVector xAxis, yAxis, zAxis;
	worldMatrix.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = worldMatrix.GetOrigin();
	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);
	FMatrix ueMatrix;
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);

	if (!hasLight && (meshId < 0))
		return;
	auto world = GEditor->GetEditorWorldContext().World();
	if (!world){
		UE_LOG(JsonLog, Warning, TEXT("No world"));
		return; 
	}

	const float ueAttenuationBoost = 2.0f;

	if (hasLight){
		UE_LOG(JsonLog, Log, TEXT("Creating light"));
		FActorSpawnParameters spawnParams;
		FTransform transform;
		FVector lightX, lightY, lightZ;
		ueMatrix.GetScaledAxes(lightX, lightY, lightZ);
		FVector lightNewX = lightZ;
		FVector lightNewY = lightY;
		FVector lightNewZ = -lightX;
		FMatrix lightMatrix = ueMatrix;
		lightMatrix.SetAxes(&lightNewX, &lightNewY, &lightNewZ);
		transform.SetFromMatrix(lightMatrix);

		if (lightType == "Point"){
			APointLight *actor = Cast<APointLight>(GEditor->AddActor(GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				APointLight::StaticClass(), transform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn point light"));
			}
			else{
				actor->SetActorLabel(name, true);
				if (folderPath.Len())
					actor->SetFolderPath(FName(*folderPath));

				auto light = actor->PointLightComponent;
				//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity

				light->SetIntensity(lightIntensity);
				light->bUseInverseSquaredFalloff = false;
				//light->LightFalloffExponent = 2.0f;
				light->SetLightFalloffExponent(2.0f);

				light->SetLightColor(lightColor);
				float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;//those are fine
				light->AttenuationRadius = attenRadius;
				light->SetAttenuationRadius(attenRadius);
				light->CastShadows = lightCastShadow;// != FString("None");
				//light->SetVisibility(params.visible);
				if (isStatic)
					actor->SetMobility(EComponentMobility::Static);
				actor->MarkComponentsRenderStateDirty();
			}
		}
		else if (lightType == "Spot"){
			ASpotLight *actor = Cast<ASpotLight>(GEditor->AddActor(GCurrentLevelEditingViewportClient->GetWorld()->GetCurrentLevel(),
				ASpotLight::StaticClass(), transform));
			if (!actor){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn point light"));
			}
			else{
				actor->SetActorLabel(name, true);
				if (folderPath.Len())
					actor->SetFolderPath(FName(*folderPath));

				auto light = actor->SpotLightComponent;
				//light->SetIntensity(lightIntensity * 2500.0f);//100W lamp per 1 point of intensity
				light->SetIntensity(lightIntensity);
				light->bUseInverseSquaredFalloff = false;
				//light->LightFalloffExponent = 2.0f;
				light->SetLightFalloffExponent(2.0f);


				light->SetLightColor(lightColor);
				float attenRadius = lightRange*100.0f;//*ueAttenuationBoost;
				light->AttenuationRadius = attenRadius;
				light->SetAttenuationRadius(attenRadius);
				light->CastShadows = lightCastShadow;// != FString("None");
				//light->InnerConeAngle = lightSpotAngle * 0.25f;
				light->InnerConeAngle = 0.0f;
				light->OuterConeAngle = lightSpotAngle * 0.5f;
				//light->SetVisibility(params.visible);
				if (isStatic)
					actor->SetMobility(EComponentMobility::Static);
				actor->MarkComponentsRenderStateDirty();
			}
		}
	}

	if (meshId < 0)
		return;

	UE_LOG(JsonLog, Log, TEXT("Mesh found in object %d, name %s"), objId, *name);
	
	//ueMatrix.SetScaledAxes(zAxis, xAxis, yAxis);
	//ueMatrix.SetOrigin(pos);

	//visible/static parameters
	auto meshPath = meshIdMap[meshId];
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(ueMatrix);

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return;
	}

	AActor *actor = world->SpawnActor<AActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
	if (!actor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn actor"));
		return;
	}

	actor->SetActorLabel(name, true);
	actor->SetFolderPath(FName(*folderPath));

	AStaticMeshActor *worldMesh = Cast<AStaticMeshActor>(actor);
	//if params is static
	if (!worldMesh){
		UE_LOG(JsonLog, Warning, TEXT("Wrong actor class"));
		return;
	}

	auto meshComp = worldMesh->GetStaticMeshComponent();
	meshComp->StaticMesh = meshObject;

	bool hasShadows = false;
	bool twoSidedShadows = false;
	bool hideInGame = false;
	if (shadowCastingMode == FString("ShadowsOnly")){
		twoSidedShadows = false;
		hasShadows = true;
		hideInGame = true;
	}
	else if (shadowCastingMode == FString("On")){
		hasShadows = true;
		twoSidedShadows = false;
	}
	else if (shadowCastingMode == FString("TwoSided")){
		hasShadows = true;
		twoSidedShadows = true;
	}
	else{
		hasShadows = false;
		twoSidedShadows = false;
	}
	logValue("hasShadows", hasShadows);
	logValue("twoSidedShadows", twoSidedShadows);

	worldMesh->SetActorHiddenInGame(hideInGame);

	if (isStatic)
		meshComp->SetMobility(EComponentMobility::Static);

	if (meshObject){
		bool emissiveMesh = false;
		for(auto cur: meshObject->Materials){
			auto mat = cur->GetMaterial();
			if (!mat)
				continue;
			if (mat->EmissiveColor.IsConnected()){
				emissiveMesh = true;
				break;
			}
//			if (mat->
			//cur->Emi
		}
		meshComp->LightmassSettings.bUseEmissiveForStaticLighting = emissiveMesh;
	}
	//meshComp->LightmassSettings


	meshComp->SetCastShadow(hasShadows);
	meshComp->bCastShadowAsTwoSided = twoSidedShadows;

	worldMesh->MarkComponentsRenderStateDirty();
}

void JsonImporter::importMesh(JsonObjPtr obj, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh %d"), meshId);
#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(id, getInt)
	GETPARAM(vertexCount, getInt)
	GETPARAM(path, getString)
	GETPARAM(name, getString)
	GETPARAM(subMeshCount, getInt)
#undef GETPARAM
	//auto verts = getObject(obj, "verts");
	//auto materials = getObject(obj, "materials");

	const JsonValPtrs *verts = 0, *materials = 0, *colors = 0, *normals = 0, *uv0 = 0, *uv1 = 0, *uv2 = 0, *uv3 = 0, *subMeshes = 0;

	loadArray(obj, verts, TEXT("verts"));
	if (verts){
		UE_LOG(JsonLog, Log, TEXT("Verts: %d"), verts->Num());
	}
	loadArray(obj, subMeshes, TEXT("submeshes"));
	if (subMeshes){
		UE_LOG(JsonLog, Log, TEXT("submeshes: %d"), subMeshes->Num());
	}
	loadArray(obj, materials, TEXT("materials"));
	if (materials){
		UE_LOG(JsonLog, Log, TEXT("Materials: %d"), materials->Num());
	}
	loadArray(obj, colors, TEXT("colors"));
	if (colors){
		UE_LOG(JsonLog, Log, TEXT("Colors: %d"), colors->Num());
	}
	loadArray(obj, normals, TEXT("normals"));
	if (normals){
		UE_LOG(JsonLog, Log, TEXT("Normals: %d"), normals->Num());
	}
	loadArray(obj, uv0, TEXT("uv0"));
	if (uv0){
		UE_LOG(JsonLog, Log, TEXT("uv0: %d"), uv0->Num());
	}
	loadArray(obj, uv1, TEXT("uv1"));
	if (uv1){
		UE_LOG(JsonLog, Log, TEXT("uv1: %d"), uv1->Num());
	}
	loadArray(obj, uv2, TEXT("uv2"));
	if (uv2){
		UE_LOG(JsonLog, Log, TEXT("uv2: %d"), uv2->Num());
	}
	loadArray(obj, uv3, TEXT("uv3"));
	if (uv3){
		UE_LOG(JsonLog, Log, TEXT("uv3: %d"), uv3->Num());
	}

	if (!verts){
		UE_LOG(JsonLog, Warning, TEXT("No verts, cannot create mesh!"));
		return;
	}

	FString sanitizedMeshName;
	FString sanitizedPackageName;

	UStaticMesh *existingMesh = nullptr;
	UPackage *meshPackage = createPackage(
		name, path, assetRootPath, FString("Mesh"), 
		&sanitizedPackageName, &sanitizedMeshName, &existingMesh);
	if (existingMesh){
		meshIdMap.Add(id, existingMesh->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing mesh: %s (package %s)"), *sanitizedMeshName, *sanitizedPackageName);
		return;
	}

	UStaticMesh* mesh = NewObject<UStaticMesh>(meshPackage, FName(*sanitizedMeshName), RF_Standalone|RF_Public);

	if (!mesh){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't create mesh"));
		return;
	}

	UE_LOG(JsonLog, Log, TEXT("Static mesh num lods: %d"), mesh->SourceModels.Num());

	if (mesh->SourceModels.Num() < 1){
		UE_LOG(JsonLog, Warning, TEXT("Adding static mesh lod!"));
		new(mesh->SourceModels) FStaticMeshSourceModel();//???
	}

	int32 lod = 0;

	FStaticMeshSourceModel &srcModel = mesh->SourceModels[lod];

	mesh->LightingGuid = FGuid::NewGuid();
	mesh->LightMapResolution = 64;
	mesh->LightMapCoordinateIndex = 1;

	FRawMesh newRawMesh;
	srcModel.RawMeshBulkData->LoadRawMesh(newRawMesh);
	newRawMesh.VertexPositions.SetNum(0);

	/*
	bool hasNormals = true;
	{
		FVector a(0.0f, 0.0f, 0.0f), b(100.0f, 0.0f, 0.0f), c(0.0f, 0.0f, 100.0f);
		newRawMesh.VertexPositions.Add(a);
		newRawMesh.VertexPositions.Add(b);
		newRawMesh.VertexPositions.Add(c);

		newRawMesh.WedgeIndices.Add(0);
		newRawMesh.WedgeIndices.Add(1);
		newRawMesh.WedgeIndices.Add(2);

		FVector n(0.0f, 1.0, 0.0f);
		FVector tanU(1.0f, 0.0, 0.0f);
		FVector tanV(0.0f, 0.0, 1.0f);

		FVector2D uvA(0.0f, 0.0f), uvB(1.0f, 0.0f), uvC(0.0f, 1.0f);

		newRawMesh.WedgeTexCoords[0].Add(uvA);
		newRawMesh.WedgeTexCoords[0].Add(uvB);
		newRawMesh.WedgeTexCoords[0].Add(uvC);


		FColor color(255, 255, 255, 255);
		newRawMesh.FaceMaterialIndices.Add(0);
		newRawMesh.FaceSmoothingMasks.Add(0);
	}
	*/

	bool hasNormals = false;
	{
		UE_LOG(JsonLog, Log, TEXT("Generating mesh"));
		auto vertFloats = toFloatArray(verts);
		UE_LOG(JsonLog, Log, TEXT("Num vert floats: %d"), vertFloats.Num());
		float scale = 100.0f;
		for(int i = 0; (i + 2) < vertFloats.Num(); i += 3){
			FVector pos(vertFloats[i], vertFloats[i+1], vertFloats[i+2]);
			newRawMesh.VertexPositions.Add(unityToUe(pos * scale));
		}
		UE_LOG(JsonLog, Log, TEXT("Num verts: %d"), newRawMesh.VertexPositions.Num());

		int32 faceIndex = 0;

		auto normalFloats = toFloatArray(normals);
		UE_LOG(JsonLog, Log, TEXT("Num normal floats: %d"), vertFloats.Num());
		hasNormals = normalFloats.Num() != 0;
		UE_LOG(JsonLog, Log, TEXT("has normals: %d"), (int)hasNormals);
		const int32 maxUvs = 4;

		TArray<float> uvFloats[maxUvs] = {
			toFloatArray(uv0), toFloatArray(uv1), toFloatArray(uv2), toFloatArray(uv3)
		};
		bool hasUvs[maxUvs];
		for(int32 i = 0; i < maxUvs; i++){
			hasUvs[i] = uvFloats[i].Num() != 0;
			UE_LOG(JsonLog, Log, TEXT("Uv floats[%d]: %d, hasUvs: %d"), i, uvFloats[i].Num(), (int)hasUvs[i]);
		}

		//submesh generation
		newRawMesh.WedgeIndices.SetNum(0);

		for(int i = 0; i < MAX_MESH_TEXTURE_COORDS; i++)
			newRawMesh.WedgeTexCoords[i].SetNum(0);

		newRawMesh.WedgeColors.SetNum(0);
		newRawMesh.WedgeTangentZ.SetNum(0);

		UE_LOG(JsonLog, Log, TEXT("Sub meshes: %d"), (int)(subMeshes? subMeshes->Num(): 0));

		if (subMeshes){
			UE_LOG(JsonLog, Log, TEXT("Processing submeshes"));
			int32 nextMatIndex = 0;
			auto getIdxVector2 = [](TArray<float>& floats, int32 idx){
				if (floats.Num() <= (idx*2 + 1))
					return FVector2D();
				return FVector2D(floats[idx*2], floats[idx*2+1]);
			};

			auto getIdxVector3 = [](TArray<float>& floats, int32 idx){
				if (floats.Num() <= (idx*3 + 2))
					return FVector();
				return FVector(floats[idx*3], floats[idx*3+1], floats[idx*3+2]);
			};

			for(auto cur: *subMeshes){
				auto matIndex = nextMatIndex;
				nextMatIndex++;				
				UE_LOG(JsonLog, Log, TEXT("Processing submesh %d"), matIndex);
				auto trigJson = cur->AsObject();
				if (!trigJson.IsValid())
					continue;
				const JsonValPtrs* trigObj = 0;
				loadArray(trigJson, trigObj, "Triangles");
				if (!trigObj)
					continue;
				auto trigs = toIntArray(*trigObj);
				UE_LOG(JsonLog, Log, TEXT("Num triangle verts %d"), trigs.Num());

				auto processIndex = [&](int32 trigVertIdx){
					auto origIndex = trigs[trigVertIdx];
					newRawMesh.WedgeIndices.Add(origIndex);
					if (hasNormals)
						newRawMesh.WedgeTangentZ.Add(unityToUe(getIdxVector3(normalFloats, origIndex)));
					for(int32 uvIndex = 0; uvIndex < maxUvs; uvIndex++){
						if (!hasUvs[uvIndex])
							continue;
						auto tmpUv = getIdxVector2(uvFloats[uvIndex], origIndex);
						tmpUv.Y = 1.0f - tmpUv.Y;
						newRawMesh.WedgeTexCoords[uvIndex].Add(tmpUv);
					}
					if ((trigVertIdx % 3) == 0){
						newRawMesh.FaceMaterialIndices.Add(matIndex);
						newRawMesh.FaceSmoothingMasks.Add(0);
					}
				};

				for(int32 trigVertIndex = 0; (trigVertIndex + 2) < trigs.Num(); trigVertIndex += 3){
					processIndex(trigVertIndex);
					processIndex(trigVertIndex + 2);
					processIndex(trigVertIndex + 1);
					/*
					auto origIndex = trigs[trigVertIdx];
					newRawMesh.WedgeIndices.Add(origIndex);
					if (hasNormals)
						newRawMesh.WedgeTangentZ.Add(unityToUe(getIdxVector3(normalFloats, origIndex)));
					for(int32 uvIndex = 0; uvIndex < maxUvs; uvIndex++){
						if (!hasUvs[uvIndex])
							continue;
						newRawMesh.WedgeTexCoords[uvIndex].Add(getIdxVector2(uvFloats[uvIndex], origIndex));
					}
					if ((trigVertIdx % 3) == 0){
						newRawMesh.FaceMaterialIndices.Add(matIndex);
						newRawMesh.FaceSmoothingMasks.Add(0);
					}
					*/
				}
				UE_LOG(JsonLog, Log, TEXT("New wedge indices %d"), newRawMesh.WedgeIndices.Num());
				UE_LOG(JsonLog, Log, TEXT("Face mat indices: %d"), newRawMesh.FaceMaterialIndices.Num());
				for(int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; i++){
					UE_LOG(JsonLog, Log, TEXT("Uv[%d] size: %d"), i, newRawMesh.WedgeTexCoords[i].Num());
				}
			}
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("No Submeshes found!"));
		}
	}

	bool valid = newRawMesh.IsValid();
	bool fixable = newRawMesh.IsValidOrFixable();
	UE_LOG(JsonLog, Log, TEXT("Mesh is valid: %d, mesh is validOrFixable: %d"), (int)valid, (int)fixable);
	if (!valid){
		UE_LOG(JsonLog, Warning, TEXT("Mesh is not valid!"));
		if (!fixable){
			UE_LOG(JsonLog, Warning, TEXT("Mesh is not fixable!"));
		}
	}

	mesh->Materials.Empty();
	if (materials){
		auto matIds = toIntArray(*materials);
		for(auto matId: matIds){
			UMaterial *material = loadMaterial(matId);
			mesh->Materials.Add(material);
		}
	}

	srcModel.RawMeshBulkData->SaveRawMesh(newRawMesh);

	srcModel.BuildSettings.bRecomputeNormals = false;//!hasNormals;//hasNormals
	srcModel.BuildSettings.bRecomputeTangents = true;

	TArray<FText> buildErrors;
	mesh->Build(false, &buildErrors);
	for(FText& err: buildErrors){
		UE_LOG(JsonLog, Error, TEXT("MeshBuildError: %s"), *(err.ToString()));
	}
	//srcModel.BuildSettings

	if (mesh){
		meshIdMap.Add(id, mesh->GetPathName());
		FAssetRegistryModule::AssetCreated(mesh);
		meshPackage->SetDirtyFlag(true);
	}
}

void JsonImporter::importMaterial(JsonObjPtr obj, int32 matId){
	UE_LOG(JsonLog, Log, TEXT("Importing material %d"), matId);

#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(name, getString)
	GETPARAM(id, getInt)
	GETPARAM(path, getString)
	GETPARAM(shader, getString)
	GETPARAM(renderQueue, getInt)

	GETPARAM(mainTexture, getInt)
	GETPARAM(mainTextureOffset, getVector2)
	GETPARAM(mainTextureScale, getVector2)
	GETPARAM(color, getLinearColor)

	GETPARAM(useNormalMap, getBool)
	GETPARAM(useAlphaTest, getBool)
	GETPARAM(useAlphaPremultiply, getBool)
	GETPARAM(useEmission, getBool)
	GETPARAM(useParallax, getBool)
	GETPARAM(useDetailMap, getBool)
	GETPARAM(useMetallic, getBool)
	GETPARAM(useSpecular, getBool)
	GETPARAM(albedoTex, getInt)
	GETPARAM(specularTex, getInt)
	GETPARAM(normalMapTex, getInt)
	GETPARAM(occlusionTex, getInt)
	GETPARAM(parallaxTex, getInt)
	GETPARAM(emissionTex, getInt)
	GETPARAM(detailMaskTex, getInt)
	GETPARAM(detailAlbedoTex, getInt)
	GETPARAM(detailNormalMapTex, getInt)

	GETPARAM(alphaCutoff, getFloat)
	GETPARAM(smoothness, getFloat)
	GETPARAM(specularColor, getLinearColor)
	GETPARAM(metallic, getFloat)
	GETPARAM(bumpScale, getFloat)
	GETPARAM(parallaxScale, getFloat)
	GETPARAM(occlusionStrength, getFloat)
	GETPARAM(emissionColor, getLinearColor)
	GETPARAM(detailMapScale, getFloat)
	GETPARAM(secondaryUv, getFloat)
#undef GETPARAM
	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = createPackage(
		name, path, assetRootPath, FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);
	if (existingMaterial){
		matIdMap.Add(id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return;
	}

	bool isTransparentQueue = (renderQueue >= 3000) && (renderQueue < 4000);
	bool isAlphaTestQueue = (renderQueue >= 2450) && (renderQueue < 3000);
	bool isGeomQueue = (!isTransparentQueue && !isAlphaTestQueue) || 
		((renderQueue >= 2000) && (renderQueue < 2450));

	logValue("Transparent queue", isTransparentQueue);
	logValue("Alpha test queue", isAlphaTestQueue);
	logValue("Geom queue", isGeomQueue);

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//albedo
	UE_LOG(JsonLog, Log, TEXT("Creating albedo"));
	UMaterialExpressionTextureSample *albedoTexExpression = 0;
	UMaterialExpressionVectorParameter *albedoColorExpression = 0;
	createMaterialInputMultiply(material, mainTexture, &color, material->BaseColor, &albedoTexExpression, &albedoColorExpression);
	if (useNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Creating normal map"));
		createMaterialInput(material, normalMapTex, nullptr, material->Normal, true);
	}

	UE_LOG(JsonLog, Log, TEXT("Creating specular"));//TODO: connect specular alpha to smoothness

	UMaterialExpressionTextureSample *specTexExpression = 0;//, *metalTexExpression = 0; no metallic textures, huh.

	createMaterialInput(material, specularTex, &specularColor, material->Specular, false, &specTexExpression);

	createMaterialInput(material, occlusionTex, nullptr, material->AmbientOcclusion, false);

	useEmission = useEmission || (emissionTex >= 0);
	if (useEmission){
		UE_LOG(JsonLog, Log, TEXT("Creating emissive"));

		UMaterialExpressionTextureSample *emissiveTexExp = 0;

		createMaterialInputMultiply(material, emissionTex, &emissionColor, material->EmissiveColor);
		material->bUseEmissiveForDynamicAreaLighting = true;
		/*
		if (emissionTex >= 0){
			emissiveTexExp = createTextureExpression(material, emissionTex, false);
		}
		auto emissiveColorExp = createVectorExpression(material, emissionColor);

		if (emissiveTexExp){
			auto mulExp = createExpression<UMaterialExpressionMultiply>(material);

			mulExp->A.Expression = emissiveTexExp;
			mulExp->B.Expression = emissiveTexExp;
			material->EmissiveColor.Expression = mulExp;
		}
		else
			material->EmissiveColor.Expression = emissiveColorExp;
		*/
		emissiveMaterials.Add(matId);

		//createMaterialInput(material, emissionTex, &emissionColor, material->EmissiveColor, false);
	}

	if (useMetallic){
		UE_LOG(JsonLog, Log, TEXT("Creating metallic value"));
		createMaterialSingleInput(material, metallic, material->Metallic);
	}

	//UMaterialExpressionSubtract *smoothToRough = NewObject<UMaterialExpressionSubtract>(material);
	/*
	UMaterialExpressionSubtract *smoothToRough = NewObject<UMaterialExpressionSubtract>(material);
	material->Expressions.Add(smoothToRough);
	material->Roughness.Expression = smoothToRough;
	smoothToRough->ConstA = 1.0f;*/

	UE_LOG(JsonLog, Log, TEXT("specularMode:%d"), (int)(useSpecular));
	UE_LOG(JsonLog, Log, TEXT("specTex exiss:%d"), (int)(specTexExpression != nullptr));
	if (specTexExpression)
		UE_LOG(JsonLog, Log, TEXT("num outputs: %d"), specTexExpression->Outputs.Num());

	//if (useSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	//useSpecular is false? the heck..
	//auto tmpNode1 = NewObject<UMaterialExpressionAdd>(material);
	//material->Expressions.Add(tmpNode1);
	//auto tmpNode2 = NewObject<UMaterialExpressionSubtract>(material);
	//material->Expressions.Add(tmpNode2);

	if ((specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
		auto mulNode = NewObject<UMaterialExpressionMultiply>(material);
		material->Expressions.Add(mulNode);
		auto addNode = NewObject<UMaterialExpressionAdd>(material);
		material->Expressions.Add(addNode);
		addNode->ConstA = 1.0f;
		addNode->B.Expression = mulNode;
		mulNode->ConstA = -1.0f;

		specTexExpression->ConnectExpression(&mulNode->B, 4);
		material->Roughness.Expression = addNode;
	}
	else{
		//material->Roughness.Expression = addNode;
		createMaterialSingleInput(material, 1.0f - smoothness, material->Roughness);
	}

	if (isTransparentQueue)
		material->BlendMode = BLEND_Translucent;
	if (isAlphaTestQueue)
		material->BlendMode = BLEND_Masked;
	if (isGeomQueue)
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (isTransparentQueue || isAlphaTestQueue) && !isGeomQueue;
	if (needsOpacity){
		auto &opacityTarget = isTransparentQueue ? material->Opacity: material->OpacityMask;

		if (albedoTexExpression && albedoColorExpression){
			auto opacityMul = createExpression<UMaterialExpressionMultiply>(material);
			albedoTexExpression->ConnectExpression(&opacityMul->A, 4);
			albedoColorExpression->ConnectExpression(&opacityMul->B, 4);
			opacityTarget.Expression = opacityMul;
		}else if (albedoTexExpression != 0)
			albedoTexExpression->ConnectExpression(&opacityTarget, 4);
		else if (albedoColorExpression != 0)
			albedoColorExpression->ConnectExpression(&opacityTarget, 4);
		else{
			UE_LOG(JsonLog, Warning, TEXT("Could not find matchin opacity source in material %s"), *name);
			//createMaterialSingleInput(material, alphaCutoff, opacityTarget);
		}
	}

	auto numExpressions = material->Expressions.Num();
	int expressionRows = (int)(sqrtf((float)numExpressions))+1;
	if (expressionRows != 0){
			for (int i = 0; i < numExpressions; i++){
					auto cur = material->Expressions[i];
					auto row = i / expressionRows;
					auto col = i % expressionRows;

					int32 size = 256;

					int32 x = (col - expressionRows) * size;
					int32 y = row * size;

					cur->MaterialExpressionEditorX = x;
					cur->MaterialExpressionEditorY = y;
			}
	}
	
	material->PreEditChange(0);
	material->PostEditChange();

	if (material){
		matIdMap.Add(id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();
}


void JsonImporter::importTexture(JsonObjPtr obj, const FString &rootPath){
	auto filename = getString(obj, "path");
	auto id = getInt(obj, "id");
	auto name = getString(obj, "name");
	auto filter = getString(obj, "filterMode");
	auto mipBias = getFloat(obj, "mipMapBias");
	auto width = getInt(obj, "width");
	auto height = getInt(obj, "height");
	auto wrapMode = getString(obj, "wrapMode");
	//isTex2D, isRenderTarget, alphaTransparency, anisoLevel

	UE_LOG(JsonLog, Log, TEXT("Texture: %s, %s, %d x %d"), *filename, *name, width, height);

	bool isNormalMap = name.EndsWith(FString("_n")) || name.EndsWith(FString("Normals"));
	if (isNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Texture recognized as normalmap: %s(%s)"), *name, *filename);
	}

	UTexture* existingTexture = 0;
	//auto fileSystemPath = FPaths::Combine(*rootPath, *filename);
	FString ext = FPaths::GetExtension(filename);
	UE_LOG(JsonLog, Log, TEXT("filename: %s, ext: %s, assetRootPath: %s"), *filename, *ext, *rootPath);

	FString textureName;
	FString packageName;
	UPackage *texturePackage = createPackage(name, filename, rootPath, FString("Texture"), 
		&packageName, &textureName, &existingTexture);

	if (existingTexture){
		texIdMap.Add(id, existingTexture->GetPathName());
		UE_LOG(JsonLog, Warning, TEXT("Texutre %s already exists, package %s"), *textureName, *packageName);
		return;
	}

	TArray<uint8> binaryData;

	FString fileSystemPath = FPaths::Combine(*assetRootPath, *filename);

	if (ext.ToLower() == FString("tif")){
		UE_LOG(JsonLog, Warning, TEXT("TIF image extension found! Fixing it to png: %s. Image will fail to load if no png file is present."), *fileSystemPath);
		ext = FString("png");
		
		FString pathPart, namePart, extPart;
		FPaths::Split(fileSystemPath, pathPart, namePart, extPart);
		FString newBaseName = FString::Printf(TEXT("%s.%s"), *namePart, *ext);
		fileSystemPath = FPaths::Combine(*pathPart, *newBaseName);
		UE_LOG(JsonLog, Warning, TEXT("New path: %s"), *fileSystemPath);
	}

	if (!FFileHelper::LoadFileToArray(binaryData, *fileSystemPath)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load texture %s(%s)"), *name, *filename);
		return;
	}

	if (binaryData.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No binary data: %s"), *name);
		return;
	}

	UE_LOG(JsonLog, Log, TEXT("Loading tex data: %s (%d bytes)"), *name, binaryData.Num());
	auto texFab = NewObject<UTextureFactory>();
	texFab->AddToRoot();
	texFab->SuppressImportOverwriteDialog();
	const uint8* data = binaryData.GetData();

	if (isNormalMap){
		texFab->LODGroup = TEXTUREGROUP_WorldNormalMap;
		texFab->CompressionSettings = TC_Normalmap;
	}

	UE_LOG(JsonLog, Log, TEXT("Attempting to create package: texName %s"), *name);
	UTexture *unrealTexture = (UTexture*)texFab->FactoryCreateBinary(
		UTexture2D::StaticClass(), texturePackage, *textureName, RF_Standalone|RF_Public, 0, *ext, data, data + binaryData.Num(), GWarn);

	if (unrealTexture){
		texIdMap.Add(id, unrealTexture->GetPathName());
		FAssetRegistryModule::AssetCreated(unrealTexture);
		texturePackage->SetDirtyFlag(true);
	}
	texFab->RemoveFromRoot();
}

void JsonImporter::importScene(const FString& filename){
	assetRootPath = FPaths::GetPath(filename);
	sourceBaseName = FPaths::GetBaseFilename(filename);

	FString jsonString;
	if (!FFileHelper::LoadFileToString(jsonString, *filename)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load file %s"), *filename);
		return;
	}

	UE_LOG(JsonLog, Log, TEXT("Loaded file %s"), *filename);
	JsonReaderRef reader = TJsonReaderFactory<>::Create(jsonString);

	JsonObjPtr jsonData = MakeShareable(new FJsonObject());
	if (!FJsonSerializer::Deserialize(reader, jsonData)){
		UE_LOG(JsonLog, Warning, TEXT("Could not parse file %s"), *filename);
		return;
	}

	const JsonValPtrs *resources = 0, *objects = 0, *textures = 0, *materials = 0, *meshes = 0;

	loadArray(jsonData, resources, TEXT("resources"), TEXT("Resources"));
	loadArray(jsonData, objects, TEXT("objects"), TEXT("Objects"));
	loadArray(jsonData, textures, TEXT("textures"), TEXT("Textures"));
	loadArray(jsonData, materials, TEXT("materials"), TEXT("Materials"));
	loadArray(jsonData, meshes, TEXT("meshes"), TEXT("Meshes"));

	assetCommonPath = findCommonPath(resources);

	if (textures){
		FScopedSlowTask texProgress(textures->Num(), LOCTEXT("Importing textures", "Importing textures"));
		texProgress.MakeDialog();
		UE_LOG(JsonLog, Log, TEXT("Processing textures"));
		for(auto cur: *textures){
			auto obj = cur->AsObject();
			if (!obj.IsValid())
				continue;
			importTexture(obj, assetRootPath);
			texProgress.EnterProgressFrame(1.0f);
		}
	}

	if (materials){
		FScopedSlowTask matProgress(materials->Num(), LOCTEXT("Importing materials", "Importing materials"));
		matProgress.MakeDialog();
		UE_LOG(JsonLog, Log, TEXT("Processing materials"));
		int32 matId = 0;
		for(auto cur: *materials){
			auto obj = cur->AsObject();
			auto curId = matId;
			matId++;
			if (!obj.IsValid())
				continue;
			importMaterial(obj, curId);
			matProgress.EnterProgressFrame(1.0f);
		}
	}

	if (meshes){
		FScopedSlowTask meshProgress(materials->Num(), LOCTEXT("Importing materials", "Importing meshes"));
		meshProgress.MakeDialog();
		UE_LOG(JsonLog, Log, TEXT("Processing meshes"));
		int32 meshId = 0;
		for(auto cur: *meshes){
			auto obj = cur->AsObject();
			auto curId = meshId;
			meshId++;
			if (!obj.IsValid())
				continue;
			importMesh(obj, curId);
			meshProgress.EnterProgressFrame(1.0f);
		}
	}

	if (objects){
		FScopedSlowTask objProgress(objects->Num(), LOCTEXT("Importing objects", "Importing objects"));
		objProgress.MakeDialog();
		UE_LOG(JsonLog, Log, TEXT("Import objects"));
		int32 objId = 0;
		for(auto cur: *objects){
			auto obj = cur->AsObject();
			auto curId = objId;
			objId++;
			if (!obj.IsValid())
				continue;
			importObject(obj, objId);
			objProgress.EnterProgressFrame(1.0f);
		}
	}
}


void FJsonImportModule::StartupModule(){
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FJsonImportStyle::Initialize();
	FJsonImportStyle::ReloadTextures();

	FJsonImportCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FJsonImportModule::ShutdownModule(){
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FJsonImportStyle::Shutdown();

	FJsonImportCommands::Unregister();
}


void FJsonImportModule::PluginButtonClicked(){
	// Put your "OnButtonClicked" stuff here
	FStringArray files;

	if (!FDesktopPlatformModule::Get()->OpenFileDialog(0,
			FString("Open JSON File"), FString(), FString(), FString("JSON file|*.json"), EFileDialogFlags::None, files))
			return;

	if (!files.Num()){
			UE_LOG(JsonLog, Log, TEXT("No file to open"));
			return;
	}

	auto filename = FPaths::ConvertRelativePathToFull(files[0]);
	JsonImporter importer;
	importer.importScene(filename);

	/*
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FJsonImportModule::PluginButtonClicked()")),
							FText::FromString(TEXT("JsonImport.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);*/
}

void FJsonImportModule::AddMenuExtension(FMenuBuilder& Builder){
	Builder.AddMenuEntry(FJsonImportCommands::Get().PluginAction);
}

void FJsonImportModule::AddToolbarExtension(FToolBarBuilder& Builder){
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJsonImportModule, JsonImport)