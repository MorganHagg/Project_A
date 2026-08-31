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

	if (Effect.Attribute == EEffectAttribute::Health)
	{
		FResource& Health = MyTarget->AttributeComponent->BaseHealth;
		Health.Value = Effect.Operation == EEffectOperation::Modify ? Effect.Magnitude : Health.Value + SignedMagnitude;
		Health.Value = FMath::Clamp(Health.Value, 0.f, Health.Max);
		return;
	}

	// EEffectAttribute mirrors EAttributeType with Health inserted at the front.
	const EAttributeType AttributeType = static_cast<EAttributeType>(static_cast<uint8>(Effect.Attribute) - 1);

	if (Effect.Operation == EEffectOperation::Modify)
	{
		MyTarget->AttributeComponent->SetAttribute(AttributeType, Effect.Magnitude);
	}
	else
	{
		MyTarget->AttributeComponent->ModifyAttribute(AttributeType, SignedMagnitude);
	}
}