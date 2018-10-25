#pragma once
#include "JsonTypes.h"

namespace JsonObjects{
	template<typename T> TArray<T> getJsonObjArray(JsonObjPtr jsonData, const char* name){
		TArray<T> result;
		getJsonObjArray<T>(jsonData, result, name);
		return result;
	}

	template<typename T> void getJsonObjArray(JsonObjPtr jsonData, TArray<T>& result, const char* name){
		auto arrayVal = jsonData->GetArrayField(name);

		if (arrayVal.Num() <= 0)
			return;
		
		for (int i = 0; i < arrayVal.Num(); i++){
			auto jsonVal = arrayVal[i];
			auto jsonObj = jsonVal->AsObject();
			if (!jsonObj.IsValid()){	
				UE_LOG(JsonLog, Warning, TEXT("Could not retrieve index %d from \"%s\""), i, name);
				continue;
			}
			result.Add(T(jsonObj));
		}
	}

	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name, const FString &warnName);
	void loadArray(JsonObjPtr data, const JsonValPtrs *&valPtrs, const FString &name);

	int32 getInt(JsonObjPtr data, const char* name);
	bool getBool(JsonObjPtr data, const char* name);
	float getFloat(JsonObjPtr data, const char* name);
	FString getString(JsonObjPtr data, const char* name);
	JsonObjPtr getObject(JsonObjPtr data, const char* name);

	IntArray getIntArray(JsonObjPtr, const char *name);

	TArray<float> toFloatArray(const JsonValPtrs &inData){
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
	
	TArray<float> toFloatArray(const JsonValPtrs* inData){
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
	
	TArray<int32> toIntArray(const JsonValPtrs &inData){
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
	
	void getJsonValue(FLinearColor& outValue, JsonObjPtr data, const char*name);
	void getJsonValue(FMatrix& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector2D& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FVector4& outValue, JsonObjPtr data, const char *name);
	void getJsonValue(FQuat& outValue, JsonObjPtr data, const char *name);

	void getJsonValue(int& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(bool& outValue, JsonObjPtr data, const char* name);
	void getJsonValue(float &outValue, JsonObjPtr data, const char* name);
	void getJsonValue(FString &outValue, JsonObjPtr data, const char* name);
	//JsonObjPtr getObject(JsonObjPtr data, const char* name);

	FLinearColor getLinearColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());
	FMatrix getMatrix(JsonObjPtr data, const char* name, const FMatrix &defaultVal = FMatrix::Identity);
	FVector2D getVector2(JsonObjPtr data, const char* name, const FVector2D &defaultVal = FVector2D());
	FVector getVector(JsonObjPtr data, const char* name, const FVector &defaultVal = FVector());
	FVector4 getVector4(JsonObjPtr data, const char* name, const FVector4 &defaultVal = FVector4());
	FLinearColor getColor(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());

	FLinearColor getGammaColorAsLinear(JsonObjPtr data, const char* name, const FLinearColor &defaultVal = FLinearColor());

	FQuat getQuat(JsonObjPtr data, const char* name, const FQuat &defaultVal = FQuat());
}
