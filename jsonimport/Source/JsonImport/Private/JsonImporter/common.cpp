#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

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

void JsonImporter::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *warnName);
	}
}

void JsonImporter::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *name);
	}
}

void JsonImporter::logValue(const FString &msg, const bool val){
	FString str = val ? TEXT("true"): TEXT("false");
	//const char *str = val ? "true": "false";
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *str);
}

FVector JsonImporter::unityToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

void JsonImporter::logValue(const FString &msg, const FVector2D &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f"), *msg, val.X, val.Y);
}

void JsonImporter::logValue(const FString &msg, const FVector &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f"), *msg, val.X, val.Y, val.Z);
}

void JsonImporter::logValue(const FString &msg, const FQuat &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f, w %f"), *msg, val.X, val.Y, val.Z, val.W);
}

void JsonImporter::logValue(const FString &msg, const FMatrix &val){
	UE_LOG(JsonLog, Log, TEXT("%s:"), *msg);
	for(int i = 0; i < 4; i++)
		UE_LOG(JsonLog, Log, TEXT("    %f %f %f %f"), val.M[i][0], val.M[i][1], val.M[i][2], val.M[i][3]);
}

void JsonImporter::logValue(const FString &msg, const int val){
	UE_LOG(JsonLog, Log, TEXT("%s: %d"), *msg, val);
}

void JsonImporter::logValue(const FString &msg, const float val){
	UE_LOG(JsonLog, Log, TEXT("%s: %f"), *msg, val);
}

void JsonImporter::logValue(const FString &msg, const FString &val){
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *val);
}

void JsonImporter::logValue(const FString &msg, const FLinearColor &val){
	UE_LOG(JsonLog, Log, TEXT("%s: r %f, g %f, b %f, a %f"), *msg, val.R, val.G, val.B, val.A);
}

int32 JsonImporter::getInt(JsonObjPtr data, const char* name){
	return data->GetIntegerField(FString(name));
}

bool JsonImporter::getBool(JsonObjPtr data, const char* name){
	return data->GetBoolField(FString(name));
}

float JsonImporter::getFloat(JsonObjPtr data, const char* name){
	return data->GetNumberField(FString(name));
}

FString JsonImporter::getString(JsonObjPtr data, const char* name){
	return data->GetStringField(FString(name));
}

JsonObjPtr JsonImporter::getObject(JsonObjPtr data, const char* name){
	if (!data || !data->HasField(name))
		return 0;
	auto result = data->GetObjectField(FString(name));
	return result;
	//if (!result || !result->IsvV
}

FLinearColor JsonImporter::getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
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

FMatrix JsonImporter::getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal){
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

FVector2D JsonImporter::getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal){
	FVector2D result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");

	return result;
}

FVector JsonImporter::getVector(JsonObjPtr data, const char* name, const FVector &defaultVal){
	FVector result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");

	return result;
}

FLinearColor JsonImporter::getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
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

FQuat JsonImporter::getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal){
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

int JsonImporter::findMatchingLength(const FString& arg1, const FString& arg2){
	int result = 0;
	for(; (result < arg1.Len()) && (result < arg2.Len()); result++){
		if (arg1[result] != arg2[result])
			break;
	} 
	return result;
}

FString JsonImporter::findCommonPath(const JsonValPtrs* resources){
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

