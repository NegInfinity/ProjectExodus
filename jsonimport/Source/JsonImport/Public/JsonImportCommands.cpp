// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"
#include "JsonImportCommands.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

void FJsonImportCommands::RegisterCommands()
{
	UI_COMMAND(PluginImportAction, "JsonImport", "Execute JsonImport action", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginTestAction, "Exodus object test", "Run JsonImport Test", 
		EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginLandscapeTestAction, "Exodus landscape crash test", "Run JsonImport landscape test", 
		EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
