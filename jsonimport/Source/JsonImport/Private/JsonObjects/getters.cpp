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

FLinearColor JsonObjects::toLinearColor(const FJsonValue &data, const FLinearColor &defaultVal){
	auto objPtr = data.AsObject();
	return toLinearColor(objPtr);
}

FMatrix JsonObjects::toMatrix(const FJsonValue &data, const FMatrix &defaultVal){
	auto objPtr = data.AsObject();
	return toMatrix(objPtr);
}

FMatrix JsonObjects::toMatrix(JsonObjPtr data, const FMatrix &defaultVal){
	if (!data.IsValid())
			return defaultVal;
	FMatrix result;

	result.M[0][0] = getFloat(data, "e00");
	result.M[0][1] = getFloat(data, "e10");
	result.M[0][2] = getFloat(data, "e20");
	result.M[0][3] = getFloat(data, "e30");
	result.M[1][0] = getFloat(data, "e01");
	result.M[1][1] = getFloat(data, "e11");
	result.M[1][2] = getFloat(data, "e21");
	result.M[1][3] = getFloat(data, "e31");
	result.M[2][0] = getFloat(data, "e02");
	result.M[2][1] = getFloat(data, "e12");
	result.M[2][2] = getFloat(data, "e22");
	result.M[2][3] = getFloat(data, "e32");
	result.M[3][0] = getFloat(data, "e03");
	result.M[3][1] = getFloat(data, "e13");
	result.M[3][2] = getFloat(data, "e23");
	result.M[3][3] = getFloat(data, "e33");
		
	return result;
}



FLinearColor JsonObjects::toLinearColor(JsonObjPtr data, const FLinearColor &defaultVal){
	FLinearColor result = defaultVal;
	if (!data.IsValid())
			return result;
	
	result.A = getFloat(data, "a");
	result.R = getFloat(data, "r");
	result.G = getFloat(data, "g");
	result.B = getFloat(data, "b");

	return result;
}

FLinearColor JsonObjects::getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal){
	FLinearColor result = defaultVal;

	auto colorObj = getObject(data, name);
	return toLinearColor(colorObj, defaultVal);
}

FMatrix JsonObjects::getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal){
	FMatrix result = defaultVal;
	auto matObj = getObject(data, name);
	if (!matObj.IsValid())
			return result;

	return toMatrix(matObj);
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

IntArray JsonObjects::getIntArray(JsonObjPtr jsonObj, const char *name, bool optional){
	if (optional){
		if (!jsonObj->HasField(name))
			return IntArray();
	}
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toIntArray(*arrValues);
	return IntArray();
}

ByteArray JsonObjects::getByteArray(JsonObjPtr jsonObj, const char *name, bool optional){
	if (optional){
		if (!jsonObj->HasField(name))
			return ByteArray();
	}
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toByteArray(*arrValues);
	return ByteArray();
}

FloatArray JsonObjects::getFloatArray(JsonObjPtr jsonObj, const char *name, bool optional){
	if (optional){
		if (!jsonObj->HasField(name))
			return FloatArray();
	}
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toFloatArray(*arrValues);
	return FloatArray();
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

void JsonObjects::getJsonValue(StringArray &outValue, JsonObjPtr data, const char* name){
	outValue = getStringArray(data, name);
}

void JsonObjects::getJsonValue(FloatArray &outValue, JsonObjPtr data, const char* name){
	outValue = getFloatArray(data, name);
}


StringArray JsonObjects::toStringArray(const JsonValPtrs& inData){
	StringArray result;
	for(auto cur: inData){
		FString val;
		if (cur.IsValid()){
			cur->TryGetString(val);
		}
		result.Add(val);
	}
	return result;
}

FloatArray JsonObjects::toFloatArray(const JsonValPtrs* inData){
	FloatArray result;
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

void JsonObjects::getJsonValue(ByteArray &outValue, JsonObjPtr data, const char* name){
	outValue = getByteArray(data, name);
}

void JsonObjects::getJsonValue(LinearColorArray &outValue, JsonObjPtr data, const char* name){
	outValue = getLinearColorArray(data, name);
}

void JsonObjects::getJsonValue(MatrixArray &outValue, JsonObjPtr data, const char *name){
	outValue = getMatrixArray(data, name);
}


IntArray JsonObjects::toIntArray(const JsonValPtrs &inData){
	IntArray result;
	for(auto cur: inData){
		int32 val;
		if (cur.IsValid()){
			cur->TryGetNumber(val);
		}
		result.Add(val);
	}
	return result;
}

ByteArray JsonObjects::toByteArray(const JsonValPtrs &inData){
	ByteArray result;
	for(auto cur: inData){
		uint32 val = 0;
		if (cur.IsValid()){
			cur->TryGetNumber(val);
		}
		result.Add(val);
	}
	return result;
}

StringArray JsonObjects::getStringArray(JsonObjPtr jsonObj, const char *name, bool optional){
	if (optional){
		if (!jsonObj->HasField(name))
			return StringArray();
	}
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toStringArray(*arrValues);
	return StringArray();
}

FloatArray JsonObjects::toFloatArray(const JsonValPtrs &inData){
	FloatArray result;
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

LinearColorArray JsonObjects::toLinearColorArray(const JsonValPtrs &inData){
	LinearColorArray result;
	for(auto cur: inData){
		FLinearColor val;
		if (cur.IsValid())
			val = toLinearColor(*cur);
		result.Add(val);
	}
	return result;
}

MatrixArray JsonObjects::toMatrixArray(const JsonValPtrs &inData){
	MatrixArray result;
	for(auto cur: inData){
		FMatrix val = FMatrix::Identity;
		if (cur.IsValid()){
			val = toMatrix(*cur);
		}
		result.Add(val);
	}
	return result;
}


LinearColorArray JsonObjects::getLinearColorArray(JsonObjPtr jsonObj, const char *name){
	const JsonValPtrs* arrValues = 0;
	loadArray(jsonObj, arrValues, name);
	if (arrValues)
		return toLinearColorArray(*arrValues);
	return LinearColorArray();
}

MatrixArray JsonObjects::getMatrixArray(JsonObjPtr data, const char *name, bool optional){
	if (optional){
		if (!data->HasField(name))
			return MatrixArray();
	}
	const JsonValPtrs* arrValues = 0;
	loadArray(data, arrValues, name);
	if (arrValues)
		return toMatrixArray(*arrValues);
	return MatrixArray();
}
