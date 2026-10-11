#include "EffectComponent.h"
#include "../Unit/UnitBase.h"
#include "../Component/AttributeComponent.h"
#include "../Effect/GrantedOverTimeEffect.h"

UEffectComponent::UEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	MyTarget = CastChecked<AUnitBase>(GetOwner());
}

void UEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Iterates a copy - hooks can add, cleanse, or (by killing the target) end effects mid-loop.
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect && !Effect->IsResolved() && Effect->Advance(DeltaTime))
		{
			ResolveAndRemove(Effect, EOverTimeEffectEnd::Ended);
		}
	}
}

FAttributeEffect UEffectComponent::ApplyEffect(const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;

	// Also catches a unit killed earlier in the same frame.
	if (!MyTarget || !MyTarget->AttributeComponent || IsTargetDead())
	{
		Applied.Magnitude = 0.f;
		return Applied;
	}

	switch (Effect.Operation)
	{
	case EEffectOperation::Add:
	case EEffectOperation::Subtract:
	{
		const float SignedMagnitude = Effect.Operation == EEffectOperation::Subtract ? -Effect.Magnitude : Effect.Magnitude;

		if (Effect.Attribute == EAttributeType::Health)
		{
			if (SignedMagnitude < 0.f)
			{
				Applied.Magnitude = MyTarget->ReceiveDamage(-SignedMagnitude, Effect.DamageType);
			}
			else if (SignedMagnitude > 0.f)
			{
				Applied.Magnitude = MyTarget->ReceiveHeal(SignedMagnitude);
			}
			return Applied;
		}

		MyTarget->AttributeComponent->ModifyAttribute(Effect.Attribute, SignedMagnitude);
		return Applied;
	}
	// Not damage - unmitigated, and doesn't fire OnReceiveDamage/OnReceiveHeal.
	case EEffectOperation::AddPercentage:
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, Effect.Magnitude);
		return Applied;
	case EEffectOperation::SubtractPercentage:
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, -Effect.Magnitude);
		return Applied;
	}

	return Applied;
}

void UEffectComponent::AddOverTimeEffect(UGrantedOverTimeEffect* GrantedEffect)
{
	if (!GrantedEffect || !GrantedEffect->EffectClass || !MyTarget || IsTargetDead())
	{
		return;
	}

	// Stacks of the same effect - per caster unless the granted effect shares them across casters.
	TArray<UOverTimeEffect*> Matches;
	for (UOverTimeEffect* Effect : OverTimeEffects)
	{
		if (Effect && !Effect->IsResolved() && Effect->GetEffectTag() == GrantedEffect->EffectTag &&
			(!GrantedEffect->bMultipleCaster || Effect->GetCaster() == GrantedEffect->MyCaster))
		{
			Matches.Add(Effect);
		}
	}

	if (GrantedEffect->StackLimit == 0 && Matches.Num() > 0)
	{
		return;
	}

	// At the limit, the oldest stacks fully end (hook, stat revert, removal) before the new one
	// applies, so their stat changes are gone before the new stack makes its own.
	if (GrantedEffect->StackLimit > 0)
	{
		for (int32 Index = 0; Index <= Matches.Num() - GrantedEffect->StackLimit; ++Index)
		{
			ResolveAndRemove(Matches[Index], EOverTimeEffectEnd::Ended);
		}

		// An ending hook can kill the target.
		if (IsTargetDead())
		{
			return;
		}
	}

	UOverTimeEffect* NewEffect = NewObject<UOverTimeEffect>(this, GrantedEffect->EffectClass);
	OverTimeEffects.Add(NewEffect);
	NewEffect->Begin(GrantedEffect, MyTarget);
}

void UEffectComponent::RemoveOverTimeEffect(UOverTimeEffect* Effect)
{
	if (Effect && OverTimeEffects.Contains(Effect))
	{
		ResolveAndRemove(Effect, EOverTimeEffectEnd::Removed);
	}
}

void UEffectComponent::RemoveOverTimeEffectsByTag(FGameplayTag EffectTag)
{
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect && Effect->GetEffectTag() == EffectTag)
		{
			ResolveAndRemove(Effect, EOverTimeEffectEnd::Removed);
		}
	}
}

float UEffectComponent::ModifyIncomingDamage(float Amount, EDamageType DamageType)
{
	// Iterates a copy - an effect's override can add or cleanse effects.
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect && !Effect->IsResolved())
		{
			Amount = Effect->ModifyIncomingDamage(Amount, DamageType);
		}
	}
	return Amount;
}

float UEffectComponent::ModifyIncomingHeal(float Amount)
{
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect && !Effect->IsResolved())
		{
			Amount = Effect->ModifyIncomingHeal(Amount);
		}
	}
	return Amount;
}

void UEffectComponent::HandleUnitDeath()
{
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect)
		{
			Effect->Resolve(EOverTimeEffectEnd::UnitDeath);
		}
	}

	OverTimeEffects.Empty();
	SetComponentTickEnabled(false);
}

void UEffectComponent::ResolveAndRemove(UOverTimeEffect* Effect, EOverTimeEffectEnd Reason)
{
	Effect->Resolve(Reason);
	OverTimeEffects.Remove(Effect);
}

bool UEffectComponent::IsTargetDead() const
{
	return MyTarget && MyTarget->AttributeComponent && MyTarget->AttributeComponent->IsDead();
}
