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
	for (int32 Index = GameplayEffects.Num() - 1; Index >= 0; --Index)
	{
		FActiveGameplayEffect& ActiveEffect = GameplayEffects[Index];

		if (ActiveEffect.Ticker.ShouldTick(DeltaTime))
		{
			ApplyEffect(ActiveEffect.Effect);
		}

		ActiveEffect.DurationTimer -= DeltaTime;
		if (ActiveEffect.DurationTimer <= 0.f)
		{
			GameplayEffects.RemoveAt(Index);
		}
	}
}

void UEffectHandler::AddEffect(const FGameplayEffect& Effect)
{
	if (Effect.Duration <= 0.f)
	{
		ApplyEffect(Effect);
		return;
	}

	FActiveGameplayEffect ActiveEffect;
	ActiveEffect.Effect = Effect;
	ActiveEffect.DurationTimer = Effect.Duration;
	ActiveEffect.Ticker.Interval = Effect.Interval;
	GameplayEffects.Add(ActiveEffect);
}

void UEffectHandler::RemoveEffect(const FGameplayEffect& Effect)
{
	// TODO: FGameplayEffect has no identifying data yet, so instances can't be matched for removal.
}

void UEffectHandler::ApplyEffect(const FGameplayEffect& Effect)
{
	if (!MyTarget || !MyTarget->AttributeComponent)
	{
		return;
	}

	const float SignedMagnitude = Effect.Operation == EEffectOperation::Subtract ? -Effect.Magnitude : Effect.Magnitude;

	if (Effect.Operation == EEffectOperation::Modify)
	{
		MyTarget->AttributeComponent->SetAttribute(Effect.Attribute, Effect.Magnitude);
	}
	else
	{
		MyTarget->AttributeComponent->ModifyAttribute(Effect.Attribute, SignedMagnitude);
	}
}