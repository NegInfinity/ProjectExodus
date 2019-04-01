#include "JsonImportPrivatePCH.h"
#include "UnrealUtilities.h"
#include "JsonImporter.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "UnrealEd/Public/PackageTools.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"

using namespace UnrealUtilities;

UPackage* UnrealUtilities::createAssetPackage(const FString &objectName, const FString* desiredDir, 
		const JsonImporter *importer, std::function<UObject*(UPackage*)> assetCreator){

	auto fullPackagePath = buildPackagePath(objectName, desiredDir, importer);

	auto newPackage = CreatePackage(0, *fullPackagePath);

	if (assetCreator){
		auto newAsset = assetCreator(newPackage);
		if (newAsset){
			FAssetRegistryModule::AssetCreated(newAsset);
			newPackage->SetDirtyFlag(true);
		}
	}

	return newPackage;
}
	

FString UnrealUtilities::getDefaultImportPath(){
	return TEXT("/Game/Import");
}

FString UnrealUtilities::sanitizeObjectName(const FString &arg){
	return ObjectTools::SanitizeObjectName(arg);
}

FString UnrealUtilities::sanitizePackageName(const FString &arg){
	return PackageTools::SanitizePackageName(arg);
}

FString UnrealUtilities::buildPackagePath(const FString &desiredName, const FString &desiredDir, const JsonImporter *importer){
	return buildPackagePath(desiredName, &desiredDir, importer);
}

FString UnrealUtilities::buildPackagePath(const FString &desiredName, const FString *desiredDir, const JsonImporter *importer){
	if (importer){
		auto commonPath = importer->getAssetCommonPath();
		auto importPath = importer->getProjectImportPath();
		return buildPackagePath(desiredName, desiredDir, &importPath, &commonPath);
	}
	return buildPackagePath(desiredName, desiredDir, nullptr, nullptr);
}


FString UnrealUtilities::buildPackagePath(const FString &desiredName, const FString *desiredDir, const FString *defaultPackageRoot, const FString *commonAssetPath){
	FString packageRoot = defaultPackageRoot ? *defaultPackageRoot: *getDefaultImportPath();

	FString finalPath;

	if (desiredDir && (desiredDir->Len() > 0)){
		auto targetDir = *desiredDir;
		if (commonAssetPath){
			if (targetDir.StartsWith(*commonAssetPath)){
				targetDir = FString(*targetDir + commonAssetPath->Len());
			}
		}
		finalPath = FPaths::Combine(*packageRoot, *targetDir, *desiredName);
	}
	else{
		finalPath = FPaths::Combine(*packageRoot, *desiredName);
	}

	finalPath = PackageTools::SanitizePackageName(finalPath);
	return finalPath;
}

UPackage* UnrealUtilities::createPackage(const FString &targetPath,
		const FString *defaultPackageRoot){

	auto fullPackagePath = buildPackagePath(targetPath, nullptr, 
		defaultPackageRoot);

	UPackage *package = 0;
	package = CreatePackage(0, *fullPackagePath);
	return package;
}


/*
TODO:

Sooo.... this is actually a bit nuts. This function performs to many things at once.
Sanitizes names of object and package and returns them. 
Actually builds package name and builds path, shortening if needed...
Attempts to load existing package several times, and returns the result, if it is successful.

This needs to be refactored. Badly. But, not right now.
*/
UPackage* UnrealUtilities::createPackage(const FString &basePackageName, 
		const FString &srcPackagePath, 
		const FString &objectNameSuffix, 
		const FString *commonAssetPath,
		const FString *defaultPackageRoot,
		FString *outSanitizedPackageName, 
		FString *outSanitizedObjName, 		
		PackageLoaderFunc existingPackageLoader){

	FString objDir = FPaths::GetPath(srcPackagePath);

	auto objSuffixName = ObjectTools::SanitizeObjectName(basePackageName + TEXT("_") + objectNameSuffix);
	auto objName = ObjectTools::SanitizeObjectName(basePackageName);

	auto fullPackagePath = buildPackagePath(objName, &objDir, 
		defaultPackageRoot, commonAssetPath);

	if (outSanitizedPackageName){
		*outSanitizedPackageName = fullPackagePath;
	}

	if (outSanitizedObjName){
		*outSanitizedObjName = objSuffixName;
	}

	UPackage *package = 0;

	if (existingPackageLoader){
		if (!package){
			FString testPath = fullPackagePath + TEXT(".") + objSuffixName;
			package = existingPackageLoader(testPath);
		}
		if (!package){
			FString testPath = fullPackagePath + TEXT(".") + objName;
			package = existingPackageLoader(testPath);
		}
	}

	if (package){
		UE_LOG(JsonLog, Log, TEXT("Found existing package %s"), *fullPackagePath);
	}
	else{
		UE_LOG(JsonLog, Log, TEXT("Creating package %s"), *fullPackagePath);
		package = CreatePackage(0, *fullPackagePath);
		UE_LOG(JsonLog, Log, TEXT("Package created"));
	}

	UE_LOG(JsonLog, Log, TEXT("Done creating package"));
	return package;
}

