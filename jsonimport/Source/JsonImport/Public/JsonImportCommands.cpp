// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JsonImportPrivatePCH.h"
#include "JsonImportCommands.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

void FJsonImportCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "JsonImport", "Execute JsonImport action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
