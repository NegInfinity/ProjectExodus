#include "JsonImportPrivatePCH.h"
#include "loggers.h"
#include "JsonLog.h"

void JsonObjects::logValue(const FString &msg, const LinearColorArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: LinearColorArray[%d]{...}"), *msg, arr.Num());
}

void JsonObjects::logValue(const FString &msg, const IntArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: IntArray[%d]{...}"), *msg, arr.Num());
}
	
void JsonObjects::logValue(const FString &msg, const FloatArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: FloatArray[%d]{...}"), *msg, arr.Num());
}

void JsonObjects::logValue(const FString &msg, const StringArray &arr){
	UE_LOG(JsonLog, Log, TEXT("%s: StringArray[%d]{...}"), *msg, arr.Num());
}
	
void JsonObjects::logValue(const FString &msg, const bool val){
	FString str = val ? TEXT("true"): TEXT("false");
	//const char *str = val ? "true": "false";
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *str);
}

void JsonObjects::logValue(const FString &msg, const FVector2D &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f"), *msg, val.X, val.Y);
}

void JsonObjects::logValue(const FString &msg, const FVector &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f"), *msg, val.X, val.Y, val.Z);
}

void JsonObjects::logValue(const FString &msg, const FQuat &val){
	UE_LOG(JsonLog, Log, TEXT("%s: x %f, y %f, z %f, w %f"), *msg, val.X, val.Y, val.Z, val.W);
}

void JsonObjects::logValue(const FString &msg, const FMatrix &val){
	UE_LOG(JsonLog, Log, TEXT("%s:"), *msg);
	for(int i = 0; i < 4; i++)
		UE_LOG(JsonLog, Log, TEXT("    %f %f %f %f"), val.M[i][0], val.M[i][1], val.M[i][2], val.M[i][3]);
}

void JsonObjects::logValue(const FString &msg, const int val){
	UE_LOG(JsonLog, Log, TEXT("%s: %d"), *msg, val);
}

void JsonObjects::logValue(const FString &msg, const float val){
	UE_LOG(JsonLog, Log, TEXT("%s: %f"), *msg, val);
}

void JsonObjects::logValue(const FString &msg, const FString &val){
	UE_LOG(JsonLog, Log, TEXT("%s: %s"), *msg, *val);
}

void JsonObjects::logValue(const FString &msg, const FLinearColor &val){
	UE_LOG(JsonLog, Log, TEXT("%s: r %f, g %f, b %f, a %f"), *msg, val.R, val.G, val.B, val.A);
}

