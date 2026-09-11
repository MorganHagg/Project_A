// Editor-only dev tool: scans /Game/Abilities for every UAbility Blueprint class and registers
// each one's identity tag (Ability.<Name>), so DefaultGameplayTags.ini doesn't have to be
// hand-edited every time a new ability is authored. The shared, ability-agnostic event-category
// tags (Event.Cast/Finish/TargetHit/Overlap/Crit - see UAbility::ComposeEventTags) are fixed and
// don't multiply per ability, so they're maintained by hand in DefaultGameplayTags.ini instead.
// Deliberately kept out of Ability.h/.cpp so UAbility itself has no dependency on the editor-only
// GameplayTagsEditor/AssetRegistry modules.
#if WITH_EDITOR

#include "Ability.h"
#include "HAL/IConsoleManager.h"
#include "GameplayTagsEditorModule.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Misc/PackageName.h"

namespace
{
	void SyncOneAbility(const UAbility* AbilityCDO, IGameplayTagsEditorModule& TagsEditor)
	{
		const FName AbilityName = AbilityCDO->GetAbilityName();
		if (AbilityName == FName("NO_NAME_ABILITY"))
		{
			UE_LOG(LogTemp, Error, TEXT("Abilities.SyncTags: '%s' has no AbilityName set yet - skipped."), *AbilityCDO->GetClass()->GetName());
			return;
		}

		const FString NewTag = FString::Printf(TEXT("Ability.%s"), *AbilityName.ToString());

		// Skip tags that already resolve - AddNewGameplayTagToINI logs an Error for an
		// already-existing tag, which would otherwise spam the log on every re-run.
		if (!FGameplayTag::RequestGameplayTag(FName(*NewTag), /*ErrorIfNotFound=*/false).IsValid())
		{
			TagsEditor.AddNewGameplayTagToINI(NewTag);
		}

		UE_LOG(LogTemp, Log, TEXT("Abilities.SyncTags: registered identity tag for Ability.%s."), *AbilityName.ToString());
	}

	void SyncAbilityTags(const TArray<FString>& Args)
	{
		IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();

		TArray<FAssetData> AssetDatas;
		AssetRegistry.GetAssetsByPath(FName("/Game/Abilities"), AssetDatas, /*bRecursive=*/true);

		IGameplayTagsEditorModule& TagsEditor = IGameplayTagsEditorModule::Get();

		int32 SyncedCount = 0;
		for (const FAssetData& AssetData : AssetDatas)
		{
			if (AssetData.AssetClassPath != UBlueprint::StaticClass()->GetClassPathName())
			{
				continue;
			}

			FString GeneratedClassPath;
			if (!AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassPath))
			{
				continue;
			}
			GeneratedClassPath = FPackageName::ExportTextPathToObjectPath(GeneratedClassPath);

			UClass* AbilityClass = LoadObject<UClass>(nullptr, *GeneratedClassPath);
			if (!AbilityClass || !AbilityClass->IsChildOf(UAbility::StaticClass()))
			{
				continue;
			}

			SyncOneAbility(GetDefault<UAbility>(AbilityClass), TagsEditor);
			++SyncedCount;
		}

		UE_LOG(LogTemp, Log, TEXT("Abilities.SyncTags: scanned /Game/Abilities, synced %d Ability class(es)."), SyncedCount);
	}

	static FAutoConsoleCommand SyncAbilityTagsCommand(
		TEXT("Abilities.SyncTags"),
		TEXT("Scans /Game/Abilities for every UAbility Blueprint class and registers each one's Ability.<Name> identity tag."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SyncAbilityTags));
}

#endif // WITH_EDITOR
