#include "AbilityComponent.h"
#include "../Ability/GrantedAbility.h"
#include "../Unit/UnitBase.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/AbilityData.h"
#include "../Effect/OverTimeEffect.h"
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

	bool bFoundNamelessAbility = false;

	PrimaryAbilities.Tap = GrantAbility(UnitData->Primary.Tap, bFoundNamelessAbility);
	PrimaryAbilities.Hold = GrantAbility(UnitData->Primary.Hold, bFoundNamelessAbility);
	PrimaryAbilities.Modify = GrantAbility(UnitData->Primary.Modify, bFoundNamelessAbility);
	SecondaryAbilities.Tap = GrantAbility(UnitData->Secondary.Tap, bFoundNamelessAbility);
	SecondaryAbilities.Hold = GrantAbility(UnitData->Secondary.Hold, bFoundNamelessAbility);
	SecondaryAbilities.Modify = GrantAbility(UnitData->Secondary.Modify, bFoundNamelessAbility);

	for (UAbilityData* AbilityData : UnitData->ExtraAbilities)
	{
		GrantAbility(AbilityData, bFoundNamelessAbility);
	}

#if WITH_EDITOR
	if (bFoundNamelessAbility)
	{
		FMessageLog("PIE").Open(EMessageSeverity::Error, /*bForce=*/true);
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	}
#endif
}

UGrantedAbility* UAbilityComponent::GrantAbility(UAbilityData* AbilityData, bool& bFoundNamelessAbility)
{
	if (!AbilityData)
	{
		return nullptr;
	}

	// One generic class now - identity/behavior come entirely from the copied DataAsset
	// values and AbilityClass, not from a per-ability subclass.
	UGrantedAbility* NewAbility = NewObject<UGrantedAbility>(this);
	NewAbility->CoolDown = AbilityData->CoolDown;
	NewAbility->Cost = AbilityData->Cost;
	NewAbility->bActivateOnRelease = AbilityData->bActivateOnRelease;
	NewAbility->MaxChargeTime = AbilityData->MaxChargeTime;
	NewAbility->AbilityClass = AbilityData->Kind == EAbilityKind::Active ? AbilityData->AbilityClass : nullptr;
	NewAbility->Effect = AbilityData->Effect;
	NewAbility->StackLimit = AbilityData->StackLimit;
	NewAbility->bMultipleCaster = AbilityData->bMultipleCaster;
	NewAbility->EffectClass = AbilityData->EffectClass ? AbilityData->EffectClass : TSubclassOf<UOverTimeEffect>(UOverTimeEffect::StaticClass());

	// Identity: an Active ability's comes from AbilityName on its Ability class's defaults - the
	// same source AAbility::BeginPlay uses, so a granted ability and the Abilities it spawns share
	// one tag. Proc/Passive have no Ability class and are named on the data asset.
	// An Active ability without an Ability class has no identity to give it.
	const bool bIsActiveKind = AbilityData->Kind == EAbilityKind::Active;
	if (!bIsActiveKind || NewAbility->AbilityClass)
	{
		FName AbilityName = AbilityData->AbilityName;
		const UObject* NameOwner = AbilityData;
		if (bIsActiveKind)
		{
			AbilityName = NewAbility->AbilityClass->GetDefaultObject<AAbility>()->AbilityName;
#if WITH_EDITOR
			const UObject* AbilityBlueprint = UBlueprint::GetBlueprintFromClass(NewAbility->AbilityClass);
			NameOwner = AbilityBlueprint ? AbilityBlueprint : NewAbility->AbilityClass.Get();
#endif
		}

#if WITH_EDITOR
		// In the editor a nameless ability ends PIE with a Message Log error instead of reaching
		// AAbility::BeginPlay's checkf on first cast, which would crash the editor and lose
		// unsaved work. Packaged builds still hit the checkf.
		if (GIsEditor && AbilityName == FName("NO_NAME_ABILITY"))
		{
			FMessageLog("PIE").Error()
				->AddToken(FUObjectToken::Create(NameOwner))
				->AddToken(FTextToken::Create(INVTEXT("has no AbilityName set (still NO_NAME_ABILITY) - every ability must be given a unique name. Referenced by")))
				->AddToken(FUObjectToken::Create(AbilityData));
			bFoundNamelessAbility = true;
		}
#endif
		NewAbility->AbilityTag = AAbility::ComposeAbilityTag(AbilityName, /*bErrorIfNotFound=*/false);
		if (!NewAbility->AbilityTag.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("UAbilityComponent::InstantiateAbilities - %s has no registered identity tag for AbilityName '%s' (run Abilities.SyncTags)."),
				*GetNameSafe(NameOwner), *AbilityName.ToString());
		}
	}

	NewAbility->SetupAbility(MyOwner);
	GrantedAbilities.Add(NewAbility);
	return NewAbility;
}

const FGrantedInputAbilities& UAbilityComponent::GetInputAbilities(EAbilityInput Input) const
{
	return Input == EAbilityInput::Primary ? PrimaryAbilities : SecondaryAbilities;
}

UGrantedAbility* UAbilityComponent::InitiateAbility(UGrantedAbility* GrantedAbility)
{
	if (IsOwnerDead())
	{
		return nullptr;
	}

	if (GrantedAbility && MyOwner)
	{
		GrantedAbility->ActivateAbility();
		return GrantedAbility;
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
