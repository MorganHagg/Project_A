#include "EffectHandler.h"
#include "Gameframework/Character.h"
#include "../GameplayEffect/GameplayEffect.h"
#include "Stats.h"

UEffectHandler::UEffectHandler()
{
	PrimaryComponentTick.bCanEverTick = true;
	MyTarget = CastChecked<ACharacter>(GetOwner());
}


void UEffectHandler::BeginPlay()
{
	Super::BeginPlay();

}

void UEffectHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateEffect();
}

void UEffectHandler::UpdateEffect()
{
	for (const TObjectPtr<UGameplayEffect>& Effect : GameplayEffects)
	{
		if (Effect)
		{
			// Run through array of effects, and execute them if they're set to execute. i.e, interval timer is 0
			// Remove effect if duration <= 0
		}
	}
}

void UEffectHandler::AddEffect(UGameplayEffect* Effect)
{
	GameplayEffects.Add(Effect);
}

void UEffectHandler::RemoveEffect(UGameplayEffect* Effect)
{
	GameplayEffects.Remove(Effect);
}

