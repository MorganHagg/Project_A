#include "AbilityData.h"

#if WITH_EDITOR
void UAbilityData::PostLoad()
{
	Super::PostLoad();
	SyncEffectDisplay();
}

void UAbilityData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SyncEffectDisplay();
}

void UAbilityData::SyncEffectDisplay()
{
	Effect.bHideDuration = Kind == EAbilityKind::Passive;
}
#endif
