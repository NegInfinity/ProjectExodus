#include "JsonImportPrivatePCH.h"
#include "GeometryComponentBuilder.h"
#include "JsonImporter.h"
#include "UnrealUtilities.h"
#include "Engine/StaticMeshActor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/CollisionProfile.h"

/*
This does it. 

This method processess collision and mesh during object import
*/
ImportedObject GeometryComponentBuilder::processMeshAndColliders(ImportContext& workData,
	const JsonGameObject& jsonGameObj, ImportedObject* parentObject, const FString& folderPath,
	bool spawnAsComponents,//DesiredObjectType desiredObjectType,
	JsonImporter* importer, OuterCreatorCallback outerCreator) {
	using namespace UnrealUtilities;
	/*
	There are several scenarios ....

	* If there is no mesh and no colliders, we return blank objects.
	* If mesh is not present, but colliders are here, then one collider is selected as a "root" and the rest of them are parented to it.
	* If mesh is present, but does not have a collider, then it is parented to the collider mini-tree. The reasoning for that is that colliders govern rigidbody movement, so the mesh should follow components.
	* If the mesh is present, and is used as a collider, then it becomes the root of collider mini-tree, and the rest of the colliders are parented to it.

	Mesh must be attached to a moving collider, as collider doubles down as a rigidbody.
	*/

	/*
	"Outer" woes.

	A component cannot be created in vacuum and needs to be enclosed in a package.
	By default the package is "Transient", but transient packages disappear.

	Meaning.... by default we need to find closest parent, and grab "Outer" from there.

	If there's no such parent, OR if the user requested an actor, then we either spawn a blank, OR process static mesh as an actor and return that.
	*/

	//UObject* outer = nullptr;

	/*
	We only need outer for components. If components weren't requested, an actor would be created and will serve as outer container for everything else
	*/
	bool componentRequested = spawnAsComponents;//(desiredObjectType == DesiredObjectType::Component);
	if (componentRequested)
		check(outerCreator);

	/*
	if (outerCreator && componentRequested) {
		outer = outerCreator();
	}
	else {
		outer = workData.findSuitableOuter(jsonGameObj);
	}
	*/
	//can be null at this point

	bool hasMainMesh = jsonGameObj.hasMesh();
	int mainMeshColliderIndex = jsonGameObj.findMainMeshColliderIndex();
	auto mainMeshCollider = jsonGameObj.getColliderByIndex(mainMeshColliderIndex);

	ImportedObject collisionMesh, displayOnlyMesh;

	/*
	Not spawning mesh as component means spawning it as an actor.
	We... we only want to spawn static mesh as an actor if:
	1. The object has no colliders
	2. It has a signle collider that uses the same mesh

	And that's it.
	*/
	if (jsonGameObj.hasMesh()) {
		if (jsonGameObj.colliders.Num() == 0) {//only display mesh is present
			//return processStaticMesh(workData, jsonGameObj, parentObject, folderPath, nullptr, componentRequested && outer, outer, importer);
			return processStaticMesh(workData, jsonGameObj, parentObject, folderPath, nullptr, componentRequested, outerCreator, importer);
		}
		if ((jsonGameObj.colliders.Num() == 1) && mainMeshCollider) {
			//return processStaticMesh(workData, jsonGameObj, parentObject, folderPath, mainMeshCollider, componentRequested && outer, outer, importer);
			return processStaticMesh(workData, jsonGameObj, parentObject, folderPath, mainMeshCollider, componentRequested, outerCreator, importer);
		}
	}

	//No mesh and no colliders
	if (!jsonGameObj.hasMesh() && !jsonGameObj.hasColliders()) {
		return ImportedObject();//We're processing an empty and by default they're not recreated as scene components. This may change in future.
	}

	/*
	Goddamit this is getting too complicated.
	*/
	/*
	if (!spawnAsComponents) {//desiredObjectType == DesiredObjectType::Actor){
		outer = nullptr;//this will force creation of blank actor that will also serve as outer package.
	}
	*/

	USceneComponent* rootComponent = nullptr;
	AActor* createdRootActor = nullptr;
	OuterCreatorCallback colliderOuterCreator = [&]() -> UObject*{
		if (spawnAsComponents){
			check(outerCreator);
			return outerCreator();
		}
		if (!createdRootActor){
			createdRootActor = workData.world->SpawnActor<AActor>(AActor::StaticClass(), jsonGameObj.getUnrealTransform());
			createdRootActor->SetActorLabel(jsonGameObj.ueName);
			createdRootActor->SetFolderPath(*folderPath);
			check(createdRootActor);
		}
		return createdRootActor;
	};

	bool spawnMeshAsComponent = true;
	/*
	if (!outer){
		rootActor = workData.world->SpawnActor<AActor>(AActor::StaticClass(), jsonGameObj.getUnrealTransform());
		rootActor->SetActorLabel(jsonGameObj.ueName);
		rootActor->SetFolderPath(*folderPath);
		outer = rootActor;
	}
	*/

	//check(outer);
	//object has main mesh... meaning display or collision mesh.
	if (hasMainMesh){
		//This is nearly a code duplication
		if (mainMeshCollider){
			collisionMesh = processStaticMesh(workData, jsonGameObj, nullptr, folderPath, mainMeshCollider, spawnMeshAsComponent, colliderOuterCreator, importer);
			auto name = FString::Printf(TEXT("%s_collisionMesh"), *jsonGameObj.ueName);
			collisionMesh.setNameOrLabel(*name);
		}
		else{
			displayOnlyMesh = processStaticMesh(workData, jsonGameObj, nullptr, folderPath, nullptr, spawnMeshAsComponent, colliderOuterCreator, importer);
			auto name = FString::Printf(TEXT("%s_displayMesh"), *jsonGameObj.ueName);
			displayOnlyMesh.setNameOrLabel(*name);
		}
	}
	//check(outer);

	ImportedObject rootObject;
	//check(outer);

	TArray<UPrimitiveComponent*> newColliders;
	//Walk through collider list, create primtivies, except that one collider used for the main static mesh.
	for (int i = 0; i < jsonGameObj.colliders.Num(); i++){
		const auto &curCollider = jsonGameObj.colliders[i];

		if (hasMainMesh && (i == mainMeshColliderIndex) && (curCollider.isMeshCollider())){
			newColliders.Add(nullptr);
			continue;
		}

		//auto collider = processCollider(workData, jsonGameObj, outer, curCollider, importer);
		auto collider = processCollider(workData, jsonGameObj, colliderOuterCreator, curCollider, importer);
		if (!collider){
			UE_LOG(JsonLog, Warning, TEXT("Could not create collider %d on %d(%s)"), i, jsonGameObj.id, *jsonGameObj.name);
			continue;
		}
		auto name = FString::Printf(TEXT("%s_collider#%cd hd(%s)"), *jsonGameObj.ueName, i, *curCollider.colliderType);

		collider->Rename(*name);
		newColliders.Add(collider);
	}

	//Pick a component suitable for the "root" of the collider hierarchy
	int rootComponentIndex = mainMeshColliderIndex;
	if (!mainMeshCollider){
		rootComponentIndex = jsonGameObj.findSuitableRootColliderIndex();
		if ((rootComponentIndex < 0) || (rootComponentIndex >= newColliders.Num())){
			UE_LOG(JsonLog, Warning, TEXT("Could not find suitable root collider on %s(%d)"), *jsonGameObj.name, jsonGameObj.id);
			rootComponentIndex = 0;
		}

		check(newColliders.Num() > 0);
		rootComponent = newColliders[rootComponentIndex];
		check(rootComponent);
		if (createdRootActor)
			createdRootActor->SetRootComponent(rootComponent);
		///newColliders.RemoveAt(rootCompIndex);//It is easier to handle this here... or not
	}

	check(rootComponent);
	rootObject = createdRootActor ? ImportedObject(createdRootActor)
		:ImportedObject(rootComponent);

	for (int i = 0; i < newColliders.Num(); i++){
		auto curCollider = newColliders[i];
		if (!curCollider)
			continue;
		if (i != rootComponentIndex){
			auto tmpObj = ImportedObject(curCollider);
			tmpObj.attachTo(rootObject);
		}

		makeComponentVisibleInEditor(curCollider);
		convertToInstanceComponent(curCollider);
	}

	if (displayOnlyMesh.isValid()){
		check(rootObject.isValid());
		displayOnlyMesh.attachTo(rootObject);
		displayOnlyMesh.fixEditorVisibility();
	}

	if (displayOnlyMesh.isValid()){
		displayOnlyMesh.fixEditorVisibility();
		displayOnlyMesh.convertToInstanceComponent();
	}

	if (collisionMesh.isValid()){
		collisionMesh.fixEditorVisibility();
		collisionMesh.convertToInstanceComponent();
	}

	if (rootObject.isValid() && !createdRootActor){
		/*
		Well... in this case we're rebuilding as components and there's no dummy to replicate unity name.
		So we rename the component.
		*/
		rootObject.setNameOrLabel(jsonGameObj.ueName);
	}

	check((spawnAsComponents && rootObject.hasComponent()) 
		|| (!spawnAsComponents && rootObject.hasActor()));

	return rootObject;
}

