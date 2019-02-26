#include "JsonImportPrivatePCH.h"
#include "PluginDebugTest.h"
#include "Editor.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

void PluginDebugTest::run(){
	UE_LOG(JsonLog, Log, TEXT("Debug function started"));
	auto world = GEditor->GetEditorWorldContext().World();

	FTransform transform;
	transform.SetFromMatrix(FMatrix::Identity);
	auto actor1 = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	actor1->SetActorLabel("Actor 1");

	auto scene1 = NewObject<USceneComponent>(actor1, USceneComponent::StaticClass());
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
