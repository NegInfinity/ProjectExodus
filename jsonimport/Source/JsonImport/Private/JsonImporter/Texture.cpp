#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextureCube.h"
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
#include "UnrealUtilities.h"

#include "DesktopPlatformModule.h"
#include "JsonObjects.h"

using namespace UnrealUtilities;

UTextureCube* JsonImporter::getCubemap(int32 id) const{
	if (id < 0)
		return 0;
	return loadCubemap(id);
}

UTextureCube* JsonImporter::loadCubemap(int32 id) const{
	return staticLoadResourceById<UTextureCube>(cubeIdMap, id, TEXT("cubemap"));
}

void JsonImporter::importCubemap(JsonObjPtr data, const FString &rootPath){
}

UTexture* JsonImporter::getTexture(int32 id) const{
	if (id < 0)
		return 0;
	return loadTexture(id);
}

UTexture* JsonImporter::loadTexture(int32 id) const{
	return staticLoadResourceById<UTexture>(texIdMap, id, TEXT("texture"));
}

void JsonImporter::importTexture(JsonObjPtr obj, const FString &rootPath){
	JsonTexture jsonTex(obj);

	/*
	auto filename = getString(obj, "path");
	auto id = getInt(obj, "id");
	auto name = getString(obj, "name");
	auto filter = getString(obj, "filterMode");
	auto mipBias = getFloat(obj, "mipMapBias");
	auto width = getInt(obj, "width");
	auto height = getInt(obj, "height");
	auto wrapMode = getString(obj, "wrapMode");
	auto sRGB = getBool(obj, "sRGB");
	auto normalMapFlag = getBool(obj, "normalMapFlag");
	auto importDataFound = getBool(obj, "importDataFound");
	*/

	//isTex2D, isRenderTarget, alphaTransparency, anisoLevel

	//UE_LOG(JsonLog, Log, TEXT("Texture: %s, %s, %d x %d"), *filename, *name, width, height);
	UE_LOG(JsonLog, Log, TEXT("Texture: %s, %s, %d x %d"), 
		*jsonTex.path, *jsonTex.name, jsonTex.width, jsonTex.height);

	bool isNormalMap = false;//name.EndsWith(FString("_n")) || name.EndsWith(FString("Normals"));
	
	if (jsonTex.importDataFound && jsonTex.normalMapFlag){
		isNormalMap = true;
	}
	else{
		isNormalMap = jsonTex.name.EndsWith(FString("_n")) || jsonTex.name.EndsWith(FString("Normals"));
	}

	if (isNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Texture recognized as normalmap: %s(%s)"), *jsonTex.name, *jsonTex.path);
	}

	UTexture* existingTexture = 0;
	FString ext = FPaths::GetExtension(jsonTex.path);
	UE_LOG(JsonLog, Log, TEXT("filename: %s, ext: %s, assetRootPath: %s"), *jsonTex.path, *ext, *rootPath);

	FString textureName;
	FString packageName;
	UPackage *texturePackage = createPackage(jsonTex.name, jsonTex.path, rootPath, FString("Texture"), 
		&packageName, &textureName, &existingTexture);

	if (existingTexture){
		texIdMap.Add(jsonTex.id, existingTexture->GetPathName());
		UE_LOG(JsonLog, Warning, TEXT("Texutre %s already exists, package %s"), *textureName, *packageName);
		return;
	}

	TArray<uint8> binaryData;

	FString fileSystemPath = FPaths::Combine(*assetRootPath, *jsonTex.path);

	if (ext.ToLower() == FString("tif")){
		UE_LOG(JsonLog, Warning, TEXT("TIF image extension found! Fixing it to png: %s. Image will fail to load if no png file is present."), *fileSystemPath);
		ext = FString("png");
		
		FString pathPart, namePart, extPart;
		FPaths::Split(fileSystemPath, pathPart, namePart, extPart);
		FString newBaseName = FString::Printf(TEXT("%s.%s"), *namePart, *ext);
		fileSystemPath = FPaths::Combine(*pathPart, *newBaseName);
		UE_LOG(JsonLog, Warning, TEXT("New path: %s"), *fileSystemPath);
	}

	if (!FFileHelper::LoadFileToArray(binaryData, *fileSystemPath)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load texture %s(%s)"), *jsonTex.name, *jsonTex.path);
		return;
	}

	if (binaryData.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No binary data: %s"), *jsonTex.name);
		return;
	}

	UE_LOG(JsonLog, Log, TEXT("Loading tex data: %s (%d bytes)"), *jsonTex.name, binaryData.Num());
	auto texFab = NewObject<UTextureFactory>();
	texFab->AddToRoot();
	texFab->SuppressImportOverwriteDialog();
	const uint8* data = binaryData.GetData();

	if (isNormalMap){
		texFab->LODGroup = TEXTUREGROUP_WorldNormalMap;
		texFab->CompressionSettings = TC_Normalmap;
	}

	UE_LOG(JsonLog, Log, TEXT("Attempting to create package: texName %s"), *jsonTex.name);
	UTexture *unrealTexture = (UTexture*)texFab->FactoryCreateBinary(
		UTexture2D::StaticClass(), texturePackage, *textureName, RF_Standalone|RF_Public, 0, *ext, data, data + binaryData.Num(), GWarn);

	if (unrealTexture){
		texIdMap.Add(jsonTex.id, unrealTexture->GetPathName());
		FAssetRegistryModule::AssetCreated(unrealTexture);
		texturePackage->SetDirtyFlag(true);
	}
	texFab->RemoveFromRoot();

#if 0
	auto filename = getString(obj, "path");
	auto id = getInt(obj, "id");
	auto name = getString(obj, "name");
	auto filter = getString(obj, "filterMode");
	auto mipBias = getFloat(obj, "mipMapBias");
	auto width = getInt(obj, "width");
	auto height = getInt(obj, "height");
	auto wrapMode = getString(obj, "wrapMode");
	auto sRGB = getBool(obj, "sRGB");
	auto normalMapFlag = getBool(obj, "normalMapFlag");
	auto importDataFound = getBool(obj, "importDataFound");

	//isTex2D, isRenderTarget, alphaTransparency, anisoLevel

	UE_LOG(JsonLog, Log, TEXT("Texture: %s, %s, %d x %d"), *filename, *name, width, height);

	bool isNormalMap = false;//name.EndsWith(FString("_n")) || name.EndsWith(FString("Normals"));
	
	if (importDataFound && normalMapFlag){
		isNormalMap = true;
	}
	else{
		isNormalMap = name.EndsWith(FString("_n")) || name.EndsWith(FString("Normals"));
	}

	if (isNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Texture recognized as normalmap: %s(%s)"), *name, *filename);
	}

	UTexture* existingTexture = 0;
	FString ext = FPaths::GetExtension(filename);
	UE_LOG(JsonLog, Log, TEXT("filename: %s, ext: %s, assetRootPath: %s"), *filename, *ext, *rootPath);

	FString textureName;
	FString packageName;
	UPackage *texturePackage = createPackage(name, filename, rootPath, FString("Texture"), 
		&packageName, &textureName, &existingTexture);

	if (existingTexture){
		texIdMap.Add(id, existingTexture->GetPathName());
		UE_LOG(JsonLog, Warning, TEXT("Texutre %s already exists, package %s"), *textureName, *packageName);
		return;
	}

	TArray<uint8> binaryData;

	FString fileSystemPath = FPaths::Combine(*assetRootPath, *filename);

	if (ext.ToLower() == FString("tif")){
		UE_LOG(JsonLog, Warning, TEXT("TIF image extension found! Fixing it to png: %s. Image will fail to load if no png file is present."), *fileSystemPath);
		ext = FString("png");
		
		FString pathPart, namePart, extPart;
		FPaths::Split(fileSystemPath, pathPart, namePart, extPart);
		FString newBaseName = FString::Printf(TEXT("%s.%s"), *namePart, *ext);
		fileSystemPath = FPaths::Combine(*pathPart, *newBaseName);
		UE_LOG(JsonLog, Warning, TEXT("New path: %s"), *fileSystemPath);
	}

	if (!FFileHelper::LoadFileToArray(binaryData, *fileSystemPath)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load texture %s(%s)"), *name, *filename);
		return;
	}

	if (binaryData.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No binary data: %s"), *name);
		return;
	}

	UE_LOG(JsonLog, Log, TEXT("Loading tex data: %s (%d bytes)"), *name, binaryData.Num());
	auto texFab = NewObject<UTextureFactory>();
	texFab->AddToRoot();
	texFab->SuppressImportOverwriteDialog();
	const uint8* data = binaryData.GetData();

	if (isNormalMap){
		texFab->LODGroup = TEXTUREGROUP_WorldNormalMap;
		texFab->CompressionSettings = TC_Normalmap;
	}

	UE_LOG(JsonLog, Log, TEXT("Attempting to create package: texName %s"), *name);
	UTexture *unrealTexture = (UTexture*)texFab->FactoryCreateBinary(
		UTexture2D::StaticClass(), texturePackage, *textureName, RF_Standalone|RF_Public, 0, *ext, data, data + binaryData.Num(), GWarn);

	if (unrealTexture){
		texIdMap.Add(id, unrealTexture->GetPathName());
		FAssetRegistryModule::AssetCreated(unrealTexture);
		texturePackage->SetDirtyFlag(true);
	}
	texFab->RemoveFromRoot();
#endif
}
