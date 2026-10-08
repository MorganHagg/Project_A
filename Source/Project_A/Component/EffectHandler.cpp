#include "EffectHandler.h"
#include "../Unit/UnitBase.h"
#include "../Component/AttributeComponent.h"
#include "../Effect/OverTimeEffectSlot.h"

UEffectHandler::UEffectHandler()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UEffectHandler::BeginPlay()
{
	Super::BeginPlay();
	MyTarget = CastChecked<AUnitBase>(GetOwner());
}

void UEffectHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

void UEffectHandler::ApplyEffect(const FGameplayEffect& Effect, EAbilityType AbilityType)
{
	// Also catches a unit killed earlier in the same frame.
	if (!MyTarget || !MyTarget->AttributeComponent || IsTargetDead())
	{
		return;
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
				MyTarget->ReceiveDamage(-SignedMagnitude, AbilityType);
			}
			else if (SignedMagnitude > 0.f)
			{
				MyTarget->ReceiveHeal(SignedMagnitude);
			}
			return;
		}

		MyTarget->AttributeComponent->ModifyAttribute(Effect.Attribute, SignedMagnitude);
		return;
	}
	// Not damage - unmitigated, and doesn't fire OnReceiveDamage/OnReceiveHeal.
	case EEffectOperation::AddPercentage:
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, Effect.Magnitude);
		return;
	case EEffectOperation::SubtractPercentage:
		MyTarget->AttributeComponent->ModifyAttributePercent(Effect.Attribute, -Effect.Magnitude);
		return;
	}
}

void UEffectHandler::AddOverTimeEffect(UOverTimeEffectSlot* Slot)
{
	if (!Slot || !Slot->EffectClass || !MyTarget || IsTargetDead())
	{
		return;
	}

	// Stacks of the same effect - per caster unless the slot shares them across casters.
	TArray<UOverTimeEffect*> Matches;
	for (UOverTimeEffect* Effect : OverTimeEffects)
	{
		if (Effect && !Effect->IsResolved() && Effect->GetEffectTag() == Slot->EffectTag &&
			(!Slot->bMultipleCaster || Effect->GetCaster() == Slot->MyCaster))
		{
			Matches.Add(Effect);
		}
	}

	if (Slot->StackLimit == 0 && Matches.Num() > 0)
	{
		return;
	}

	// At the limit, the oldest stacks fully end (hook, stat revert, removal) before the new one
	// applies, so their stat changes are gone before the new stack makes its own.
	if (Slot->StackLimit > 0)
	{
		for (int32 Index = 0; Index <= Matches.Num() - Slot->StackLimit; ++Index)
		{
			ResolveAndRemove(Matches[Index], EOverTimeEffectEnd::Ended);
		}

		// An ending hook can kill the target.
		if (IsTargetDead())
		{
			return;
		}
	}

	UOverTimeEffect* NewEffect = NewObject<UOverTimeEffect>(this, Slot->EffectClass);
	OverTimeEffects.Add(NewEffect);
	NewEffect->Begin(Slot, MyTarget);
}

void UEffectHandler::RemoveOverTimeEffect(UOverTimeEffect* Effect)
{
	if (Effect && OverTimeEffects.Contains(Effect))
	{
		ResolveAndRemove(Effect, EOverTimeEffectEnd::Removed);
	}
}

void UEffectHandler::RemoveOverTimeEffectsByTag(FGameplayTag EffectTag)
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

float UEffectHandler::ModifyIncomingDamage(float Amount, EAbilityType AbilityType)
{
	// Iterates a copy - an effect's override can add or cleanse effects.
	const TArray<UOverTimeEffect*> Effects = OverTimeEffects;
	for (UOverTimeEffect* Effect : Effects)
	{
		if (Effect && !Effect->IsResolved())
		{
			Amount = Effect->ModifyIncomingDamage(Amount, AbilityType);
		}
	}
	return Amount;
}

float UEffectHandler::ModifyIncomingHeal(float Amount)
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

void UEffectHandler::HandleUnitDeath()
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

void UEffectHandler::ResolveAndRemove(UOverTimeEffect* Effect, EOverTimeEffectEnd Reason)
{
	Effect->Resolve(Reason);
	OverTimeEffects.Remove(Effect);
}

bool UEffectHandler::IsTargetDead() const
{
	return MyTarget && MyTarget->AttributeComponent && MyTarget->AttributeComponent->IsDead();
}
