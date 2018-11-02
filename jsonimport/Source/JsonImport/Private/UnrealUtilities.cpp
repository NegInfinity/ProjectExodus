#include "JsonImportPrivatePCH.h"
#include "UnrealUtilities.h"

using namespace UnrealUtilities;

FString UnrealUtilities::getDefaultImportPath(){
	return TEXT("/Game/Import");
}

FString UnrealUtilities::sanitizeObjectName(const FString &arg){
	return ObjectTools::SanitizeObjectName(arg);
}

FString UnrealUtilities::sanitizePackageName(const FString &arg){
	return PackageTools::SanitizePackageName(arg);
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
