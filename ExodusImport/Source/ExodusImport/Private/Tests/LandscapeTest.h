#pragma once
#include "CoreMinimal.h"

class UMaterial;
class ALandscape;
class ULandscapeLayerInfoObject;

class LandscapeTest{
public:
	void run();
protected:
	static UMaterial *createLandscapeMaterial(const TArray<FString> &names);
	static ALandscape* createTestLandscape(UWorld *world, const TArray<FString> &layerNames, UMaterial *landMaterial);
	static ULandscapeLayerInfoObject* createTerrainLayerInfo(const FString &layerName, int layerIndex);
	static FLinearColor indexToColor(int index);
	static FString getTestRootPath();
};