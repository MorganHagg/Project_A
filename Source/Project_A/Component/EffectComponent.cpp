#include "EffectComponent.h"
#include "../Unit/UnitBase.h"
#include "../Component/AttributeComponent.h"
#include "../Ability/GrantedAbility.h"

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
	if (Effect.IsTimed())
	{
		return AddOverTimeEffect(Effect);
	}

	FAttributeEffect Scaled = Effect;
	if (const UGrantedAbility* Source = Effect.Source.Get())
	{
		Scaled.Magnitude *= Source->GetMagnitudeMultiplier();
	}
	return ApplyInstantEffect(Scaled);
}

FAttributeEffect UEffectComponent::ApplyInstantEffect(const FAttributeEffect& Effect)
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

FAttributeEffect UEffectComponent::ApplyStatModifier(const UObject* Source, const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;

	if (!MyTarget || !MyTarget->AttributeComponent || IsTargetDead())
	{
		Applied.Magnitude = 0.f;
		return Applied;
	}

	MyTarget->AttributeComponent->AddModifier(Source, Effect);
	return Applied;
}

void UEffectComponent::RemoveStatModifiers(const UObject* Source)
{
	if (MyTarget && MyTarget->AttributeComponent)
	{
		MyTarget->AttributeComponent->RemoveModifiers(Source);
	}
}

FAttributeEffect UEffectComponent::AddOverTimeEffect(const FAttributeEffect& Effect)
{
	FAttributeEffect NothingApplied = Effect;
	NothingApplied.Magnitude = 0.f;

	if (!MyTarget || IsTargetDead())
	{
		return NothingApplied;
	}

	// An effect made inline has no granted ability behind it: no identity to stack by, so every
	// application is its own instance of the generic class.
	const UGrantedAbility* Source = Effect.Source.Get();
	const int32 StackLimit = Source ? Source->StackLimit : -1;
	TSubclassOf<UOverTimeEffect> EffectClass = Source && Source->EffectClass ? Source->EffectClass : TSubclassOf<UOverTimeEffect>(UOverTimeEffect::StaticClass());

	if (Source)
	{
		// Stacks of the same effect - per caster unless the granted ability shares them across casters.
		TArray<UOverTimeEffect*> Matches;
		for (UOverTimeEffect* Existing : OverTimeEffects)
		{
			if (Existing && !Existing->IsResolved() && Existing->GetEffectTag() == Source->AbilityTag &&
				(!Source->bMultipleCaster || Existing->GetCaster() == Source->MyCaster))
			{
				Matches.Add(Existing);
			}
		}

		if (StackLimit == 0 && Matches.Num() > 0)
		{
			return NothingApplied;
		}

		// At the limit, the oldest stacks fully end (hook, stat revert, removal) before the new one
		// applies, so their stat changes are gone before the new stack makes its own.
		if (StackLimit > 0)
		{
			for (int32 Index = 0; Index <= Matches.Num() - StackLimit; ++Index)
			{
				ResolveAndRemove(Matches[Index], EOverTimeEffectEnd::Ended);
			}

			// An ending hook can kill the target.
			if (IsTargetDead())
			{
				return NothingApplied;
			}
		}
	}

	UOverTimeEffect* NewEffect = NewObject<UOverTimeEffect>(this, EffectClass);
	OverTimeEffects.Add(NewEffect);
	return NewEffect->Begin(Effect, MyTarget);
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
