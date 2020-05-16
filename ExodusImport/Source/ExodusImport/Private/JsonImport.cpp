// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "JsonImportStyle.h"
#include "JsonImportCommands.h"

#include "JsonImporter.h"

#include "LevelEditor.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "DesktopPlatformModule.h"

#include "JsonLog.h"

#include "Tests/LandscapeTest.h"
#include "Tests/OutlinerTest.h"
#include "Tests/SkinMeshTest.h"
#include "Tests/PluginDebugTest.h"

#include "LocTextNamespace.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

static const FName JsonImportTabName("JsonImport");

void FJsonImportModule::StartupModule(){
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	LOCTEXT("Importing textures", "Importing textures");
	FJsonImportStyle::Initialize();
	FJsonImportStyle::ReloadTextures();

	FJsonImportCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginImportAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginImportButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginDebugAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginDebugButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginTestButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginLandscapeTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginLandscapeTestButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginSkinMeshTestAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginSkinMeshTestButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, 
			FMenuExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, 
			FToolBarExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FJsonImportModule::ShutdownModule(){
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FJsonImportStyle::Shutdown();

	FJsonImportCommands::Unregister();
}

void FJsonImportModule::PluginDebugButtonClicked(){
	PluginDebugTest test;
	test.run();
}

void FJsonImportModule::PluginTestButtonClicked(){
	OutlinerTest test;
	test.run();
}

void FJsonImportModule::PluginLandscapeTestButtonClicked(){
	LandscapeTest test;
	test.run();
}

void FJsonImportModule::PluginImportButtonClicked(){
	// Put your "OnButtonClicked" stuff here
	FStringArray files;

	if (!FDesktopPlatformModule::Get()->OpenFileDialog(0,
			FString("Open JSON File"), FString(), FString(), FString("JSON file|*.json"), EFileDialogFlags::None, files))
			return;

	if (!files.Num()){
			UE_LOG(JsonLog, Log, TEXT("No file to open"));
			return;
	}

	auto filename = FPaths::ConvertRelativePathToFull(files[0]);
	JsonImporter importer;
	importer.importProject(filename);
	//importer.importScene(filename);
}

void FJsonImportModule::PluginSkinMeshTestButtonClicked(){
	SkinMeshTest skinTest;
	skinTest.run();
}

void FJsonImportModule::AddMenuExtension(FMenuBuilder& Builder){
	Builder.AddMenuEntry(FJsonImportCommands::Get().PluginImportAction);
}

void FJsonImportModule::AddToolbarExtension(FToolBarBuilder& Builder){
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginImportAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginDebugAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginSkinMeshTestAction);
	/*
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginTestAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginLandscapeTestAction);
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginSkinMeshTestAction);
	*/
}

IMPLEMENT_MODULE(FJsonImportModule, ExodusImport)

#undef LOCTEXT_NAMESPACE
