#pragma once
#include "JsonTypes.h"

class JsonRenderer{
public:
	int lightmapIndex = -1;
	FString shadowCastingMode;
	//FVector4 lightmapScaleOffset;
	TArray<int32> materials;
	bool receiveShadows;

	bool hasMaterials() const{
		return materials.Num() > 0;
	}

	void load(JsonObjPtr jsonData);
	JsonRenderer() = default;
	JsonRenderer(JsonObjPtr jsonData){
		load(jsonData);
	}
};

