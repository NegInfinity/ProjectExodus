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

#define LOCTEXT_NAMESPACE "FJsonImportModule"

static const FName JsonImportTabName("JsonImport");

DEFINE_LOG_CATEGORY(JsonLog);

void FJsonImportModule::StartupModule(){
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	LOCTEXT("Importing textures", "Importing textures");
	FJsonImportStyle::Initialize();
	FJsonImportStyle::ReloadTextures();

	FJsonImportCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FJsonImportCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FJsonImportModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FJsonImportModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FJsonImportModule::ShutdownModule(){
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FJsonImportStyle::Shutdown();

	FJsonImportCommands::Unregister();
}


void FJsonImportModule::PluginButtonClicked(){
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

void FJsonImportModule::AddMenuExtension(FMenuBuilder& Builder){
	Builder.AddMenuEntry(FJsonImportCommands::Get().PluginAction);
}

void FJsonImportModule::AddToolbarExtension(FToolBarBuilder& Builder){
	Builder.AddToolBarButton(FJsonImportCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJsonImportModule, JsonImport)