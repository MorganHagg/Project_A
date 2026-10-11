#include "GrantedOverTimeEffect.h"
#include "OverTimeEffect.h"
#include "../DataAsset/OverTimeEffectData.h"
#include "../Unit/UnitBase.h"
#include "../Component/EffectComponent.h"

void UGrantedOverTimeEffect::SetupEffect(const UOverTimeEffectData* Data, AUnitBase* Caster)
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
		UE_LOG(LogTemp, Error, TEXT("UGrantedOverTimeEffect::SetupEffect - %s has no registered identity tag for EffectName '%s' (run Abilities.SyncTags)."),
			*Data->GetName(), *Data->EffectName.ToString());
	}
}

FGameplayTag UGrantedOverTimeEffect::ComposeEffectTag(FName InEffectName, bool bErrorIfNotFound)
{
	return FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Effect.")) + InEffectName.ToString())), bErrorIfNotFound);
}

void UGrantedOverTimeEffect::ApplyOverTimeEffect(AUnitBase* Target)
{
	if (Target && Target->EffectComponent)
	{
		Target->EffectComponent->AddOverTimeEffect(this);
	}
}
