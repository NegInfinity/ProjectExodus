// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "JsonImportStyle.h"

class FJsonImportCommands : public TCommands<FJsonImportCommands>
{
public:

	FJsonImportCommands()
		: TCommands<FJsonImportCommands>(
			TEXT("ExodusImport"), 
			NSLOCTEXT("Contexts", "ExodusImport", "ExodusImport Plugin"), 
			NAME_None, FJsonImportStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginImportAction;
	TSharedPtr< FUICommandInfo > PluginDebugAction;
	TSharedPtr< FUICommandInfo > PluginTestAction;
	TSharedPtr< FUICommandInfo > PluginLandscapeTestAction;
	TSharedPtr< FUICommandInfo > PluginSkinMeshTestAction;
};
