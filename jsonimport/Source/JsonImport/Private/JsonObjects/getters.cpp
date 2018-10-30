#include "JsonImportPrivatePCH.h"
#include "getters.h"

void JsonObjects::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *warnName);
	}
}

void JsonObjects::loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name){
	if (!data->TryGetArrayField(name, valPtrs)){
		UE_LOG(JsonLog, Warning, TEXT("Could not get val array %s"), *name);
	}
}

int32 JsonObjects::getInt(JsonObjPtr data, const char* name){
	return data->GetIntegerField(FString(name));
}

bool JsonObjects::getBool(JsonObjPtr data, const char* name){
	return data->GetBoolField(FString(name));
}

float JsonObjects::getFloat(JsonObjPtr data, const char* name){
	return data->GetNumberField(FString(name));
}

FString JsonObjects::getString(JsonObjPtr data, const char* name){
	return data->GetStringField(FString(name));
}

JsonObjPtr JsonObjects::getObject(JsonObjPtr data, const char* name){
	if (!data || !data->HasField(name))
		return 0;
	auto result = data->GetObjectField(FString(name));
	return result;
	//if (!result || !result->IsvV
}

FLinearColor JsonObjects::getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
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

FMatrix JsonObjects::getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal){
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

FVector4 JsonObjects::getVector4(JsonObjPtr data, const char* name, const FVector4 &defaultVal){
	FVector4 result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");
	result.W = getFloat(vecObj, "W");

	return result;
}

FVector2D JsonObjects::getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal){
	FVector2D result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");

	return result;
}

FVector JsonObjects::getVector(JsonObjPtr data, const char* name, const FVector &defaultVal){
	FVector result = defaultVal;

	auto vecObj = getObject(data, name);
	if (!vecObj.IsValid())
			return result;
	result.X = getFloat(vecObj, "X");
	result.Y = getFloat(vecObj, "Y");
	result.Z = getFloat(vecObj, "Z");

	return result;
}

FLinearColor JsonObjects::getGammaColorAsLinear(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
	auto result = getColor(data, name, defaultVal);
	const float gammaVal = 2.2f;
	result.R = powf(result.R, gammaVal);
	result.G = powf(result.G, gammaVal);
	result.B = powf(result.B, gammaVal);

	return result;
}

FLinearColor JsonObjects::getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
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

FQuat JsonObjects::getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal){
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

IntArray JsonObjects::getIntArray(JsonObjPtr jsonObj, const char *name){
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toIntArray(*arrValues);
	return IntArray();
}

void JsonObjects::getJsonValue(FLinearColor& outValue, JsonObjPtr data, const char*name){
	outValue = getLinearColor(data, name);
}

void JsonObjects::getJsonValue(FMatrix& outValue, JsonObjPtr data, const char *name){
	outValue = getMatrix(data, name);
}

void JsonObjects::getJsonValue(FVector2D& outValue, JsonObjPtr data, const char *name){
	outValue = getVector2(data, name);
}

void JsonObjects::getJsonValue(FVector& outValue, JsonObjPtr data, const char *name){
	outValue = getVector(data, name);
}

void JsonObjects::getJsonValue(FVector4& outValue, JsonObjPtr data, const char *name){
	outValue = getVector4(data, name);
}

void JsonObjects::getJsonValue(FQuat& outValue, JsonObjPtr data, const char *name){
	outValue = getQuat(data, name);
}

void JsonObjects::getJsonValue(int& outValue, JsonObjPtr data, const char* name){
	outValue = getInt(data, name);
}

void JsonObjects::getJsonValue(bool& outValue, JsonObjPtr data, const char* name){
	outValue = getBool(data, name);
}

void JsonObjects::getJsonValue(float &outValue, JsonObjPtr data, const char* name){
	outValue = getFloat(data, name);
}

void JsonObjects::getJsonValue(FString &outValue, JsonObjPtr data, const char* name){
	outValue = getString(data, name);
}

void JsonObjects::getJsonValue(FColor& outValue, JsonObjPtr data, const char *name){
	outValue = getRgbColor(data, name);
}

FColor JsonObjects::getRgbColor(JsonObjPtr data, const char *name, const FColor &defaultVal){
	FColor result = defaultVal;

	auto colorObj = getObject(data, name);
	if (!colorObj.IsValid())
			return result;
	result.A = getInt(colorObj, "a");
	result.R = getInt(colorObj, "r");
	result.G = getInt(colorObj, "g");
	result.B = getInt(colorObj, "b");

	return result;
}

void JsonObjects::getJsonValue(IntArray &outValue, JsonObjPtr data, const char* name){
	outValue = getIntArray(data, name);
}