ImportedObject GeometryComponentBuilder::processStaticMesh(ImportContext &workData, const JsonGameObject &jsonGameObj, 
		ImportedObject *parentObject, const FString& folderPath, const JsonCollider *colliderData, bool spawnAsComponent, 
		OuterCreatorCallback outerCreator,
		//UObject *outer,
		JsonImporter *importer){
	using namespace UnrealUtilities;
	if (!jsonGameObj.hasMesh())
		return ImportedObject();

	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	AStaticMeshActor *meshActor = nullptr;
	UStaticMeshComponent *meshComp = nullptr;

	//
	if (spawnAsComponent){
		check(outerCreator);
		UObject* curOuter = outerCreator ? outerCreator(): nullptr;//outer ? outer: GetTransientPackage();
		check(curOuter);
		/*if (curOuter == nullptr)
			curOuter = GetTransientPackage();*/
		meshComp = NewObject<UStaticMeshComponent>(curOuter);
		meshComp->SetWorldTransform(transform);
	}
	else{
		//I wonder why it is "spawn" here and Add everywhere else. But whatever.
		meshActor = workData.world->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), transform, spawnParams);
		if (!meshActor){
			UE_LOG(JsonLog, Warning, TEXT("Couldn ot spawn mesh actor"));
			return ImportedObject();
		}

		meshActor->SetActorLabel(jsonGameObj.ueName, true);
		meshComp = meshActor->GetStaticMeshComponent();
	}

	///This is awkward. Previously we couldd attempt loading the mesh prior to building the actor, but now...
	if (!configureStaticMeshComponent(workData, meshComp, jsonGameObj, true, colliderData, importer)){
		UE_LOG(JsonLog, Warning, TEXT("Configuration of static mesh component failed on object '%s'(%d)"), *jsonGameObj.name, jsonGameObj.id);
		//return ImportedObject(meshActor);
	}

	const auto* renderer = jsonGameObj.getFirstRenderer();
	if (renderer){
		if (renderer->castsShadowsOnly()){
			if (meshActor){
				meshActor->SetActorHiddenInGame(true);//this doesn't seem to do anything? (-_-)
			}
			if (meshComp){
				meshComp->bCastHiddenShadow = true;
				meshComp->bHiddenInGame = true;
			}
		}
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("First renderer not found on %s(%d)"), *jsonGameObj.name, jsonGameObj.id);
	}

	auto result = meshActor ? ImportedObject(meshActor) : ImportedObject(meshComp);

	if (meshActor)
		meshActor->MarkComponentsRenderStateDirty();

	result.setNameOrLabel(jsonGameObj.ueName);
	setObjectHierarchy(result, parentObject, folderPath, workData, jsonGameObj);
	workData.registerGameObject(jsonGameObj, result);

	return result;
}

