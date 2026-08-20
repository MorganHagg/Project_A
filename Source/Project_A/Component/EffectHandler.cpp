#include "EffectHandler.h"
#include "../Unit/UnitBase.h"
#include "../GameplayEffect/GameplayEffect.h"
#include "Stats.h"

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
	UpdateEffect();
}

void UEffectHandler::UpdateEffect()
{
	for (TObjectPtr<UGameplayEffect> Effect : GameplayEffects)
	{
		if (Effect)
		{
			// Effect->...
		}
	}
}

void UEffectHandler::AddEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!EffectClass)
		return;

	UGameplayEffect* Effect = NewObject<UGameplayEffect>(this, EffectClass);
	GameplayEffects.Add(Effect);
}

void UEffectHandler::RemoveEffect(TSubclassOf<UGameplayEffect> Effect)
{
	GameplayEffects.RemoveAll([Effect](const TObjectPtr<UGameplayEffect>& ExistingEffect)
	{
		return ExistingEffect && ExistingEffect->IsA(Effect);
	});
}