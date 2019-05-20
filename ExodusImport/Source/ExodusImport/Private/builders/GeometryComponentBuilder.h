#pragma once
#include "JsonTypes.h"
#include "ImportWorkData.h"
#include "JsonObjects/JsonGameObject.h"

class JsonImporter;
class UBoxComponent;
class USphereComponent;
class UCapsuleComponent;
class UStaticMeshComponent;

class GeometryComponentBuilder{
protected:
	static ImportedObject processStaticMesh(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		int objId, ImportedObject *parentObject, const FString& folderPath, const JsonCollider *colliderData, bool spawnAsComponent, UObject *outer,
		JsonImporter *importer);

	static void setupCommonColliderSettings(const ImportWorkData &workData, UPrimitiveComponent *dstCollider, const JsonGameObject &jsonGameObj, const JsonCollider &collider);
	static bool configureStaticMeshComponent(ImportWorkData &workData, UStaticMeshComponent *meshComp, 
		const JsonGameObject &gameObj, bool configForRender, const JsonCollider *collider, JsonImporter *importer);

	static UBoxComponent *createBoxCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static USphereComponent *createSphereCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static UCapsuleComponent *createCapsuleCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static UStaticMeshComponent *createMeshCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider, 
		ImportWorkData &workData, JsonImporter *importer);

	static UPrimitiveComponent* processCollider(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		UObject *ownerPtr, const JsonCollider &collider, JsonImporter *importer);
public:
	static ImportedObject processMeshAndColliders(ImportWorkData &workData, 
		const JsonGameObject &jsonGameObj, int objId, ImportedObject *parentObject, const FString &folderPath, DesiredObjectType desiredObjectType, JsonImporter *importer);
};
