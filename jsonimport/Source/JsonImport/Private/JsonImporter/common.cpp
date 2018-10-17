#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "DesktopPlatformModule.h"

int JsonImporter::findMatchingLength(const FString& arg1, const FString& arg2){
	int result = 0;
	for(; (result < arg1.Len()) && (result < arg2.Len()); result++){
		if (arg1[result] != arg2[result])
			break;
	} 
	return result;
}

FString JsonImporter::findCommonPath(const JsonValPtrs* resources){
	FString result;
	if (!resources)
		return result;
	UE_LOG(JsonLog, Log, TEXT("Listing files"));
	bool first = true;
	int minCommonLength = 0;

	for(auto cur: *resources){
		auto resPath = cur->AsString();
		UE_LOG(JsonLog, Log, TEXT("Resource %s is found"), *resPath);
		auto curPath = FPaths::GetPath(resPath);
		auto curLen = curPath.Len();
		auto ext = FPaths::GetExtension(resPath);
		if (!ext.Len()){
			UE_LOG(JsonLog, Warning, TEXT("Invalid ext for candidate %s"), *resPath);
			continue;
		}
		if (first){
			result = curPath;
			minCommonLength = curLen;
			first = false;
			continue;
		}

		auto commonLen = findMatchingLength(curPath, result);
		if (commonLen < minCommonLength){
			minCommonLength = commonLen;
			result = FString(minCommonLength, *curPath);
		}
		if (commonLen == 0)
			return FString();
	}
	return result;
}

