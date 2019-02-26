// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"
#include "JsonImportCommands.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

void FJsonImportCommands::RegisterCommands()
{
	UI_COMMAND(PluginImportAction, "ExodusImport", "Execute ExodusImport action", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginDebugAction, "ExodusImport Debug", "Execute ExodusImport debug action (do not use)",
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginTestAction, "Exodus object test", "Run ExodusImport Test",
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginLandscapeTestAction, "Exodus landscape crash test", "Run ExodusImport landscape test", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginSkinMeshTestAction, "SkinMesh Test", "Run skin mesh test", 
		EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
