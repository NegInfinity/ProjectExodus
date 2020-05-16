#pragma once

#include "CoreMinimal.h"
#include "JsonTypes.h"
#include "JsonLog.h"
#include "ImportContext.h"
#include "RawMesh.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include <functional>
#include "JsonObjects/loggers.h"

class JsonImporter;
class UStaticMesh;
class USceneComponent;

#if ((ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 24))
	#define EXODUS_UE_VER_4_24_GE
#endif
#if ((ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 23))
	#define EXODUS_UE_VER_4_23_GE
#endif
#if ((ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 22))
	#define EXODUS_UE_VER_4_22_GE
#endif

namespace UnrealUtilities{
	FVector getUnityUpVector();
	FVector getUnityRightVector();
	FVector getUnityForwardVector();

	void unityMatrixToUnrealBasisVectors(const FMatrix& unityWorld, FVector *outX, FVector *outY, FVector *outZ, FVector *outPos);
	void unityMatrixToUnrealBasisVectors(const FMatrix& unityWorld, FVector &outX, FVector &outY, FVector &outZ, FVector &outPos);

	void convertToInstanceComponent(USceneComponent *comp);
	void makeComponentVisibleInEditor(USceneComponent *comp);

	//Creates a vector perpendicular to the argument, uses Projection onto plane
	FVector makePerpendicular(const FVector& arg);
	//Creates a vector perpendicular to the argument, uses Projection onto plane
	FVector makePerpendicular(const FVector& arg, const FVector& candidate);
	//Creates a vector perpendicular to the argument, uses cross products
	FVector makeCrossPerpendicular(const FVector& arg);
	//Creates a vector perpendicular to the argument, uses cross products. The candidate is used as first argument of the cross product.
	FVector makeCrossPerpendicular(const FVector& arg, const FVector& crossCandidate);

	float unityAngularVelocityToUe(float unityAngularVelocity);
	FVector unityAngularVelocityToUe(const FVector &unityAngularVelocity);

	//Used for converting break forces on constraints
	float unityBreakTorqueToUnreal(float force);

	//Used for converting break forces on constraints
	float unityBreakForceToUnreal(float force);

	/*
	float unityAngularMotorForceToUnreal(float force);
	float unityAngularSpringForceToUnreal(float force);
	*/

	FVector unityToUe(const FVector& arg);
	float unityDistanceToUe(const float arg);
	FVector unityVecToUe(const FVector& arg);
	FVector unityPosToUe(const FVector& arg);
	FVector unitySizeToUe(const FVector& arg);
	FMatrix unityWorldToUe(const FMatrix &unityMatrix);
	FMatrix unityWorldToUe(const FMatrix &unityMatrix, const FVector &localPositionOffset);
	FVector2D unityUvToUnreal(const FVector2D& arg);

	FVector4 getIdxVector4(const TArray<float>& floats, int32 idx);
	FVector2D getIdxVector2(const TArray<float>& floats, int32 idx);
	FVector getIdxVector3(const TArray<float>& floats, int32 idx);
	FColor getIdxColor(const TArray<uint8>& colors, int32 idx);

	template<typename Factory> auto makeFactoryRootPtr(){
		auto factory = NewObject<Factory>();
		factory ->AddToRoot();
		return TSharedPtr<Factory>(
			factory , 
			[](Factory* p){
				if (p) 
					p->RemoveFromRoot();
			}
		);
	}

	template<typename Res> Res* loadResourceById(
			const IdNameMap &idMap, int32 id, 
			std::function<Res*(const FString &path)> loader, 
			const FString& resType = TEXT("Unspecified")){
		check(loader);
		UE_LOG(JsonLog, Log, TEXT("Looking for resource: %s; id: %d"), *resType, id);
		if (id < 0){
			UE_LOG(JsonLog, Log, TEXT("Invalid id %d"), id);
			return 0;
		}
		auto foundPath = idMap.Find(id);
		if (!foundPath){
			UE_LOG(JsonLog, Log, TEXT("Id %d is not in the map"), id);
			return 0;
		}
		auto resPath = *foundPath;
		Res* result = loader(resPath);
		UE_LOG(JsonLog, Log, TEXT("Resource type located: %s"), *resType);
		return result;
	}

	template<typename Res> Res* staticLoadResourceById(
			const IdNameMap &idMap, int32 id, const FString& resType = TEXT("Unspecified")){
		return loadResourceById<Res>(idMap, id, 
			[](const FString &path) -> auto{
				return Cast<Res>(StaticLoadObject(Res::StaticClass(), 0, *path));
			}, TEXT("cubemap")
		);
	}