UPrimitiveComponent* GeometryComponentBuilder::processCollider(ImportContext &workData, const JsonGameObject &jsonGameObj, 
		OuterCreatorCallback outerCreator,//UObject *ownerPtr, 
		const JsonCollider &collider, JsonImporter *importer){
	using namespace UnrealUtilities;
	//trigger support...?
	UPrimitiveComponent *colliderComponent = nullptr;
	if (collider.isBoxCollider()){
		colliderComponent = createBoxCollider(outerCreator, jsonGameObj, collider);
	}
	else if (collider.isSphereCollider()){
		colliderComponent = createSphereCollider(outerCreator, jsonGameObj, collider);
	}
	else if (collider.isCapsuleCollider()){
		colliderComponent = createCapsuleCollider(outerCreator, jsonGameObj, collider);
	}
	else if (collider.isMeshCollider()){
		colliderComponent = createMeshCollider(outerCreator, jsonGameObj, collider, workData, importer);
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("Unknown or unsupported collider type \'%s\" on object \'%s\' (%d)"),
			*collider.colliderType, *jsonGameObj.name, jsonGameObj.id);
		return nullptr;//ImportedObject();
	}
	if (!colliderComponent){
		return nullptr;// ImportedObject();
	}

	colliderComponent->RegisterComponent();
	setupCommonColliderSettings(workData, colliderComponent, jsonGameObj, collider);

	return colliderComponent;// ImportedObject(colliderComponent);
}

