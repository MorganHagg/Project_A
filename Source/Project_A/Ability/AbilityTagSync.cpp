// Editor-only dev tool: scans /Game/Abilities for every AAbility Blueprint class (e.g.
// Projectile_Fireball, AA_Firewall) and registers each one's identity tag (Ability.<Name>), and
// scans the project for every UOverTimeEffectDataAsset and registers its identity tag
// (Effect.<Name>), so DefaultGameplayTags.ini doesn't have to be hand-edited every time a new
// ability or over-time effect is authored.
// The shared, ability-agnostic event-category tags (Event.Cast/Finish/TargetHit/Overlap/Crit,
// Event.Applied/Tick/Ended/Removed/UnitDeath - see AAbility::ComposeEventTags and
// UOverTimeEffect::ReportEvent) are fixed and don't multiply per ability, so they're maintained
// by hand in DefaultGameplayTags.ini instead.
// Deliberately kept out of Ability.h/.cpp so AAbility itself has no dependency on the editor-only
// GameplayTagsEditor/AssetRegistry modules.
#if WITH_EDITOR

#include "Ability.h"
#include "../DataAsset/OverTimeEffectDataAsset.h"
#include "HAL/IConsoleManager.h"
#include "GameplayTagsEditorModule.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Misc/PackageName.h"

namespace
{
	// Registers "<Prefix>.<Name>" unless Name is still the unset placeholder. OwnerName is only
	// used for logging.
	void SyncOneTag(const TCHAR* Prefix, FName Name, FName UnsetName, const FString& OwnerName, IGameplayTagsEditorModule& TagsEditor)
	{
		if (Name == UnsetName)
		{
			UE_LOG(LogTemp, Error, TEXT("Abilities.SyncTags: '%s' has no name set yet - skipped."), *OwnerName);
			return;
		}

		const FString NewTag = FString::Printf(TEXT("%s.%s"), Prefix, *Name.ToString());

		// Skip tags that already resolve - AddNewGameplayTagToINI logs an Error for an
		// already-existing tag, which would otherwise spam the log on every re-run.
		if (!FGameplayTag::RequestGameplayTag(FName(*NewTag), /*ErrorIfNotFound=*/false).IsValid())
		{
			TagsEditor.AddNewGameplayTagToINI(NewTag);
		}

		UE_LOG(LogTemp, Log, TEXT("Abilities.SyncTags: registered identity tag %s."), *NewTag);
	}

	void SyncAbilityTags(const TArray<FString>& Args)
	{
		IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
		IGameplayTagsEditorModule& TagsEditor = IGameplayTagsEditorModule::Get();

		// Abilities: AbilityName lives on each AAbility Blueprint class's defaults.
		TArray<FAssetData> AssetDatas;
		AssetRegistry.GetAssetsByPath(FName("/Game/Abilities"), AssetDatas, /*bRecursive=*/true);

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
			if (!AbilityClass || !AbilityClass->IsChildOf(AAbility::StaticClass()))
			{
				continue;
			}

			SyncOneTag(TEXT("Ability"), GetDefault<AAbility>(AbilityClass)->AbilityName, FName("NO_NAME_ABILITY"),
				AbilityClass->GetName(), TagsEditor);
			++SyncedCount;
		}

		UE_LOG(LogTemp, Log, TEXT("Abilities.SyncTags: scanned /Game/Abilities, synced %d Ability class(es)."), SyncedCount);

		// Over-time effects: EffectName lives on each data asset, wherever it is.
		TArray<FAssetData> EffectAssetDatas;
		AssetRegistry.GetAssetsByClass(UOverTimeEffectDataAsset::StaticClass()->GetClassPathName(), EffectAssetDatas, /*bSearchSubClasses=*/true);

		int32 SyncedEffectCount = 0;
		for (const FAssetData& AssetData : EffectAssetDatas)
		{
			const UOverTimeEffectDataAsset* EffectData = Cast<UOverTimeEffectDataAsset>(AssetData.GetAsset());
			if (!EffectData)
			{
				continue;
			}

			SyncOneTag(TEXT("Effect"), EffectData->EffectName, FName("NO_NAME_EFFECT"), EffectData->GetName(), TagsEditor);
			++SyncedEffectCount;
		}

		UE_LOG(LogTemp, Log, TEXT("Abilities.SyncTags: synced %d over-time effect data asset(s)."), SyncedEffectCount);
	}

	static FAutoConsoleCommand SyncAbilityTagsCommand(
		TEXT("Abilities.SyncTags"),
		TEXT("Registers the Ability.<Name> identity tag of every AAbility Blueprint class in /Game/Abilities, and the Effect.<Name> tag of every over-time effect data asset."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SyncAbilityTags));
}

#endif // WITH_EDITOR
