#include "TalentBase.h"

#include "Project_A/Unit/PlayerUnit.h"

void UTalentBase::BindToPlayerUnit(APlayerUnit* PlayerUnit)
{
	MyPlayerUnit = PlayerUnit;
	if (!PlayerUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null PlayerUnit"));
		return;
	}

	PlayerUnit->OnAbilityEvent.AddDynamic(this, &UTalentBase::HandleAbilityEvent);

	OnSetup();
}

void UTalentBase::HandleAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload)
{
	if (EventTag.MatchesTag(ListenTag))
	{
		OnAbilityEvent(Payload);
	}
}