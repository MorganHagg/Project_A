#include "EffectHandler.h"
#include "../Unit/UnitBase.h"
#include "../Component/AttributeComponent.h"
#include "../Misc/GameplayEffect.h"
#include "../Misc/AttributeSet.h"

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
	UpdateEffect(DeltaTime);
}

void UEffectHandler::UpdateEffect(float DeltaTime)
{
	// Death cancels every active effect - nothing is applied to a dead unit.
	if (IsTargetDead())
	{
		GameplayEffects.Empty();
		SetComponentTickEnabled(false);
		return;
	}

	for (int32 Index = GameplayEffects.Num() - 1; Index >= 0; --Index)
	{
		FActiveGameplayEffect& ActiveEffect = GameplayEffects[Index];

		if (ActiveEffect.Ticker.ShouldTick(DeltaTime))
		{
			ApplyEffect(ActiveEffect.Effect, ActiveEffect.AbilityType);
		}

		ActiveEffect.DurationTimer -= DeltaTime;
		if (ActiveEffect.DurationTimer <= 0.f)
		{
			GameplayEffects.RemoveAt(Index);
		}
	}
}

void UEffectHandler::AddEffect(const FGameplayEffect& Effect, EAbilityType AbilityType)
{
	if (IsTargetDead())
	{
		return;
	}

	if (Effect.Duration <= 0.f)
	{
		ApplyEffect(Effect, AbilityType);
		return;
	}

	FActiveGameplayEffect ActiveEffect;
	ActiveEffect.Effect = Effect;
	ActiveEffect.DurationTimer = Effect.Duration;
	ActiveEffect.Ticker.Interval = Effect.Interval;
	ActiveEffect.AbilityType = AbilityType;
	GameplayEffects.Add(ActiveEffect);
}

void UEffectHandler::RemoveEffect(const FGameplayEffect& Effect)
{
	// TODO: FGameplayEffect has no identifying data yet, so instances can't be matched for removal.
}

void UEffectHandler::ApplyEffect(const FGameplayEffect& Effect, EAbilityType AbilityType)
{
	// Also catches a unit killed earlier in the same UpdateEffect pass.
	if (!MyTarget || !MyTarget->AttributeComponent || IsTargetDead())
	{
		return;
	}

	const float SignedMagnitude = Effect.Operation == EEffectOperation::Subtract ? -Effect.Magnitude : Effect.Magnitude;

	if (Effect.Operation == EEffectOperation::Modify)
	{
		MyTarget->AttributeComponent->SetAttribute(Effect.Attribute, Effect.Magnitude);
		return;
	}

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
}

bool UEffectHandler::IsTargetDead() const
{
	return MyTarget && MyTarget->AttributeComponent && MyTarget->AttributeComponent->IsDead();
}
