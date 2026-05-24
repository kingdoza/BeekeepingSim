// Copyright Epic Games, Inc. All Rights Reserved.

#include "BeekeepingSim.h"
#include "Modules/ModuleManager.h"
#if WITH_EDITOR
#include "PropertyEditorModule.h"
#include "WorldActors/BeehiveDualSwarmActorCustomization.h"
#endif

class FBeekeepingSimModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
#if WITH_EDITOR
		FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditor.RegisterCustomClassLayout("BeehiveDualSwarmActor", FOnGetDetailCustomizationInstance::CreateStatic(&FBeehiveDualSwarmActorCustomization::MakeInstance));
		PropertyEditor.RegisterCustomClassLayout("NiagaraComponent", FOnGetDetailCustomizationInstance::CreateStatic(&FBeehiveDualSwarmNiagaraComponentCustomization::MakeInstance));
		PropertyEditor.NotifyCustomizationModuleChanged();
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_EDITOR
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyEditor = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditor.UnregisterCustomClassLayout("BeehiveDualSwarmActor");
			PropertyEditor.UnregisterCustomClassLayout("NiagaraComponent");
			PropertyEditor.NotifyCustomizationModuleChanged();
		}
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FBeekeepingSimModule, BeekeepingSim, "BeekeepingSim");

DEFINE_LOG_CATEGORY(LogBeekeepingSim)
