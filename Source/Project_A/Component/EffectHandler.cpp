#include "EffectHandler.h"
#include "../Unit/UnitBase.h"
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
	UpdateEffect();
}

void UEffectHandler::UpdateEffect()
{
	// TODO: FGameplayEffect is currently blank; nothing to tick yet.
}

void UEffectHandler::AddEffect(const FGameplayEffect& Effect)
{
	GameplayEffects.Add(Effect);
}

void UEffectHandler::RemoveEffect(const FGameplayEffect& Effect)
{
	// TODO: FGameplayEffect has no identifying data yet, so instances can't be matched for removal.
}