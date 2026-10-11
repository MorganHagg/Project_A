#include "AbilityComponent.h"
#include "../Ability/GrantedAbility.h"
#include "../Unit/UnitBase.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/AbilityData.h"
#include "../DataAsset/OverTimeEffectData.h"
#include "../Effect/GrantedOverTimeEffect.h"
#include "AttributeComponent.h"
#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Blueprint.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

UAbilityComponent::UAbilityComponent()
{
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	MyOwner = CastChecked<AUnitBase>(GetOwner());
}

void UAbilityComponent::InstantiateAbilities(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

#if WITH_EDITOR
	bool bFoundNamelessAbility = false;
#endif

	for (UAbilityData* AbilityData : UnitData->DefaultAbilities)
	{
		if (!AbilityData)
		{
			continue;
		}

		// One generic class now - identity/behavior come entirely from the copied DataAsset
		// values and AbilityClass, not from a per-ability subclass.
		UGrantedAbility* NewAbility = NewObject<UGrantedAbility>(this);
		NewAbility->CoolDown = AbilityData->CoolDown;
		NewAbility->Cost = AbilityData->Cost;
		NewAbility->bActivateOnRelease = AbilityData->bActivateOnRelease;
		NewAbility->MaxChargeTime = AbilityData->MaxChargeTime;
		NewAbility->AbilityClass = AbilityData->AbilityClass;
		if (NewAbility->AbilityClass)
		{
			// Identity comes from AbilityName on the Ability class's defaults - the same source
			// AAbility::BeginPlay uses, so a granted ability and the Abilities it spawns share one tag.
			const FName AbilityName = NewAbility->AbilityClass->GetDefaultObject<AAbility>()->AbilityName;
#if WITH_EDITOR
			// In the editor a nameless ability ends PIE with a Message Log error instead of reaching
			// AAbility::BeginPlay's checkf on first cast, which would crash the editor and lose
			// unsaved work. Packaged builds still hit the checkf.
			if (GIsEditor && AbilityName == FName("NO_NAME_ABILITY"))
			{
				const UObject* NamelessSource = UBlueprint::GetBlueprintFromClass(NewAbility->AbilityClass);
				FMessageLog("PIE").Error()
					->AddToken(FUObjectToken::Create(NamelessSource ? NamelessSource : NewAbility->AbilityClass.Get()))
					->AddToken(FTextToken::Create(INVTEXT("has no AbilityName set (still NO_NAME_ABILITY) - every ability must be given a unique name. Referenced by")))
					->AddToken(FUObjectToken::Create(AbilityData));
				bFoundNamelessAbility = true;
			}
#endif
			NewAbility->AbilityTag = AAbility::ComposeAbilityTag(AbilityName, /*bErrorIfNotFound=*/false);
			if (!NewAbility->AbilityTag.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::InstantiateAbilities - %s has no registered identity tag for AbilityName '%s' (run Abilities.SyncTags)."),
					*NewAbility->AbilityClass->GetName(), *AbilityName.ToString());
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

UGrantedAbility* UAbilityComponent::InitiateAbility(int32 Slot)
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

void UAbilityComponent::SetActiveAbility(UGrantedAbility* NewActiveAbility)
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
			"UAbilityComponent::ActivateAbility doesn't have valid *NewActiveAbility."));
}

void UAbilityComponent::EndActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->EndAbility();
		ActiveAbility = nullptr;
	}
}

void UAbilityComponent::ReleaseActiveAbility()
{
	if (ActiveAbility)
	{
		ActiveAbility->ReleaseAbility();
		ActiveAbility = nullptr;
	}
}

bool UAbilityComponent::IsOwnerDead() const
{
	return MyOwner && MyOwner->AttributeComponent && MyOwner->AttributeComponent->IsDead();
}

UGrantedAbility* UAbilityComponent::FindGrantedAbility(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::FindGrantedAbility called with an empty tag."));
		return nullptr;
	}

	for (UGrantedAbility* GrantedAbility : GrantedAbilities)
	{
		if (GrantedAbility && GrantedAbility->AbilityTag == AbilityTag)
		{
			return GrantedAbility;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::FindGrantedAbility - %s has no ability tagged %s."),
		*GetNameSafe(GetOwner()), *AbilityTag.ToString());
	return nullptr;
}

void UAbilityComponent::InstantiateOverTimeEffects(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (const UOverTimeEffectData* EffectData : UnitData->DefaultOverTimeEffects)
	{
		if (!EffectData)
		{
			continue;
		}

		UGrantedOverTimeEffect* NewGrantedEffect = NewObject<UGrantedOverTimeEffect>(this);
		NewGrantedEffect->SetupEffect(EffectData, MyOwner);
		GrantedOverTimeEffects.Add(NewGrantedEffect);
	}
}

UGrantedOverTimeEffect* UAbilityComponent::FindGrantedOverTimeEffect(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::FindGrantedOverTimeEffect called with an empty tag."));
		return nullptr;
	}

	for (UGrantedOverTimeEffect* GrantedEffect : GrantedOverTimeEffects)
	{
		if (GrantedEffect && GrantedEffect->EffectTag == EffectTag)
		{
			return GrantedEffect;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::FindGrantedOverTimeEffect - %s has no over-time effect tagged %s."),
		*GetNameSafe(GetOwner()), *EffectTag.ToString());
	return nullptr;
}
