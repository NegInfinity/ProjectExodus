#pragma once
#include "JsonTypes.h"
#include "ImportContext.h"
#include "JsonObjects/JsonGameObject.h"

class JsonImporter;
class UBoxComponent;
class USphereComponent;
class UCapsuleComponent;
class UStaticMeshComponent;

class GeometryComponentBuilder{
protected:
	static ImportedObject processStaticMesh(ImportContext &workData, const JsonGameObject &jsonGameObj, 
		ImportedObject *parentObject, const FString& folderPath, const JsonCollider *colliderData, bool spawnAsComponent, 
		OuterCreatorCallback outerCreator,
		//UObject *outer,
		JsonImporter *importer);

	static void setupCommonColliderSettings(const ImportContext &workData, UPrimitiveComponent *dstCollider, const JsonGameObject &jsonGameObj, const JsonCollider &collider);
	static bool configureStaticMeshComponent(ImportContext &workData, UStaticMeshComponent *meshComp, 
		const JsonGameObject &gameObj, bool configForRender, const JsonCollider *collider, JsonImporter *importer);

	/*static UBoxComponent *createBoxCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static USphereComponent *createSphereCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static UCapsuleComponent *createCapsuleCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider);
	static UStaticMeshComponent *createMeshCollider(UObject *ownerPtr, const JsonGameObject &gameObj, const JsonCollider &collider, 
		ImportContext &workData, JsonImporter *importer);
	static UPrimitiveComponent* processCollider(ImportContext &workData, const JsonGameObject &jsonGameObj, 
		UObject *ownerPtr, const JsonCollider &collider, JsonImporter *importer);*/
	static UBoxComponent* createBoxCollider(OuterCreatorCallback outerCreator, const JsonGameObject& gameObj, const JsonCollider& collider);
	static USphereComponent* createSphereCollider(OuterCreatorCallback outerCreator, const JsonGameObject& gameObj, const JsonCollider& collider);
	static UCapsuleComponent* createCapsuleCollider(OuterCreatorCallback outerCreator, const JsonGameObject& gameObj, const JsonCollider& collider);
	static UStaticMeshComponent* createMeshCollider(OuterCreatorCallback outerCreator, const JsonGameObject& gameObj, const JsonCollider& collider,
		ImportContext& workData, JsonImporter* importer);
	static UPrimitiveComponent* processCollider(ImportContext& workData, const JsonGameObject& jsonGameObj,
		OuterCreatorCallback outerCreator, const JsonCollider& collider, JsonImporter* importer);
public:
	static ImportedObject processMeshAndColliders(ImportContext &workData, 
		const JsonGameObject &jsonGameObj, ImportedObject *parentObject, const FString &folderPath, 
		bool spawnAsComponents,
		//DesiredObjectType desiredObjectType, 
		JsonImporter *importer, OuterCreatorCallback outerCreator);
};
