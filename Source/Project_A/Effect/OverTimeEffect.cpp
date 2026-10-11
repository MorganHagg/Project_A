#include "OverTimeEffect.h"
#include "GrantedOverTimeEffect.h"
#include "../Component/EffectComponent.h"
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
	return MyGrantedEffect ? MyGrantedEffect->EffectTag : FGameplayTag();
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
		return Handler->ApplyEffect(Effect);
	}
	return FAttributeEffect();
}

float UOverTimeEffect::ModifyIncomingDamage_Implementation(float Amount, EDamageType DamageType)
{
	return Amount;
}

float UOverTimeEffect::ModifyIncomingHeal_Implementation(float Amount)
{
	return Amount;
}

void UOverTimeEffect::Begin(UGrantedOverTimeEffect* GrantedEffect, AUnitBase* Target)
{
	MyGrantedEffect = GrantedEffect;
	MyTarget = Target;
	MyCaster = GrantedEffect ? GrantedEffect->MyCaster : nullptr;

	Ticker.Interval = GrantedEffect ? GrantedEffect->Interval : 0.f;
	Ticker.Start(GrantedEffect ? GrantedEffect->Duration : 0.f);

	FAttributeEffect AppliedEffect;
	if (Ticker.Interval <= 0.f)
	{
		AppliedEffect = ApplyGrantedEffect();
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
}

bool UOverTimeEffect::Advance(float DeltaTime)
{
	bool bFinished = false;
	const int32 TicksDue = Ticker.Advance(DeltaTime, bFinished);

	// A tick can end this effect (e.g. by killing its target) - stop as soon as it does.
	for (int32 TickIndex = 0; TickIndex < TicksDue && !bResolved; ++TickIndex)
	{
		const FAttributeEffect AppliedEffect = ApplyGrantedEffect();
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
	if (bModifiedStat)
	{
		RevertModifyStat();
	}

	ReportEvent(EventName);
}

FAttributeEffect UOverTimeEffect::ModifyStat(const FAttributeEffect& Effect)
{
	UEffectComponent* Handler = GetHandler();
	if (!Handler)
	{
		return FAttributeEffect();
	}

	const FAttributeEffect Applied = Handler->ApplyEffect(Effect);

	ModifiedStats.Add(Effect);
	bModifiedStat = true;
	return Applied;
}

void UOverTimeEffect::RevertModifyStat()
{
	UEffectComponent* Handler = GetHandler();
	if (Handler)
	{
		for (int32 Index = ModifiedStats.Num() - 1; Index >= 0; --Index)
		{
			FAttributeEffect Inverse = ModifiedStats[Index];
			switch (Inverse.Operation)
			{
			case EEffectOperation::Add:					Inverse.Operation = EEffectOperation::Subtract;				break;
			case EEffectOperation::Subtract:			Inverse.Operation = EEffectOperation::Add;					break;
			case EEffectOperation::AddPercentage:		Inverse.Operation = EEffectOperation::SubtractPercentage;	break;
			case EEffectOperation::SubtractPercentage:	Inverse.Operation = EEffectOperation::AddPercentage;		break;
			}
			Handler->ApplyEffect(Inverse);
		}
	}

	ModifiedStats.Empty();
	bModifiedStat = false;
}

FAttributeEffect UOverTimeEffect::ApplyGrantedEffect()
{
	if (!MyGrantedEffect || MyGrantedEffect->Effect.Magnitude == 0.f)
	{
		return FAttributeEffect();
	}

	// Copied before applying - the granted effect can be changed by a talent reacting to the result.
	const FAttributeEffect Effect = MyGrantedEffect->Effect;
	return ApplyEffect(Effect);
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

	FAbilityEventPayload Payload;
	Payload.Target = MyTarget;
	Payload.OverTimeEffect = this;
	Payload.AppliedEffect = AppliedEffect;

	PlayerUnit->BroadcastAbilityEvent(EventTags, Payload);
}
