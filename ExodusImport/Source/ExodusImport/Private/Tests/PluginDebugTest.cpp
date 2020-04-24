#include "JsonImportPrivatePCH.h"
#include "PluginDebugTest.h"
#include "Editor.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/DrawSphereComponent.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "UnrealUtilities.h"

void PluginDebugTest::run(){
	UE_LOG(JsonLog, Log, TEXT("Debug function started: testing blueprint creation"));
	auto world = GEditor->GetEditorWorldContext().World();

	FTransform transform;
	transform.SetFromMatrix(FMatrix::Identity);
	auto actor1 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor1->SetActorLabel("Root Actor 1");

	/*
	So. To summarize it:

	Attaching component without outer change makes the component disappear. Need to change outer manually using rename() so it persists
	Registering component prior to rename() with owner change causes rename() to fail. Need to unregister first, apparently.

	This is going to be painful.
	*/

	auto comp1 = NewObject<USceneComponent>();
	comp1->Rename(TEXT("comp1"));
	comp1->SetWorldLocation(FVector(0.0f, 0.0f, 100.0f));

	auto comp2 = NewObject<USceneComponent>();
	comp2->Rename(TEXT("comp2"));
	comp2->SetWorldLocation(FVector(0.0f, 0.0f, 200.0f));
	comp2->AttachToComponent(comp1, FAttachmentTransformRules::KeepWorldTransform);

	auto comp3 = NewObject<USceneComponent>();
	comp3->Rename(TEXT("comp3"));
	comp3->SetWorldLocation(FVector(0.0f, 0.0f, 300.0f));
	comp3->AttachToComponent(comp2, FAttachmentTransformRules::KeepWorldTransform);

	comp1->Rename(0, actor1);
	comp1->RegisterComponent();
	actor1->SetRootComponent(comp1);

	/*
	comp2->RegisterComponent();
	comp3->RegisterComponent();

	comp2->UnregisterComponent();
	comp3->UnregisterComponent();
	*/

	comp2->Rename(0, actor1);
	comp3->Rename(0, actor1);

	comp2->RegisterComponent();
	comp3->RegisterComponent();

	actor1->AddInstanceComponent(comp1);
	actor1->AddInstanceComponent(comp2);
	actor1->AddInstanceComponent(comp3);

	auto lightComp2 = NewObject<UPointLightComponent>();
	{
		FMatrix lightMatrix = FMatrix::Identity;
		auto worldPos = FVector(0.0f, 0.0f, 600.0f);
		lightMatrix.SetOrigin(worldPos);
		FTransform lightTransform(lightMatrix);
		lightComp2->SetWorldTransform(lightTransform);
	}
	lightComp2->AttachToComponent(comp3, FAttachmentTransformRules::KeepWorldTransform);
	actor1->AddInstanceComponent(lightComp2);
	lightComp2->Rename(0, actor1);
	lightComp2->RegisterComponent();

		//auto subComponent = NewObject<USceneComponent>(actor1);
	/*
	subComponent->SetWorldLocation(FVector(0.0f, 0.0f, 600.0f));
	auto subComponent = NewObject<USceneComponent>();
	subComponent->SetWorldLocation(FVector(0.0f, 0.0f, 600.0f));
	subComponent->AttachToComponent(lightComp, FAttachmentTransformRules::KeepWorldTransform);

	auto lightComp2 = NewObject<UPointLightComponent>();
	{
		FMatrix lightMatrix = FMatrix::Identity;
		lightMatrix.SetOrigin(FVector(400.0f, 500.0f, 600.0f));
		FTransform lightTransform(lightMatrix);
		lightComp2->SetWorldTransform(lightTransform);
	}
	lightComp2->AttachToComponent(subComponent, FAttachmentTransformRules::KeepRelativeTransform);
	lightComp2->RegisterComponent();
	lightComp2->Rename(0, actor1, ERenameFlags::

	subComponent->Rename(0, actor1);
	subComponent->RegisterComponent();

	actor1->AddInstanceComponent(subComponent);
	actor1->AddInstanceComponent(lightComp);
	*/
}

#if 0
void PluginDebugTest::run(){
	UE_LOG(JsonLog, Log, TEXT("Debug function started"));
	auto world = GEditor->GetEditorWorldContext().World();

	FTransform transform;
	transform.SetFromMatrix(FMatrix::Identity);
	auto actor1 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor1->SetActorLabel("Actor 1");

	//auto physBody1 = NewObject<UPrimitiveComponent>(actor1);
	auto body1= NewObject<UDrawSphereComponent>(actor1);
	actor1->SetRootComponent(body1);
	body1->SetSphereRadius(100.0f);
	body1->SetWorldLocation(FVector(0.0f, 0.0f, 300.0f));
	body1->RegisterComponent();

	auto body2 = NewObject<UDrawSphereComponent>(actor1);
	//actor1->SetRootComponent(body2);
	body2->SetWorldLocation(FVector(100.0f, 0.0f, 300.0f));
	body2->AttachToComponent(body1, FAttachmentTransformRules::KeepWorldTransform);
	body2->SetSphereRadius(100.0f);
	body2->RegisterComponent();

	actor1->AddInstanceComponent(body1);
	actor1->AddInstanceComponent(body2);

	//auto physBody2 = NewObject<UBox
	//auto scene1 = NewObject<USceneComponent>(actor1, USceneComponent::StaticClass());
	/*
	auto scene1 = NewObject<USceneComponent>();//Will this make it move to another outer?
	scene1->Rename(nullptr, actor1);
	actor1->SetRootComponent(scene1);
	scene1->RegisterComponent();
	actor1->AddInstanceComponent(scene1);
	//scene1->SetFlags(EObjectFlags::RF_ArchetypeObject);

	auto actor2 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor2->SetActorLabel("Actor 2");
	UE_LOG(JsonLog, Log, TEXT("Debug function end"));


	auto scene2 = NewObject<USphereComponent>(actor2, USphereComponent::StaticClass());
	actor2->SetRootComponent(scene2);
	scene2->RegisterComponent();
	*/
}
#endif

#if 0
void PluginDebugTest::run(){
	UE_LOG(JsonLog, Log, TEXT("Debug function started"));
	auto world = GEditor->GetEditorWorldContext().World();

	FTransform transform;
	transform.SetFromMatrix(FMatrix::Identity);
	auto actor1 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor1->SetActorLabel("Actor 1");

	//auto scene1 = NewObject<USceneComponent>(actor1, USceneComponent::StaticClass());
	auto scene1 = NewObject<USceneComponent>();//Will this make it move to another outer?
	scene1->Rename(nullptr, actor1);
	actor1->SetRootComponent(scene1);
	scene1->RegisterComponent();
	actor1->AddInstanceComponent(scene1);
	//scene1->SetFlags(EObjectFlags::RF_ArchetypeObject);

	auto actor2 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor2->SetActorLabel("Actor 2");
	UE_LOG(JsonLog, Log, TEXT("Debug function end"));

	auto scene2 = NewObject<USphereComponent>(actor2, USphereComponent::StaticClass());
	actor2->SetRootComponent(scene2);
	scene2->RegisterComponent();
}
#endif