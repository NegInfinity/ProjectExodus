#pragma once

#include "CoreMinimal.h"
#include "JsonTypes.h"
#include "JsonLog.h"
#include "ImportWorkData.h"
#include <functional>

namespace UnrealUtilities{
	template <typename T>T* createActor(UWorld *world, FTransform transform, bool editorMode, const TCHAR* logName = 0){
		T* result = 0;
		if (editorMode){
			result = 	Cast<T>(GEditor->AddActor(world->GetCurrentLevel(),
				T::StaticClass(), transform));
		}
		else{
			result = world->SpawnActor<T>();
		}
		if (!result){
			if (logName){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn actor %s"), logName);
			}
			else{
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn templated actor"));
			}
		}
		else{
			auto moveResult = result->SetActorTransform(transform, false, nullptr, ETeleportType::ResetPhysics);
			logValue("Actor move result: ", moveResult);
		}
		return result;
	}

	template <typename T>T* createActor(ImportWorkData& workData, FTransform transform, const TCHAR* logName = 0){
		return createActor<T>(workData.world, transform, workData.editorMode, logName);
	}	

	using PackageLoaderFunc = std::function<UPackage*(const FString&)>;

	FString sanitizeObjectName(const FString &arg);
	FString sanitizePackageName(const FString &arg);

	FString buildPackagePath(const FString &desiredName, 
		const FString *desiredDir = nullptr, const FString *defaultPackageRoot = nullptr, const FString *commonAssetPath = nullptr);

	UPackage* createPackage(
		const FString &targetPath,
		const FString *defaultPackageRoot = nullptr
	);

	UPackage* createPackage(
		const FString &basePackageName, 
		const FString &srcPackagePath, 
		const FString &objectNameSuffix, 
		const FString *commonAssetPath = nullptr,
		const FString *defaultPackageRoot = nullptr,
		FString *outSanitizedPackageName = nullptr, 
		FString *outSanitizedObjName = nullptr, 		
		PackageLoaderFunc existingPackageLoader = PackageLoaderFunc());

	FString getDefaultImportPath();

	template<typename T> UPackage* createPackage(
			const FString &basePackageName, 
			const FString &srcPackagePath,
			const FString &objectNameSuffix,
			const FString *commonAssetPath = nullptr,
			const FString *defaultPackageRoot = nullptr,
			FString *outSanitizedPackageName = nullptr, 
			FString *outSanitizedObjName = nullptr, 		
			T** outExistingObj = nullptr){

		T* existingObj = nullptr;
		UPackage* result = createPackage(
			basePackageName, srcPackagePath, objectNameSuffix, commonAssetPath, defaultPackageRoot, 
			outSanitizedPackageName, outSanitizedobjName, 
			[&](const FString &path) -> UPackage*{
				auto obj  = Cast<T>(LoadObject<T>(0, *objPath));
				if (obj){
					existingObj  = obj;
					return existingObj->GetOutermost();
				}
				return nullptr;
			}
		)
		if (outExistingObj){
			*outExistingObj = existingObj;
		}
	}
}
