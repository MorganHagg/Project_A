#include "OverTimeEffect.h"
#include "../Ability/GrantedAbility.h"
#include "../Component/EffectComponent.h"
#include "../Component/AbilityComponent.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"

UWorld* UOverTimeEffect::GetWorld() const
{
	// The CDO has no outer world. Not calling Super is what marks GetWorld as implemented
	// (UObject::ImplementsGetWorld), which enables world-context nodes in Blueprint subclasses.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	const UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

AUnitBase* UOverTimeEffect::GetCaster() const
{
	return MyCaster.Get();
}

FGameplayTag UOverTimeEffect::GetEffectTag() const
{
	return MyGrantedAbility ? MyGrantedAbility->AbilityTag : FGameplayTag();
}

FAttributeEffect UOverTimeEffect::ApplyEffect(const FAttributeEffect& Effect)
{
	const bool bIsHealthChange = Effect.Attribute == EAttributeType::Health &&
		(Effect.Operation == EEffectOperation::Add || Effect.Operation == EEffectOperation::Subtract);

	if (!bIsHealthChange)
	{
		return ModifyStat(Effect);
	}

	if (UEffectComponent* Handler = GetHandler())
	{
		return Handler->ApplyInstantEffect(Effect);
	}
	return FAttributeEffect();
}

void UOverTimeEffect::ModifyAbilityMagnitude(FGameplayTag AbilityTag, float Percent)
{
	const AUnitBase* Caster = GetCaster();
	if (!Caster || !Caster->AbilityComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UOverTimeEffect::ModifyAbilityMagnitude - %s has no caster to find %s on."),
			*GetName(), *AbilityTag.ToString());
		return;
	}

	if (UGrantedAbility* GrantedAbility = Caster->AbilityComponent->FindGrantedAbility(AbilityTag))
	{
		GrantedAbility->AddMagnitudeModifier(this, Percent);
		ModifiedAbilities.AddUnique(GrantedAbility);
	}
}

float UOverTimeEffect::ModifyIncomingDamage_Implementation(float Amount, EDamageType DamageType)
{
	return Amount;
}

float UOverTimeEffect::ModifyIncomingHeal_Implementation(float Amount)
{
	return Amount;
}

FAttributeEffect UOverTimeEffect::Begin(const FAttributeEffect& Effect, AUnitBase* Target)
{
	SnapshotEffect = Effect;
	MyGrantedAbility = Effect.Source.Get();
	MyTarget = Target;
	MyCaster = MyGrantedAbility ? MyGrantedAbility->MyCaster : nullptr;

	Ticker.Interval = Effect.Interval;
	Ticker.Start(Effect.Duration);

	FAttributeEffect AppliedEffect;
	if (Ticker.Interval <= 0.f)
	{
		AppliedEffect = ApplySnapshotEffect();
	}

	// The granted effect's Effect may already have killed the target, which resolves this effect.
	if (!bResolved)
	{
		OnApplied();
	}

	// OnApplied may already have killed the target, which resolves this effect.
	if (!bResolved)
	{
		ReportEvent(TEXT("Applied"), AppliedEffect);
	}

	return AppliedEffect;
}

bool UOverTimeEffect::Advance(float DeltaTime)
{
	bool bFinished = false;
	const int32 TicksDue = Ticker.Advance(DeltaTime, bFinished);

	// A tick can end this effect (e.g. by killing its target) - stop as soon as it does.
	for (int32 TickIndex = 0; TickIndex < TicksDue && !bResolved; ++TickIndex)
	{
		const FAttributeEffect AppliedEffect = ApplySnapshotEffect();
		if (!bResolved)
		{
			OnTick();
		}
		if (!bResolved)
		{
			ReportEvent(TEXT("Tick"), AppliedEffect);
		}
	}

	return bFinished && !bResolved;
}

void UOverTimeEffect::Resolve(EOverTimeEffectEnd Reason)
{
	if (bResolved)
	{
		return;
	}
	bResolved = true;

	const TCHAR* EventName = TEXT("Ended");
	switch (Reason)
	{
	case EOverTimeEffectEnd::Ended:
		OnEnded();
		break;
	case EOverTimeEffectEnd::Removed:
		OnRemoved();
		EventName = TEXT("Removed");
		break;
	case EOverTimeEffectEnd::UnitDeath:
		OnUnitDeath();
		EventName = TEXT("UnitDeath");
		break;
	}

	// After the hook, so it can't be skipped - and so the hook still sees its own stat changes.
	if (UEffectComponent* Handler = GetHandler())
	{
		Handler->RemoveStatModifiers(this);
	}
	for (const TWeakObjectPtr<UGrantedAbility>& ModifiedAbility : ModifiedAbilities)
	{
		if (UGrantedAbility* GrantedAbility = ModifiedAbility.Get())
		{
			GrantedAbility->RemoveMagnitudeModifiers(this);
		}
	}
	ModifiedAbilities.Empty();

	ReportEvent(EventName);
}

FAttributeEffect UOverTimeEffect::ModifyStat(const FAttributeEffect& Effect)
{
	UEffectComponent* Handler = GetHandler();
	if (!Handler)
	{
		return FAttributeEffect();
	}

	return Handler->ApplyStatModifier(this, Effect);
}

FAttributeEffect UOverTimeEffect::ApplySnapshotEffect()
{
	if (SnapshotEffect.Magnitude == 0.f)
	{
		return FAttributeEffect();
	}

	return ApplyEffect(SnapshotEffect);
}

UEffectComponent* UOverTimeEffect::GetHandler() const
{
	return Cast<UEffectComponent>(GetOuter());
}

void UOverTimeEffect::ReportEvent(const TCHAR* EventName, const FAttributeEffect& AppliedEffect)
{
	// Talents only listen on the PlayerUnit - effects from any other caster report nothing.
	APlayerUnit* PlayerUnit = Cast<APlayerUnit>(MyCaster.Get());
	if (!PlayerUnit)
	{
		return;
	}

	FGameplayTagContainer EventTags;
	const FGameplayTag EffectTag = GetEffectTag();
	if (EffectTag.IsValid())
	{
		EventTags.AddTag(EffectTag);
	}
	const FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Event.")) + EventName)));
	if (EventTag.IsValid())
	{
		EventTags.AddTag(EventTag);
	}
	if (AppliedEffect.bCrit)
	{
		EventTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Crit")));
	}

	FAbilityEventPayload Payload;
	Payload.Target = MyTarget;
	Payload.OverTimeEffect = this;
	Payload.AppliedEffect = AppliedEffect;

	PlayerUnit->BroadcastAbilityEvent(EventTags, Payload);
}
