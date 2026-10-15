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

	return ApplyInstantEffect(Effect);
}

FAttributeEffect UEffectComponent::ApplyInstantEffect(const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;
	Applied.bCrit = false;

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
			Applied.Magnitude = FMath::Abs(SignedMagnitude);
			if (SignedMagnitude < 0.f)
			{
				return ApplyDamage(Applied);
			}
			if (SignedMagnitude > 0.f)
			{
				return ApplyHeal(Applied);
			}
			return Applied;
		}

		Applied.Magnitude *= GetMagnitudeMultiplier(Effect);
		MyTarget->AttributeComponent->ModifyAttribute(Effect.Attribute, Effect.Operation == EEffectOperation::Subtract ? -Applied.Magnitude : Applied.Magnitude);
		return Applied;
	}
	// Not damage - unmitigated, and doesn't fire OnReceiveDamage/OnReceiveHeal.
	case EEffectOperation::AddPercentage:
		Applied.Magnitude *= GetMagnitudeMultiplier(Effect);
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, Applied.Magnitude);
		return Applied;
	case EEffectOperation::SubtractPercentage:
		Applied.Magnitude *= GetMagnitudeMultiplier(Effect);
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, -Applied.Magnitude);
		return Applied;
	}

	return Applied;
}

FAttributeEffect UEffectComponent::ApplyDamage(const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;
	const float OutgoingDamage = CalculateOutgoingDamage(Effect, Applied.bCrit);
	Applied.Magnitude = MyTarget->ReceiveDamage(MitigateDamage(OutgoingDamage, Effect.DamageType));
	return Applied;
}

FAttributeEffect UEffectComponent::ApplyHeal(const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;
	const float Heal = Effect.Magnitude * GetMagnitudeMultiplier(Effect);
	Applied.Magnitude = MyTarget->ReceiveHeal(ModifyIncomingHeal(Heal));
	return Applied;
}

float UEffectComponent::CalculateOutgoingDamage(const FAttributeEffect& Effect, bool& bOutCrit) const
{
	bOutCrit = false;

	const UGrantedAbility* Source = Effect.Source.Get();
	if (!Source)
	{
		return Effect.Magnitude;
	}

	const AUnitBase* Caster = Source->MyCaster;
	const UAttributeComponent* CasterAttributes = IsValid(Caster) ? Caster->AttributeComponent : nullptr;

	float Damage = Effect.Magnitude;
	if (CasterAttributes)
	{
		switch (Effect.DamageType)
		{
		case EDamageType::Magic:
			Damage += Source->Coefficient * CasterAttributes->GetAttribute(EAttributeType::Intellect);
			break;
		case EDamageType::Physical:
			Damage += Source->Coefficient * CasterAttributes->GetAttribute(EAttributeType::Strength);
			break;
		case EDamageType::TrueDamage:
			break;
		}
	}

	Damage *= Source->GetMagnitudeMultiplier();

	if (CasterAttributes && FMath::FRandRange(0.f, 100.f) < CasterAttributes->GetAttribute(EAttributeType::Crit))
	{
		bOutCrit = true;
		Damage *= CritMultiplier;
	}

	return Damage;
}

float UEffectComponent::MitigateDamage(float Damage, EDamageType DamageType)
{
	float Mitigation = 0.f;
	switch (DamageType)
	{
	case EDamageType::Magic:
		Mitigation = MyTarget->AttributeComponent->GetAttribute(EAttributeType::MagicResist);
		break;
	case EDamageType::Physical:
		Mitigation = MyTarget->AttributeComponent->GetAttribute(EAttributeType::Armour);
		break;
	case EDamageType::TrueDamage:
		break;
	}

	float FinalDamage = Damage * (1.f - Mitigation / (Mitigation + MitigationConstant));
	FinalDamage = ModifyIncomingDamage(FinalDamage, DamageType);

	// Rounded to whole damage so Health stays whole - otherwise mitigation leaves fractional
	// remainders (e.g. 0.4 Health) that look like 0 on the health bar but aren't dead.
	return FMath::RoundToFloat(FMath::Max(FinalDamage, 0.f));
}

float UEffectComponent::GetMagnitudeMultiplier(const FAttributeEffect& Effect)
{
	const UGrantedAbility* Source = Effect.Source.Get();
	return Source ? Source->GetMagnitudeMultiplier() : 1.f;
}

FAttributeEffect UEffectComponent::ApplyStatModifier(const UObject* Source, const FAttributeEffect& Effect)
{
	FAttributeEffect Applied = Effect;
	Applied.bCrit = false;

	if (!MyTarget || !MyTarget->AttributeComponent || IsTargetDead())
	{
		Applied.Magnitude = 0.f;
		return Applied;
	}

	Applied.Magnitude *= GetMagnitudeMultiplier(Effect);
	MyTarget->AttributeComponent->AddModifier(Source, Applied);
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
