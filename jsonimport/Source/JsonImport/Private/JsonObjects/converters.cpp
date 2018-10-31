#include "JsonImportPrivatePCH.h"
#include "converters.h"

FVector JsonObjects::unityToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

FVector JsonObjects::unityVecToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

FVector JsonObjects::unityPosToUe(const FVector& arg){
	return unityVecToUe(arg) * 100.0f;
}

FVector JsonObjects::unitySizeToUe(const FVector& arg){
	return unityVecToUe(arg) * 100.0f;
}

FMatrix JsonObjects::unityWorldToUe(const FMatrix &unityWorld){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = unityWorld.GetOrigin();
	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);
	FMatrix ueMatrix;
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);
	return ueMatrix;
}
