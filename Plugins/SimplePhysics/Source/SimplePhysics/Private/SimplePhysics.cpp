// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimplePhysics.h"

#include "SimplePhysics_Settings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FSimplePhysicsModule"

void FSimplePhysicsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "SimplePhysics_Settings",
			LOCTEXT("RuntimeSettingsName", "Simple Physics"), LOCTEXT("RuntimeSettingsDescription", "Configure my setting"),
			GetMutableDefault<USimplePhysics_Settings>());
	}
}

void FSimplePhysicsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "SimplePhysics_Settings");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimplePhysicsModule, SimplePhysics)