// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "JsonTypes.h"

class FToolBarBuilder;
class FMenuBuilder;

class FJsonImportModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginImportButtonClicked();
	void PluginTestButtonClicked();
	void PluginDebugButtonClicked();
	void PluginLandscapeTestButtonClicked();
	void PluginSkinMeshTestButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
