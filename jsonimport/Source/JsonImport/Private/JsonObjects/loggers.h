#pragma once
#include "JsonTypes.h"

namespace JsonObjects{
	void logValue(const FString &msg, const IntArray &arr);
	void logValue(const FString &msg, const FloatArray &arr);
	void logValue(const FString &msg, const LinearColorArray &arr);
	void logValue(const FString &msg, const ByteArray &val);
	void logValue(const FString &msg, const bool val);
	void logValue(const FString &msg, const FVector2D &val);
	void logValue(const FString &msg, const FVector &val);
	void logValue(const FString &msg, const FQuat &val);
	void logValue(const FString &msg, const FMatrix &val);
	void logValue(const FString &msg, const int val);
	void logValue(const FString &msg, const float val);
	void logValue(const FString &msg, const FString &val);
	void logValue(const FString &msg, const FLinearColor &val);
	void logValue(const FString &msg, const StringArray &val);
	void logValue(const FString &msg, const MatrixArray &val);
}