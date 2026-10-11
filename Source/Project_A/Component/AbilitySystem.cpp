#include "AbilitySystem.h"
#include "../Ability/AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/AbilityDataAsset.h"
#include "../DataAsset/OverTimeEffectDataAsset.h"
#include "../Effect/OverTimeEffectSlot.h"
#include "AttributeComponent.h"
#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Blueprint.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

UAbilitySystem::UAbilitySystem()
{
}

void UAbilitySystem::BeginPlay()
{
	Super::BeginPlay();
	MyOwner = CastChecked<AUnitBase>(GetOwner());
}

void UAbilitySystem::InstantiateAbilities(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

#if WITH_EDITOR
	bool bFoundNamelessAbility = false;
#endif

	for (UAbilityDataAsset* AbilityData : UnitData->DefaultAbilities)
	{
		if (!AbilityData)
		{
			continue;
		}

		// One generic class now - identity/behavior come entirely from the copied DataAsset
		// values and ProductClass, not from a per-ability subclass.
		UAbilitySlot* NewAbility = NewObject<UAbilitySlot>(this);
		NewAbility->CoolDown = AbilityData->CoolDown;
		NewAbility->Cost = AbilityData->Cost;
		NewAbility->bActivateOnRelease = AbilityData->bActivateOnRelease;
		NewAbility->MaxChargeTime = AbilityData->MaxChargeTime;
		NewAbility->ProductClass = AbilityData->ProductClass;
		if (NewAbility->ProductClass)
		{
			// Identity comes from AbilityName on the Ability class's defaults - the same source
			// AAbility::BeginPlay uses, so a slot and the Abilities it spawns share one tag.
			const FName AbilityName = NewAbility->ProductClass->GetDefaultObject<AAbility>()->AbilityName;
#if WITH_EDITOR
			// In the editor a nameless ability ends PIE with a Message Log error instead of reaching
			// AAbility::BeginPlay's checkf on first cast, which would crash the editor and lose
			// unsaved work. Packaged builds still hit the checkf.
			if (GIsEditor && AbilityName == FName("NO_NAME_ABILITY"))
			{
				const UObject* NamelessSource = UBlueprint::GetBlueprintFromClass(NewAbility->ProductClass);
				FMessageLog("PIE").Error()
					->AddToken(FUObjectToken::Create(NamelessSource ? NamelessSource : NewAbility->ProductClass.Get()))
					->AddToken(FTextToken::Create(INVTEXT("has no AbilityName set (still NO_NAME_ABILITY) - every ability must be given a unique name. Referenced by")))
					->AddToken(FUObjectToken::Create(AbilityData));
				bFoundNamelessAbility = true;
			}
#endif
			NewAbility->AbilityTag = AAbility::ComposeAbilityTag(AbilityName, /*bErrorIfNotFound=*/false);
			if (!NewAbility->AbilityTag.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("UAbilitySystem::InstantiateAbilities - %s has no registered identity tag for AbilityName '%s' (run Abilities.SyncTags)."),
					*NewAbility->ProductClass->GetName(), *AbilityName.ToString());
			}
		}
		NewAbility->SetupAbility(MyOwner);
		GrantedAbilities.Add(NewAbility);
	}

#if WITH_EDITOR
	if (bFoundNamelessAbility)
	{
		FMessageLog("PIE").Open(EMessageSeverity::Error, /*bForce=*/true);
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	}
#endif
}

UAbilitySlot* UAbilitySystem::InitiateAbility(int32 Slot)
{
	if (IsOwnerDead())
	{
		return nullptr;
	}

	if (GrantedAbilities.IsValidIndex(Slot) &&
		GrantedAbilities[Slot] &&
		MyOwner)
	{
		GrantedAbilities[Slot]->ActivateAbility();
		return GrantedAbilities[Slot];
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		1,
		FColor::Red,
		TEXT("Activate ability failed."));

	return nullptr;
}

void UAbilitySystem::SetActiveAbility(UAbilitySlot* NewActiveAbility)
{
	if (IsOwnerDead())
	{
		return;
	}

	if (NewActiveAbility)
	{
		ActiveAbility = NewActiveAbility;
		ActiveAbility->StartCharging();
	}
	else
		UE_LOG(LogTemp, Error, TEXT(
			"AbilitySystem::ActivateAbility doesn't have valid *NewActiveAbility."));
}

void UAbilitySystem::EndActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
	}
}

void UAbilitySystem::ReleaseActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->ReleaseAbility();
		ActiveAbility = nullptr;
	}
}

bool UAbilitySystem::IsOwnerDead() const
{
	return MyOwner && MyOwner->AttributeComponent && MyOwner->AttributeComponent->IsDead();
}

UAbilitySlot* UAbilitySystem::FindSlot(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilitySystem::FindSlot called with an empty tag."));
		return nullptr;
	}

	for (UAbilitySlot* Slot : GrantedAbilities)
	{
		if (Slot && Slot->AbilityTag == AbilityTag)
		{
			return Slot;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UAbilitySystem::FindSlot - %s has no ability tagged %s."),
		*GetNameSafe(GetOwner()), *AbilityTag.ToString());
	return nullptr;
}

void UAbilitySystem::InstantiateOverTimeEffects(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (const UOverTimeEffectDataAsset* EffectData : UnitData->DefaultOverTimeEffects)
	{
		if (!EffectData)
		{
			continue;
		}

		UOverTimeEffectSlot* NewSlot = NewObject<UOverTimeEffectSlot>(this);
		NewSlot->SetupSlot(EffectData, MyOwner);
		GrantedOverTimeEffects.Add(NewSlot);
	}
}

UOverTimeEffectSlot* UAbilitySystem::FindEffectSlot(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilitySystem::FindEffectSlot called with an empty tag."));
		return nullptr;
	}

	for (UOverTimeEffectSlot* Slot : GrantedOverTimeEffects)
	{
		if (Slot && Slot->EffectTag == EffectTag)
		{
			return Slot;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UAbilitySystem::FindEffectSlot - %s has no over-time effect tagged %s."),
		*GetNameSafe(GetOwner()), *EffectTag.ToString());
	return nullptr;
}
