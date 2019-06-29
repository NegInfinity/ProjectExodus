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
	actor1->SetActorLabel("Root Actor");

	/*
	auto rootComponent= NewObject<USceneComponent>(actor1);
	actor1->SetRootComponent(rootComponent);
	rootComponent->SetWorldLocation(FVector(0.0f, 0.0f, 300.0f));
	rootComponent->RegisterComponent();

	auto subComponent = NewObject<USceneComponent>(actor1);
	subComponent->SetWorldLocation(FVector(0.0f, 0.0f, 600.0f));
	subComponent->AttachToComponent(rootComponent, FAttachmentTransformRules::KeepWorldTransform);
	subComponent->RegisterComponent();
	*/

	//auto lightComp = NewObject<UPointLightComponent>(actor1);
	auto lightComp = NewObject<UDirectionalLightComponent>(actor1);
	FMatrix lightMatrix = FMatrix::Identity;
	lightMatrix.SetOrigin(FVector(100.0f, 200.0f, 300.0f));
	FTransform lightTransform(lightMatrix);

	lightComp->SetWorldTransform(lightTransform);

	actor1->SetRootComponent(lightComp);
	//lightComp->AttachToComponent(subComponent, FAttachmentTransformRules::KeepWorldTransform);
	lightComp->RegisterComponent();

	//actor1->AddInstanceComponent(subComponent);
	actor1->AddInstanceComponent(lightComp);
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