	//using StaticMeshCallback = std::function<void(UStaticMesh* mesh)>;
	using StaticMeshBuildCallback = std::function<void(UStaticMesh* mesh, FStaticMeshSourceModel& model)>;
	using RawMeshFillCallback = std::function<void(FRawMesh& rawMesh, int lod)>;

	//Mesh generation routine. There's only one lod for now.
	void generateStaticMesh(UStaticMesh *mesh, RawMeshFillCallback fillCallback, 
		StaticMeshBuildCallback preConfig = nullptr, StaticMeshBuildCallback postConfig = nullptr);

	UPackage* createAssetPackage(const FString &objectName, const FString* desiredDir, const JsonImporter *importer, std::function<UObject*(UPackage*)> assetCreator);

	template <typename T>T* createAssetObject(const FString& objectName, const FString* desiredDir, const JsonImporter *importer, 
			std::function<void(T* obj)> onCreate, EObjectFlags objectFlags){
		return createAssetObject<T>(objectName, desiredDir, importer, onCreate, nullptr, objectFlags);
	}

	template <typename T>T* createAssetObject(const FString& objectName, const FString* desiredDir, const JsonImporter *importer, 
			std::function<void(T* obj)> onCreate, std::function<T*(UPackage* pkg, const FString&)> creatorFunc = nullptr, EObjectFlags objectFlags = RF_NoFlags, 
			bool checkForExistingObjects = true){

		T* finalResult;
		auto sanitizedName = sanitizeObjectName(*objectName);
		createAssetPackage(objectName, desiredDir, importer,
			[&](UPackage* pkg) -> T*{
				T* newObj = nullptr;
				if (checkForExistingObjects){
					auto *oldObj = FindObject<T>(pkg, *sanitizedName);
					if (oldObj){
						auto uniqueName = MakeUniqueObjectName(pkg, T::StaticClass(), *sanitizedName).ToString();
						UE_LOG(JsonLog, Log, TEXT("Unique name created: %s (old obj: %x). Original name: %s"), *uniqueName, oldObj, *sanitizedName);
						sanitizedName = uniqueName;
					}
				}
				if (creatorFunc){
					newObj = creatorFunc(pkg, sanitizedName);
				}
				else{
					newObj =  NewObject<T>(pkg, T::StaticClass(), *sanitizedName, objectFlags);
				}
				if (onCreate)
					onCreate(newObj);
				finalResult = newObj;
				return newObj;
			}
		);

		return finalResult;
	}

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
			JsonObjects::logValue("Actor move result: ", moveResult);
		}
		return result;
	}

	template <typename T>T* createActor(ImportContext& workData, FTransform transform, const TCHAR* logName = 0){
		return createActor<T>(workData.world.Get(), transform, workData.editorMode, logName);
	}

	void changeOwnerRecursively(USceneComponent *rootComponent, UObject *newOwner);
	void processComponentsRecursively(USceneComponent *rootComponent,
		std::function<bool(USceneComponent* curComponent)> filterCallback,
		std::function<void(USceneComponent* curComponent)> processCallback,
		bool childrenFirst = false);

	FString sanitizeObjectName(const FString &arg);
	FString sanitizePackageName(const FString &arg);

	FString buildPackagePath(const FString &desiredName, const FString &desiredDir, const JsonImporter *importer);
	FString buildPackagePath(const FString &desiredName, 
		const FString *desiredDir, const JsonImporter *importer);
	FString buildPackagePath(const FString &desiredName, 
		const FString *desiredDir = nullptr, const FString *defaultPackageRoot = nullptr, const FString *commonAssetPath = nullptr);

	UPackage* createPackage(
		const FString &targetPath,
		const FString *defaultPackageRoot = nullptr
	);

	using PackageLoaderFunc = std::function<UPackage*(const FString&)>;

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

	FString genTimestamp();

	void setObjectHierarchy(const ImportedObject &object, ImportedObject *parentObject, 
		const FString& folderPath, ImportContext &workData, const JsonGameObject &gameObj);
	void registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg);

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

	/*
		Those were introduced due to mesh api changed over ocurse of 4.22...4.24. 
		I decided that rather having a bunch of ifdefs all over the place, a better idea would be to wrap related functions around.
	*/
	int getNumLods(UStaticMesh *mesh);
	FStaticMeshSourceModel& getSourceModel(UStaticMesh *mesh, int lod);
	void addSourceModel(UStaticMesh *mesh);
}
