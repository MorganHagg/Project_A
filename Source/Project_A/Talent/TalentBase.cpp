#include "TalentBase.h"

#include "Project_A/Ability/Ability.h"

void UTalentBase::BindToAbility(UAbility* Ability)
{
	MyAbility = Ability;
	if (!Ability)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null Ability"));
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("I am bound to %s"),
		*Ability->GetAbilityName().ToString()
	);

	Ability->OnHit.AddDynamic(this, &UTalentBase::HandleAbilityHit);
	
	OnSetup();
}