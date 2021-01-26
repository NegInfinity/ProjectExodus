#include "JsonImportPrivatePCH.h"
#include "UnrealUtilities.h"
#include "JsonImporter.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "UnrealEd/Public/PackageTools.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "Components/SceneComponent.h"

using namespace UnrealUtilities;

UPackage* UnrealUtilities::createAssetPackage(const FString &objectName, const FString* desiredDir, 
		const JsonImporter *importer, std::function<UObject*(UPackage*)> assetCreator){

	auto fullPackagePath = buildPackagePath(objectName, desiredDir, importer);

#ifdef EXODUS_UE_VER_4_26_GE
	auto newPackage = CreatePackage(*fullPackagePath);
#else
	auto newPackage = CreatePackage(0, *fullPackagePath);
#endif

	if (assetCreator){
		auto newAsset = assetCreator(newPackage);
		if (newAsset){
			FAssetRegistryModule::AssetCreated(newAsset);
			newPackage->SetDirtyFlag(true);
		}
	}

	return newPackage;
}	

void UnrealUtilities::addSourceModel(UStaticMesh* mesh){
	check(mesh);
#ifdef EXODUS_UE_VER_4_24_GE
	mesh->AddSourceModel();
#else
	new(mesh->SourceModels) FStaticMeshSourceModel();//???
#endif
}

int UnrealUtilities::getNumLods(UStaticMesh *mesh){
	check(mesh != nullptr);
#ifdef EXODUS_UE_VER_4_24_GE
	return mesh->GetNumLODs();
#else
	return mesh->SourceModels.Num();
#endif
}

FStaticMeshSourceModel& UnrealUtilities::getSourceModel(UStaticMesh *mesh, int lod){
	check(mesh != nullptr);
#ifdef EXODUS_UE_VER_4_24_GE
	return mesh->GetSourceModel(lod);
#else
	return mesh->SourceModels[lod];
#endif
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
#ifdef EXODUS_UE_VER_4_26_GE
	package = CreatePackage(*fullPackagePath);
#else
	package = CreatePackage(0, *fullPackagePath);
#endif
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
#ifdef EXODUS_UE_VER_4_26_GE
		package = CreatePackage(*fullPackagePath);
#else
		package = CreatePackage(0, *fullPackagePath);
#endif
		UE_LOG(JsonLog, Log, TEXT("Package created"));
	}

	UE_LOG(JsonLog, Log, TEXT("Done creating package"));
	return package;
}

void UnrealUtilities::generateStaticMesh(UStaticMesh *mesh, RawMeshFillCallback fillCallback, 
		StaticMeshBuildCallback preConfig, StaticMeshBuildCallback postConfig){
	int32 lod = 0;
	if (getNumLods(mesh) < 1){
		UE_LOG(JsonLog, Warning, TEXT("Adding static mesh lod!"));
		addSourceModel(mesh);
	}

	FStaticMeshSourceModel& srcModel = getSourceModel(mesh, lod);

#ifdef EXODUS_UE_VER_4_22_GE
//#if (ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 22)
	srcModel.StaticMeshOwner = mesh;
#endif

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

	pos = unityPosToUe(pos);//unityToUe(pos)*100.0f;
	//unityPosToUe(pos);
	xAxis = unityVecToUe(xAxis);
	yAxis = unityVecToUe(yAxis);
	zAxis = unityVecToUe(zAxis);

	FMatrix ueMatrix = FMatrix::Identity;//Well, wow. I expected matrix to have a local constructor.
	ueMatrix.SetAxes(&zAxis, &xAxis, &yAxis, &pos);
	return ueMatrix;
}

