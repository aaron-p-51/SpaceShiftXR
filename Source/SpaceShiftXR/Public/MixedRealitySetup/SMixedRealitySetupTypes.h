#pragma once

#include "CoreMinimal.h"
#include "SMixedRealitySetupTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SMixedRealitySetup, Log, All);

/**
 * States for mixed reality setup
 */
UENUM(BlueprintType)
enum class ESetupState : uint8
{
	ESS_NotStarted		UMETA(DisplayName = "NotStarted"),
	ESS_Running			UMETA(DisplayName = "Running"),
	ESS_Failed			UMETA(DisplayName = "Failed"),
	ESS_Complete		UMETA(DisplayName = "Complete"),

	ESS_MAX				UMETA(DisplayName = "Defualt MAX")
};


/**
 * Commands for mixed reality setup
 */
UENUM(BlueprintType)
enum class ESetupCommand : uint8
{
	ESCRequestUseSceneData			UMETA(DisplayName = "RequestUseSceneData"),
	ESCRunSceneCapture				UMETA(DisplayName = "RunSceneCapture"),
	ESCClearScene					UMETA(DisplayName = "ClearScene"),
	ESCLoadSceneFromDevice			UMETA(DisplayName = "LoadSceneFromDevice"),
	ESCLoadPresetScene				UMETA(DisplayName = "LoadPresetScene"),
	ESCLoadGlobalMeshFromDevice		UMETA(DisplayName = "LoadGlobalMeshFromDevice"),
	ESCSetGlobalMeshVisible			UMETA(DisplayName = "SetGlobalMeshVisible"),
	ESCSetGlobalMeshHidden			UMETA(DisplayName = "SetGlobalMeshHidden"),
	ESCEnableGlobalMeshCollision	UMETA(DisplayName = "EnableGlobalMeshCollision"),
	ESCDisableGlobalMeshCollision	UMETA(DisplayName = "DisableGlobalMeshCollision"),
	ESCApplyTextureToWalls			UMETA(DisplayName = "ApplyTextureToWalls"),

	ESC_MAX							UMETA(DisplayName = "Default MAX")
};


/**
 * Preset rooms for mix reality testing in editor
 */
UENUM(BlueprintType)
enum class EPresetRoom : uint8
{
	EPR_Bedroom00		UMETA(DisplayName = "Bedroom00"),
	EPR_Bedroom01		UMETA(DisplayName = "Bedroom01"),
	EPR_Bedroom02		UMETA(DisplayName = "Bedroom02"),
	EPR_Bedroom03		UMETA(DisplayName = "Bedroom03"),
	EPR_Bedroom04		UMETA(DisplayName = "Bedroom04"),
	EPR_Bedroom05		UMETA(DisplayName = "Bedroom05"),
	EPR_Bedroom06		UMETA(DisplayName = "Bedroom06"),
	EPR_Bedroom07		UMETA(DisplayName = "Bedroom07"),

	EPR_LivingRoom00	UMETA(DisplayName = "LivingRoom00"),
	EPR_LivingRoom01	UMETA(DisplayName = "LivingRoom01"),
	EPR_LivingRoom02	UMETA(DisplayName = "LivingRoom02"),
	EPR_LivingRoom03	UMETA(DisplayName = "LivingRoom03"),
	EPR_LivingRoom04	UMETA(DisplayName = "LivingRoom04"),
	EPR_LivingRoom05	UMETA(DisplayName = "LivingRoom05"),
	EPR_LivingRoom06	UMETA(DisplayName = "LivingRoom06"),
	EPR_LivingRoom07	UMETA(DisplayName = "LivingRoom07"),

	EPR_Office00		UMETA(DisplayName = "Office00"),
	EPR_Office01		UMETA(DisplayName = "Office01"),
	EPR_Office02		UMETA(DisplayName = "Office02"),
	EPR_Office03		UMETA(DisplayName = "Office03"),
	EPR_Office04		UMETA(DisplayName = "Office04"),
	EPR_Office05		UMETA(DisplayName = "Office05"),
	EPR_Office06		UMETA(DisplayName = "Office06"),
	EPR_Office07		UMETA(DisplayName = "Office07"),
	EPR_Office08		UMETA(DisplayName = "Office08"),
	EPR_Office09		UMETA(DisplayName = "Office09"),

	ESR_TrippingHazard	UMETA(DisplayName = "TrippingHazard"),
	ESR_ExtraWide		UMETA(DisplayName = "ExtraWide"),
	ESR_RecRoom			UMETA(DisplayName = "RecRoom"),
	ESR_EmptySplit		UMETA(DisplayName = "EmptySplit"),
	ESR_empty			UMETA(DisplayName = "empty"),
	ESR_HardAngles		UMETA(DisplayName = "HardAngles"),
	ESR_Triangular		UMETA(DisplayName = "Triangular"),
	ESR_Cluttered		UMETA(DisplayName = "Cluttered"),

	ESR_MAX				UMETA(DisplayName = "DEFAULT_MAX")
};