UBoxComponent* GeometryComponentBuilder::createBoxCollider(OuterCreatorCallback ownerCallback, const JsonGameObject &jsonGameObj, const JsonCollider &collider){
	using namespace UnrealUtilities;
	check(ownerCallback);
	UObject* ownerPtr = ownerCallback();
	check(ownerPtr != nullptr);
	auto *boxComponent = NewObject<UBoxComponent>(ownerPtr, UBoxComponent::StaticClass());

	auto centerAdjust = unityPosToUe(collider.center);

	boxComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	boxComponent->SetMobility(jsonGameObj.getUnrealMobility());
	boxComponent->SetBoxExtent(unitySizeToUe(collider.size) * 0.5f);
	return boxComponent;
}

USphereComponent* GeometryComponentBuilder::createSphereCollider(OuterCreatorCallback ownerCallback, const JsonGameObject &jsonGameObj, const JsonCollider &collider){
	using namespace UnrealUtilities;
	check(ownerCallback);
	UObject* ownerPtr = ownerCallback();
	check(ownerPtr != nullptr);
	auto *sphereComponent = NewObject<USphereComponent>(ownerPtr, USphereComponent::StaticClass());
	sphereComponent->SetWorldTransform(jsonGameObj.getUnrealTransform(collider.center));
	sphereComponent->SetMobility(jsonGameObj.getUnrealMobility());
	sphereComponent->SetSphereRadius(unityDistanceToUe(collider.radius));
	return sphereComponent;
}

