#include "OverTimeEffectSlot.h"
#include "OverTimeEffect.h"
#include "../DataAsset/OverTimeEffectDataAsset.h"
#include "../Unit/UnitBase.h"
#include "../Component/EffectHandler.h"

void UOverTimeEffectSlot::SetupSlot(const UOverTimeEffectDataAsset* Data, AUnitBase* Caster)
{
	check(Data);

	MyCaster = Caster;
	EffectClass = Data->EffectClass ? Data->EffectClass : TSubclassOf<UOverTimeEffect>(UOverTimeEffect::StaticClass());
	Duration = Data->Duration;
	Interval = Data->Interval;
	StackLimit = Data->StackLimit;
	bMultipleCaster = Data->bMultipleCaster;
	Effect = Data->Effect;

	EffectTag = ComposeEffectTag(Data->EffectName, /*bErrorIfNotFound=*/false);
	if (!EffectTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UOverTimeEffectSlot::SetupSlot - %s has no registered identity tag for EffectName '%s' (run Abilities.SyncTags)."),
			*Data->GetName(), *Data->EffectName.ToString());
	}
}

FGameplayTag UOverTimeEffectSlot::ComposeEffectTag(FName InEffectName, bool bErrorIfNotFound)
{
	return FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Effect.")) + InEffectName.ToString())), bErrorIfNotFound);
}

void UOverTimeEffectSlot::ApplyOverTimeEffect(AUnitBase* Target)
{
	if (Target && Target->EffectHandlerComponent)
	{
		Target->EffectHandlerComponent->AddOverTimeEffect(this);
	}
}
