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

bool loadTextureData(ByteArray &outData, const FString &path){
	FString fileSystemPath = path;
	FString ext = FPaths::GetExtension(fileSystemPath);

	if (ext.ToLower() == FString("tif")){
		UE_LOG(JsonLog, Warning, TEXT("TIF image extension found! Fixing it to png: %s. Image will fail to load if no png file is present."), *fileSystemPath);
		ext = FString("png");
		
		FString pathPart, namePart, extPart;
		FPaths::Split(fileSystemPath, pathPart, namePart, extPart);
		FString newBaseName = FString::Printf(TEXT("%s.%s"), *namePart, *ext);
		fileSystemPath = FPaths::Combine(*pathPart, *newBaseName);
		UE_LOG(JsonLog, Warning, TEXT("New path: %s"), *fileSystemPath);
	}

	if (!FFileHelper::LoadFileToArray(outData, *fileSystemPath)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load file \"%s\""), *fileSystemPath);
		return false;
	}

	if (outData.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No binary data in \"%s\""), *fileSystemPath);
		return false;
	}

	return true;
}

struct SrcPixel32{
	uint8 r, g, b, a;
};

struct SrcPixel32F{
	float r, g, b, a;
};

struct DstPixel16F{
	FFloat16 b, g, r, a;
};

//Well, this doesn't work so I give up.
static bool loadCompressedBinary(ByteArray &outData, const FString &filename){
	ByteArray tmpData;
	if (!FFileHelper::LoadFileToArray(tmpData, *filename)){
		UE_LOG(JsonLog, Error, TEXT("Could not load data from \"%s\""), *filename);
		return false;
	}

	const auto dataPtr = tmpData.GetData();
	int32 dstSize = *((int32*)dataPtr);
	auto headerSize = sizeof(dstSize);
	int32 srcSize = tmpData.Num() - headerSize;
	outData.SetNumUninitialized(dstSize );
	const auto srcPtr = dataPtr + headerSize;
	auto dstPtr = outData.GetData();

	return FCompression::UncompressMemory(COMPRESS_ZLIB, dstPtr, dstSize, srcPtr, srcSize, false, 
		DEFAULT_ZLIB_BIT_WINDOW|32 //This is black magic needed to make FCompression treat the stream as gzip stream.
	);

	//return true;
}

void JsonImporter::importCubemap(JsonObjPtr data, const FString &rootPath){
	JsonCubemap jsonCube(data);
	UE_LOG(JsonLog, Log, TEXT("Cubemap: %d, %s, %s (%s), %dx%d"), 
		jsonCube.id, *jsonCube.name, *jsonCube.assetPath, *jsonCube.exportPath, 
		jsonCube.texParams.width, jsonCube.texParams.height);

	UTextureCube* existingTexture = 0;
	FString ext = FPaths::GetExtension(jsonCube.exportPath);
	UE_LOG(JsonLog, Log, TEXT("filename: %s, ext: %s, assetRootPath: %s"), *jsonCube.assetPath, *ext, *rootPath);

	FString textureName;
	FString packageName;
	UPackage *texturePackage = createPackage(jsonCube.name, jsonCube.assetPath, rootPath, FString("TextureCube"), 
		&packageName, &textureName, &existingTexture);

	if (existingTexture){
		cubeIdMap.Add(jsonCube.id, existingTexture->GetPathName());
		UE_LOG(JsonLog, Warning, TEXT("Cube texture %s already exists, package %s"), *textureName, *packageName);
		return;
	}

	ByteArray binaryData; 
	//well, unreal can't load 2d images for cubemaps. So, raw data is the way to go
	auto fullRawPath = FPaths::Combine(*assetRootPath, *jsonCube.rawPath);
	if (!loadCompressedBinary(binaryData, fullRawPath)){
		UE_LOG(JsonLog, Error, TEXT("Could not load compressed data from \"%s\""), *fullRawPath);
		return;
	}
	/*
	if (!FFileHelper::LoadFileToArray(binaryData, *fullRawPath)){
		UE_LOG(JsonLog, Error, TEXT("Could not load data from \"%s\""), *fullRawPath);
		return;
	}
	*/

	auto texFab = makeFactoryRootPtr<UTextureFactory>();
	texFab->SuppressImportOverwriteDialog();
	//const uint8* data = binaryData.GetData();

	auto cubeSize = jsonCube.texParams.width;

	UE_LOG(JsonLog, Log, TEXT("Attempting to create package: texName %s"), *jsonCube.name);
	UTextureCube *cubeTex = texFab->CreateTextureCube(texturePackage, *textureName, RF_Standalone|RF_Public);

	ETextureSourceFormat sourceFormat = jsonCube.isHdr ? TSF_RGBA16F: TSF_BGRA8;

//	cubeTex->Source.Init(cubeSize, cubeSize, 6, 1, sourceFormat);
	cubeTex->Source.Init(cubeSize, cubeSize, 6, 1, sourceFormat);
	if (jsonCube.isHdr){
		cubeTex->CompressionSettings = TC_HDR;
	}

	auto* lockedMip = cubeTex->Source.LockMip(0);
	const auto numSlices = 6;
	if (jsonCube.isHdr){
		const SrcPixel32F *srcData = (SrcPixel32F*)binaryData.GetData();
		DstPixel16F *dstData = (DstPixel16F*)lockedMip;
		for(int slice = 0; slice < numSlices; slice++){
			auto srcSlice = srcData + cubeSize * cubeSize * slice;
			auto dstSlice = dstData + cubeSize * cubeSize * slice;
			for(int y = 0; y < cubeSize; y++){
				auto srcScan = srcSlice + y * cubeSize;
				auto dstScan = dstSlice + y * cubeSize;
				for(int x = 0; x < cubeSize; x++){
					dstScan[x].r.Set(srcScan[x].r);
					dstScan[x].g.Set(srcScan[x].g);
					dstScan[x].b.Set(srcScan[x].b);
					dstScan[x].a.Set(srcScan[x].a);
					//dstScan[x] = srcScan[x];
				}
			}
		}
	}
	else{
		const SrcPixel32 *srcData = (SrcPixel32*)binaryData.GetData();
		SrcPixel32 *dstData = (SrcPixel32*)lockedMip;
		auto sliceSize = cubeSize * cubeSize;
		for(int slice = 0; slice < numSlices; slice++){
			auto srcSlice = srcData + sliceSize * slice;
			auto dstSlice = dstData + sliceSize * slice;
			for(int y = 0; y < cubeSize; y++){
				auto srcScan = srcSlice + y * cubeSize;
				auto dstScan = dstSlice + y * cubeSize;
				for(int x = 0; x < cubeSize; x++){
					dstScan[x] = srcScan[x];
				}
			}
		} 
	}

	cubeTex->SRGB = jsonCube.texImportParams.initialized && jsonCube.texImportParams.sRGBTexture;
	//texture mipmap generation is not supported for cubemaps?
	cubeTex->Source.UnlockMip(0);

	cubeTex->MipGenSettings = TMGS_NoMipmaps;//TMGS_LeaveExistingMips;
	//cubeTex->Source.

	if (cubeTex){
		cubeIdMap.Add(jsonCube.id, cubeTex->GetPathName());
		cubeTex->PostEditChange();
		FAssetRegistryModule::AssetCreated(cubeTex);
		texturePackage->SetDirtyFlag(true);
	}
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