UCapsuleComponent* GeometryComponentBuilder::createCapsuleCollider(OuterCreatorCallback ownerCallback, const JsonGameObject &jsonGameObj, const JsonCollider &collider){
	using namespace UnrealUtilities;
	check(ownerCallback);
	UObject* ownerPtr = ownerCallback();
	check(ownerPtr != nullptr);
	auto *capsule = NewObject<UCapsuleComponent>(ownerPtr, UCapsuleComponent::StaticClass());

	FMatrix capsuleMatrix = FMatrix::Identity;
	capsuleMatrix.SetOrigin(collider.center);

	//auto capsuleTransform = jsonGameObj.getUnrealTransform(collider.center);
	//capsule->SetAxis... //Erm? No axis control?
	switch (collider.direction){
	case(JsonCollider::XAxis):{
		auto xAxis = FVector(0.0f, -1.0f, 0.0f);
		auto yAxis = FVector(1.0f, 0.0f, 0.0f);
		auto zAxis = FVector(0.0f, 0.0f, 1.0f);
		capsuleMatrix.SetAxes(&xAxis, &yAxis, &zAxis);
		break;
	}
	case(JsonCollider::YAxis):{
		//nothing.
		break;
	}
	case(JsonCollider::ZAxis):{
		auto xAxis = FVector(1.0f, 0.0f, 0.0f);
		auto yAxis = FVector(0.0f, 0.0f, 1.0f);
		auto zAxis = FVector(0.0f, -1.0f, 0.0f);
		capsuleMatrix.SetAxes(&xAxis, &yAxis, &zAxis);
		break;
	}
	}

	FMatrix capsuleCombinedMatrix = capsuleMatrix * jsonGameObj.worldMatrix;
	FTransform capsuleTransform;
	capsuleTransform.SetFromMatrix(unityWorldToUe(capsuleCombinedMatrix));

	capsule->SetWorldTransform(capsuleTransform);
	capsule->SetMobility(jsonGameObj.getUnrealMobility());

	capsule->SetCapsuleHalfHeight(unityDistanceToUe(collider.height*0.5f));
	capsule->SetCapsuleRadius(unityDistanceToUe(collider.radius));

	return capsule;
}

UStaticMeshComponent* GeometryComponentBuilder::createMeshCollider(OuterCreatorCallback ownerCallback, 
		const JsonGameObject &jsonGameObj, const JsonCollider &collider, ImportContext &workData, JsonImporter *importer){
	using namespace UnrealUtilities;
	check(ownerCallback);
	UObject* ownerPtr = ownerCallback();
	check(importer);
	check(ownerPtr);
	auto *meshComponent = NewObject<UStaticMeshComponent>(ownerPtr, UStaticMeshComponent::StaticClass());

	configureStaticMeshComponent(workData, meshComponent, jsonGameObj, false, &collider, importer);

	meshComponent->SetWorldTransform(jsonGameObj.getUnrealTransform());//no center for the static mesh
	meshComponent->SetMobility(jsonGameObj.getUnrealMobility());

	return meshComponent;
}