void UnrealUtilities::generateStaticMesh(UStaticMesh *mesh, RawMeshFillCallback fillCallback, 
		StaticMeshBuildCallback preConfig, StaticMeshBuildCallback postConfig){
	int32 lod = 0;
	if (mesh->SourceModels.Num() < 1){
		UE_LOG(JsonLog, Warning, TEXT("Adding static mesh lod!"));
		new(mesh->SourceModels) FStaticMeshSourceModel();//???
	}

	FStaticMeshSourceModel &srcModel = mesh->SourceModels[lod];

	mesh->LightingGuid = FGuid::NewGuid();
	mesh->LightMapResolution = 64;///config?`
	mesh->LightMapCoordinateIndex = 1;
	if (preConfig)
		preConfig(mesh, srcModel);

	FRawMesh newRawMesh;
	srcModel.RawMeshBulkData->LoadRawMesh(newRawMesh);
	newRawMesh.VertexPositions.SetNum(0);

	if (fillCallback){
		fillCallback(newRawMesh, lod);
	}

	bool valid = newRawMesh.IsValid();
	bool fixable = newRawMesh.IsValidOrFixable();
	UE_LOG(JsonLog, Log, TEXT("Mesh is valid: %d, mesh is validOrFixable: %d"), (int)valid, (int)fixable);
	if (!valid){
		UE_LOG(JsonLog, Warning, TEXT("Mesh is not valid!"));
		if (!fixable){
			UE_LOG(JsonLog, Warning, TEXT("Mesh is not fixable!"));
		}
	}

	srcModel.RawMeshBulkData->SaveRawMesh(newRawMesh);

	srcModel.BuildSettings.bRecomputeNormals = false;//!hasNormals;//hasNormals
	srcModel.BuildSettings.bRecomputeTangents = true;

	if (postConfig)
		postConfig(mesh, srcModel);

	TArray<FText> buildErrors;
	mesh->Build(false, &buildErrors);
	for(FText& err: buildErrors){
		UE_LOG(JsonLog, Error, TEXT("MeshBuildError: %s"), *(err.ToString()));
	}
}

FString UnrealUtilities::genTimestamp(){
	auto curTime = FDateTime::UtcNow();
	return curTime.ToString(TEXT("%Y%m%d%H%M%S%Z"));
}

FVector2D UnrealUtilities::unityUvToUnreal(const FVector2D& arg){
	return FVector2D(arg.X, 1.0f - arg.Y);
}

FVector UnrealUtilities::unityToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

FVector UnrealUtilities::unityVecToUe(const FVector& arg){
	return FVector(arg.Z, arg.X, arg.Y);
}

FVector UnrealUtilities::unityPosToUe(const FVector& arg){
	return unityVecToUe(arg) * 100.0f;
}

FVector UnrealUtilities::unitySizeToUe(const FVector& arg){
	return unityVecToUe(arg) * 100.0f;
}

float UnrealUtilities::unityDistanceToUe(const float arg){
	return arg * 100.0f;
}


FMatrix UnrealUtilities::unityWorldToUe(const FMatrix &unityWorld, const FVector &localPositionOffset){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = unityWorld.GetOrigin();

	pos += xAxis * localPositionOffset.X + yAxis * localPositionOffset.Y + zAxis * localPositionOffset.Z;

	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);

	FMatrix ueMatrix = FMatrix::Identity;//Well, wow. I expected matrix to have a local constructor.
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);
	return ueMatrix;
}

FMatrix UnrealUtilities::unityWorldToUe(const FMatrix &unityWorld){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = unityWorld.GetOrigin();
	pos = unityToUe(pos)*100.0f;
	xAxis = unityToUe(xAxis);
	yAxis = unityToUe(yAxis);
	zAxis = unityToUe(zAxis);
	FMatrix ueMatrix = FMatrix::Identity;//Well, wow. I expected matrix to have a local constructor.
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);
	return ueMatrix;
}

FVector2D UnrealUtilities::getIdxVector2(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*2 + 1))
		return FVector2D();
	return FVector2D(floats[idx*2], floats[idx*2+1]);
};

FVector UnrealUtilities::getIdxVector3(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*3 + 2))
		return FVector();
	return FVector(floats[idx*3], floats[idx*3+1], floats[idx*3+2]);
};

FVector4 UnrealUtilities::getIdxVector4(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*4 + 3))
		return FVector();
	return FVector4(floats[idx*4], floats[idx*4+1], floats[idx*4+2], floats[idx*4+3]);
};

FColor UnrealUtilities::getIdxColor(const TArray<uint8>& colors, int32 idx){
	return FColor(
		colors[idx * 4], 
		colors[idx * 4 + 1], 
		colors[idx * 4 + 2], 
		colors[idx * 4 + 3]
	);
}

float UnrealUtilities::unityTorqueToUnreal(float torque){
	return torque;
}

float UnrealUtilities::unityForceToUnreal(float force){
	return force * 500.0f;//Why?
}
