#include "GameplayEffect.h"

void UGameplayEffect::OnStartEffect()
{
}

void UGameplayEffect::OnTickEffect()
{
}

void UGameplayEffect::OnEndEffect()
{
	if (bHasEnded) return;
	bHasEnded = true;
}