void GeometryComponentBuilder::setupCommonColliderSettings(const ImportContext &workData, UPrimitiveComponent *dstCollider, const JsonGameObject &jsonGameObj, const JsonCollider &collider){
	check(dstCollider);
	if (collider.trigger){
		dstCollider->SetCollisionProfileName(FName("OverlapAll"));//this is not available as a constant 
		dstCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else{
		dstCollider->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		dstCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	const auto *rigBody = workData.locateRigidbody(jsonGameObj);
	//Hmm. Shoudl this even be here?
	if (rigBody){
		//bool compoundColliderRoot = workData.isCompoundRigidbodyRootCollider(jsonGameObj);//TODO: Cache this?
		//int rootIndex = workData.

		bool physicsEnabled = !rigBody->isKinematic;
		if (rigBody){
			//It is necessary to do this in order to utilize body welding on compound colliders
			auto compoundColliderChild = true;
			if (workData.isCompoundRigidbodyRootCollider(jsonGameObj)){
				if (jsonGameObj.findSuitableRootColliderIndex() == collider.colliderIndex){
					compoundColliderChild = false;
				}
			}
			physicsEnabled = physicsEnabled && !compoundColliderChild;
		}

		//kinematic rigidbodies?
		dstCollider->SetSimulatePhysics(physicsEnabled);
		dstCollider->SetMassOverrideInKg(NAME_None, rigBody->mass, true);
		dstCollider->SetEnableGravity(rigBody->useGravity);

		auto ccd = rigBody->usesContinuousCollision() 
			|| rigBody->usesContinuousDynamicCollision() 
			|| rigBody->usesContinuousSpeculativeCollision();

		dstCollider->SetAllUseCCD(ccd);

		//Well, we can't set rigidbody drag and angular drag, it seems.
	}
}

bool GeometryComponentBuilder::configureStaticMeshComponent(ImportContext &workData, UStaticMeshComponent *meshComp, 
		const JsonGameObject &jsonGameObj, bool configForRender, const JsonCollider *collider, JsonImporter *importer){
	using namespace JsonObjects;
	check(meshComp);
	check(importer);

	/*
	if (!jsonGameObj.hasRenderers()){
	UE_LOG(JsonLog, Warning, TEXT("Renderer not found on %s(%d), cannot create mesh"), *jsonGameObj.ueName, jsonGameObj.id);
	return false;
	}
	check(jsonGameObj.renderers.Num() > 0);
	*/
	if (!jsonGameObj.hasRenderers() && configForRender){
		UE_LOG(JsonLog, Warning, TEXT("Renderer not found on %s(%d), while the mesh was being configured for rendering"), *jsonGameObj.ueName, jsonGameObj.id);
	}

	/*
	We're now utilizing static mesh for both visible geometry and colliders. 
	Colliders might not match the geometry.
	If collider is provided, its meshId takes priority.
	*/
	bool collisionOnlyMesh = false;
	//JsonId meshId = jsonGameObj.meshId;
	ResId meshId = jsonGameObj.meshId;
	//if (collider && isValidId(collider->meshId) && !configForRender){
	if (collider && collider->meshId.isValid() && !configForRender){
		meshId = collider->meshId;
		collisionOnlyMesh = true;
	}

	auto foundMeshPath = importer->findMeshPath(meshId);
	//auto meshPath = meshIdMap[meshId];
	//UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), *meshPath);
	if (!foundMeshPath){
		UE_LOG(JsonLog, Error, TEXT("Mesh path not found for id %d"), meshId.id);
	}
	UE_LOG(JsonLog, Log, TEXT("Mesh path: %s"), **foundMeshPath);
	auto meshPath = *foundMeshPath;

	auto *meshObject = LoadObject<UStaticMesh>(0, *meshPath);
	if (!meshObject){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mesh %s"), *meshPath);
		return false;
	}

	meshComp->SetStaticMesh(meshObject);
	meshComp->SetMobility(jsonGameObj.getUnrealMobility());

	if (collider){
		setupCommonColliderSettings(workData, meshComp, jsonGameObj, *collider);
	}
	else{
		meshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (!configForRender){
		meshComp->bHiddenInGame = true;//Is this the right way, though...
		meshComp->SetCastShadow(false);
		meshComp->SetVisibility(false);
		return true;
	}

	if (!collisionOnlyMesh){
		const auto &renderer = jsonGameObj.renderers[0];
		auto materials = jsonGameObj.getFirstMaterials();

		bool emissiveMesh = false;
		if (materials.Num() > 0){
			for (int i = 0; i < materials.Num(); i++){
				auto matId = materials[i];

				auto *jsonMat = importer->getJsonMaterial(matId);
				if (jsonMat && (jsonMat->isEmissive()))
					emissiveMesh = true;

				auto material = importer->loadMaterialInterface(matId);
				meshComp->SetMaterial(i, material);
			}
		}

		logValue("hasShadows", renderer.castsShadows());
		logValue("twoSidedShadows", renderer.castsTwoSidedShadows());

		meshComp->SetCastShadow(renderer.castsShadows());
		meshComp->bCastShadowAsTwoSided = renderer.castsTwoSidedShadows();//twoSidedShadows;

		if (emissiveMesh)
			meshComp->LightmassSettings.bUseEmissiveForStaticLighting = true;
	}

	return true;
}
