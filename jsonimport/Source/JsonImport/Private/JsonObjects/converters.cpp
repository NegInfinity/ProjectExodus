#include "JsonImportPrivatePCH.h"
#include "converters.h"

FVector JsonObjects::unityToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
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
