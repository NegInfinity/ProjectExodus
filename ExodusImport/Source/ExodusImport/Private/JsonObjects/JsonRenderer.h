#pragma once
#include "JsonTypes.h"

class JsonRenderer{
public:
	int lightmapIndex = -1;
	FString shadowCastingMode;
	//FVector4 lightmapScaleOffset;
	TArray<int32> materials;
	bool receiveShadows;

	bool castsOneSidedShadows() const{
		return shadowCastingMode == FString("On");
	}

	bool castsTwoSidedShadows() const{
		return shadowCastingMode == FString("TwoSided");
	}

	bool castsShadowsOnly() const{
		return shadowCastingMode == FString("ShadowsOnly");
	}

	bool castsShadows() const{
		return castsOneSidedShadows() || castsTwoSidedShadows() || castsShadowsOnly();
	}

	bool hasMaterials() const{
		return materials.Num() > 0;
	}

	void load(JsonObjPtr jsonData);
	JsonRenderer() = default;
	JsonRenderer(JsonObjPtr jsonData){
		load(jsonData);
	}
};