FMatrix UnrealUtilities::unityWorldToUe(const FMatrix &unityWorld){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	FVector pos = unityWorld.GetOrigin();
	pos = unityPosToUe(pos);//unityToUe(pos)*100.0f;
	xAxis = unityVecToUe(xAxis);
	yAxis = unityVecToUe(yAxis);
	zAxis = unityVecToUe(zAxis);
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

/*

float UnrealUtilities::unityAngularMotorForceToUnreal(float force){
	return force * 100.0f;///
}
float UnrealUtilities::unityAngularSpringForceToUnreal(float force){
	return force * 10.0f;//Why? *100.0f does not seem to produce correct results....
}

*/

float UnrealUtilities::unityBreakTorqueToUnreal(float torque){
	return torque * 10000.0f;
}

float UnrealUtilities::unityBreakForceToUnreal(float force){
	//return force * 500.0f;//Why?
	return force * 100.0f;//It seems that forces are also expressed in somehow centimeter-related units.
}

void UnrealUtilities::unityMatrixToUnrealBasisVectors(const FMatrix& unityWorld, FVector *outX, FVector *outY, FVector *outZ, FVector *outPos){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	if (outPos)
		*outPos = unityPosToUe(unityWorld.GetOrigin());
	if (outX)
		*outX = unityVecToUe(xAxis);
	if (outY)
		*outY = unityVecToUe(yAxis);
	if (outZ)
		*outZ = unityVecToUe(zAxis);
}

void UnrealUtilities::unityMatrixToUnrealBasisVectors(const FMatrix& unityWorld, FVector &outX, FVector &outY, FVector &outZ, FVector &outPos){
	FVector xAxis, yAxis, zAxis;
	unityWorld.GetScaledAxes(xAxis, yAxis, zAxis);
	outPos = unityPosToUe(unityWorld.GetOrigin());
	outX = unityVecToUe(xAxis);
	outY = unityVecToUe(yAxis);
	outZ = unityVecToUe(zAxis);
}

FVector UnrealUtilities::makePerpendicular(const FVector& arg){
	if (arg.IsNearlyZero())
		return FVector(1.0f, 0.0f, 0.0f);

	FVector result = FVector::VectorPlaneProject(FVector(1.0f, 0.0f, 0.0f), arg);
	if (result.IsNearlyZero()){
		result = FVector::VectorPlaneProject(FVector(0.0f, 1.0f, 0.0f), arg);
		if (result.IsNearlyZero()){
			result = FVector::VectorPlaneProject(FVector(0.0f, 0.0f, 1.0f), arg);
		}
	}

	result.Normalize();

	return result;
}

FVector UnrealUtilities::makePerpendicular(const FVector& arg, const FVector& candidate){
	auto result = FVector::VectorPlaneProject(candidate, arg);
	if (result.IsNearlyZero())
		return makePerpendicular(arg);
	result.Normalize();
	return result;
}

float UnrealUtilities::unityAngularVelocityToUe(float unityAngularVelocity){
	return unityAngularVelocity / 360.0f;
}

FVector UnrealUtilities::unityAngularVelocityToUe(const FVector &unityAngularVelocity){
	return FVector(
		unityAngularVelocityToUe(unityAngularVelocity.X),
		unityAngularVelocityToUe(unityAngularVelocity.Y),
		unityAngularVelocityToUe(unityAngularVelocity.Z)
	);
}

FVector UnrealUtilities::makeCrossPerpendicular(const FVector& arg){
	if (arg.IsNearlyZero())
		return FVector(1.0f, 0.0f, 0.0f);

	FVector result = FVector::CrossProduct(FVector(1.0f, 0.0f, 0.0f), arg);
	if (result.IsNearlyZero()){
		result = FVector::CrossProduct(FVector(0.0f, 1.0f, 0.0f), arg);
		if (result.IsNearlyZero()){
			result = FVector::CrossProduct(FVector(0.0f, 0.0f, 1.0f), arg);
		}
	}

	result.Normalize();

	return result;
}

FVector UnrealUtilities::makeCrossPerpendicular(const FVector& arg, const FVector& crossCandidate){
	auto result = FVector::CrossProduct(crossCandidate, arg);
	if (result.IsNearlyZero())
		return makeCrossPerpendicular(arg);
	result.Normalize();
	return result;
}

FVector UnrealUtilities::getUnityUpVector(){
	return FVector(0.0f, 1.0f, 0.0f);
}

FVector UnrealUtilities::getUnityRightVector(){
	return FVector(1.0f, 0.0f, 0.0f);
}

FVector UnrealUtilities::getUnityForwardVector(){
	return FVector(0.0f, 0.0f, 1.0f);
}

void UnrealUtilities::setObjectHierarchy(const ImportedObject &object, ImportedObject *parentObject, 
	const FString& folderPath, ImportContext &workData, const JsonGameObject &gameObj, bool setActiveFlag){
	if (parentObject){
		object.attachTo(*parentObject);
	}
	else{
		if (folderPath.Len())
			object.setFolderPath(*folderPath);
	}

	if (setActiveFlag)
		object.setActiveInHierarchy(gameObj.activeInHierarchy);
}

void UnrealUtilities::registerImportedObject(ImportedObjectArray *outArray, const ImportedObject &arg){
	if (!outArray)
		return;
	if (!arg.isValid())
		return;
	outArray->Push(arg);
}

void UnrealUtilities::convertToInstanceComponent(USceneComponent *comp){
	if (!comp)
		return;
	auto rootActor = comp->GetAttachmentRootActor();
	check(rootActor);
	rootActor->AddInstanceComponent(comp);
}

void UnrealUtilities::makeComponentVisibleInEditor(USceneComponent *comp){
	if (!comp)
		return;
	auto rootActor = comp->GetAttachmentRootActor();
	check(rootActor);
	comp->bEditableWhenInherited = true;
	comp->RegisterComponent();
}

void UnrealUtilities::changeOwnerRecursively(USceneComponent *rootComponent, UObject *newOwner){
	check(rootComponent != nullptr);
	check(newOwner != nullptr);

	auto rootActor = rootComponent->GetAttachmentRootActor();
	auto numChildren = rootComponent->GetNumChildrenComponents();
	for(int32 childIndex = 0; childIndex < numChildren; childIndex++){
		auto child = rootComponent->GetChildComponent(childIndex);
		if (!child)
			continue;
		auto childRootActor = child->GetAttachmentRootActor();

		//This method awkwardly detects situation when child component is a component that belong to another actor
		if (rootActor != childRootActor)
			continue;

		changeOwnerRecursively(child, newOwner);
	}

	rootComponent->Rename(0, newOwner);
}

void UnrealUtilities::processComponentsRecursively(USceneComponent *rootComponent,
		std::function<bool(USceneComponent* curComponent)> filterCallback,
		std::function<void(USceneComponent* curComponent)> processCallback,
		bool childrenFirst){

	check(processCallback);
	check(rootComponent);

	if (filterCallback && !filterCallback(rootComponent))
		return;

	if (!childrenFirst)
		processCallback(rootComponent);

	auto numChildren = rootComponent->GetNumChildrenComponents();
	for(int32 childIndex = 0; childIndex < numChildren; childIndex++){
		auto child = rootComponent->GetChildComponent(childIndex);
		if (!child)
			continue;

		if (filterCallback && !filterCallback(child))
			continue;

		processComponentsRecursively(child, filterCallback, processCallback, childrenFirst);
	}

	if (childrenFirst)
		processCallback(rootComponent);
}

bool UnrealUtilities::renameComponent(USceneComponent* component, const FString& newName, bool allowSafeRename){
	if (!component)
		return false;
	if (!allowSafeRename){
		return component->Rename(*newName);
	}

	const FString* curNameCandidate = &newName;
	FString nameCandidate;
	const int maxNameAttempts = 64;
	bool warned = false;

	for (int attemptIndex = 0; attemptIndex < maxNameAttempts; attemptIndex++){
		check(curNameCandidate);

		if (component->Rename(**curNameCandidate, 0, REN_Test)){
			return component->Rename(**curNameCandidate);
		}
		if (!warned){
			UE_LOG(JsonLog, Warning, TEXT("Name clash while tyring to rename component %s into %s. Trying to generate unique name"),
				*component->GetFullName(),
				*newName);
			warned = true;
		}

		auto tmpGuid = FGuid::NewGuid();
		auto guidName = tmpGuid.ToString(EGuidFormats::Digits);
		nameCandidate = FString::Printf(TEXT("%s-(uid:%s)"), *newName, *guidName);
		curNameCandidate = &nameCandidate;

		UE_LOG(JsonLog, Warning, TEXT("Renaming %s; attempt %d, Name candidate: %s"),
			*component->GetFullName(), attemptIndex, *nameCandidate
		);
	}

	UE_LOG(JsonLog, Warning, TEXT("Could not resolve name clash, while renaming component %s into %s. Max name attempts exceeded"),
		*component->GetFullName(), *newName
	);

	return false;
}

