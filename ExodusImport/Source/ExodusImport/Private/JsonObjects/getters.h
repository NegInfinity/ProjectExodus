#pragma once
#include "JsonTypes.h"
#include <functional>

namespace JsonObjects{
	template<typename T> TArray<T> getJsonObjArray(JsonObjPtr jsonData, const char* name){
		TArray<T> result;
		getJsonObjArray<T>(jsonData, result, name);
		return result;
	}

	template<typename T> void getJsonValArray(JsonObjPtr jsonData, TArray<T>& result, const char* name, std::function<T(JsonValPtr, int)> converter, bool optional = false){
		result.Empty();
		if (optional){	
			if (!jsonData->HasField(name))
				return;
		}
		if (!jsonData->HasField(name)){
			return;
		}

		auto arrayVal = jsonData->GetArrayField(name);

		if (arrayVal.Num() <= 0)
			return;
		
		for (int i = 0; i < arrayVal.Num(); i++){
			auto jsonVal = arrayVal[i];
			if (converter){
				result.Add(converter(jsonVal, i));
			}
			else{
				result.Add(T());
			}
		}
	}

	template<typename T> void getJsonObjArray(JsonObjPtr jsonData, TArray<T>& result, const char* name, std::function<T(JsonObjPtr, int)> converter){
		getJsonValArray<T>(jsonData, result, name, 
			[&](JsonValPtr val, int idx){
				auto jsonObj = jsonVal->AsObject();
				if (!jsonObj.IsValid()){	
					UE_LOG(JsonLog, Warning, TEXT("Could not retrieve array index %d from \"%s\""), idx, name);
					return T();
				}
				if (!converter){
					UE_LOG(JsonLog, Warning, TEXT("Converter not assigned to retrieve data[%d] from \"%s\""), idx, name);
					return T();
				}
				return converter(jsonObj, idx);
			}
		);
	}

	template<typename T> void getJsonObjArray(JsonObjPtr jsonData, TArray<T>& result, const char* name, bool optional = false){
		result.Empty();
		if (optional){
			if (!jsonData->HasField(name))
				return;
		}
		auto arrayVal = jsonData->GetArrayField(name);

		if (arrayVal.Num() <= 0)
			return;
		
		for (int i = 0; i < arrayVal.Num(); i++){
			auto jsonVal = arrayVal[i];
			auto jsonObj = jsonVal->AsObject();
			if (!jsonObj.IsValid()){	
				UE_LOG(JsonLog, Warning, TEXT("Could not retrieve index %d from \"%s\""), i, name);
				result.Add(T());
				continue;
			}
			result.Add(T(jsonObj));
		}
	}

	template<typename T> void getJsonObj(JsonObjPtr jsonData, T& result, const char* name){
		if (!jsonData || !jsonData->HasField(name))
			return;
		auto obj = jsonData->GetObjectField(name);
		result = T(obj);
	}

	template<typename T> T getJsonObj(JsonObjPtr jsonData, const char* name){
		if (!jsonData || !jsonData->HasField(name))
			return T();
		
		auto obj = jsonData->GetObjectField(name);
		return T(obj);
	}

	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName);
	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name);

	int32 getInt(JsonObjPtr data, const char* name);
	ResId getResId(JsonObjPtr data, const char* name);
	bool getBool(JsonObjPtr data, const char* name);
	float getFloat(JsonObjPtr data, const char* name);
	float getStrFloat(JsonObjPtr data, const char* name);
	FString getString(JsonObjPtr data, const char* name);
	JsonObjPtr getObject(JsonObjPtr data, const char* name);

	IntArray getIntArray(JsonObjPtr data, const char *name, bool optional = false);
	ByteArray getByteArray(JsonObjPtr jsonObj, const char *name, bool optional = false);
	StringArray getStringArray(JsonObjPtr data, const char *name, bool optional = false);
	FloatArray getFloatArray(JsonObjPtr data, const char *name, bool optional = false);
	LinearColorArray getLinearColorArray(JsonObjPtr data, const char *name);
	MatrixArray getMatrixArray(JsonObjPtr data, const char *name, bool optional = false);

	//this needs cleanup
	FloatArray toFloatArray(const JsonValPtrs &inData);
	FloatArray toFloatArray(const JsonValPtrs* inData);//two versions?
	IntArray toIntArray(const JsonValPtrs &inData);
	ByteArray toByteArray(const JsonValPtrs &inData);
	StringArray toStringArray(const JsonValPtrs &inData);
	LinearColorArray toLinearColorArray(const JsonValPtrs &inData);
	MatrixArray toMatrixArray(const JsonValPtrs &inData);

	FLinearColor toLinearColor(JsonObjPtr data, const FLinearColor &defaultVal = FLinearColor());
	FLinearColor toLinearColor(const FJsonValue &data, const FLinearColor &defaultVal = FLinearColor());
	FMatrix toMatrix(JsonObjPtr data, const FMatrix &defaultVal = FMatrix());
	FMatrix toMatrix(const FJsonValue &data, const FMatrix &defaultVal = FMatrix());
	
	void getJsonValue(FLinearColor& outValue, JsonObjPtr data, const char*name);
	void getJsonValue(FMatrix& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector2D& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector4& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FQuat& outValue, JsonObjPtr data, const char *name);


	void getJsonValue(int& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(ResId& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(bool& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(float &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(FString &outValue, JsonObjPtr data, const char* name);
	//JsonObjPtr getObject(JsonObjPtr data, const char* name);
	void getJsonValue(IntArray &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(StringArray &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(FloatArray &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(LinearColorArray &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(ByteArray &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(MatrixArray &outValue, JsonObjPtr data, const char *name);

	void getJsonValue(FColor& outValue, JsonObjPtr data, const char *name);
	FColor getRgbColor(JsonObjPtr data, const char *name, const FColor &defaultVal = FColor());

	FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity);
	FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D());
	FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector());
	FVector4 getVector4(JsonObjPtr data, const char* name, const FVector4 &defaultVal = FVector4());
	FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());

	FLinearColor getGammaColorAsLinear(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());

	FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat());
}
