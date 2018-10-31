#pragma once

#include "JsonTypes.h"

namespace JsonObjects{
	FVector unityToUe(const FVector& arg);
	FVector unityVecToUe(const FVector& arg);
	FVector unityPosToUe(const FVector& arg);
	FVector unitySizeToUe(const FVector& arg);
	FMatrix unityWorldToUe(const FMatrix &unityMatrix);
}
