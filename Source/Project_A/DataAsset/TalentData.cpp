#include "TalentData.h"
#include "../Component/TalentComponent.h"


bool UTalentData::bPrerequisitesMet(UTalentComponent* TalentComponent)
{
	if (Prerequisites.IsEmpty())
		return true;
	
	for (int i = 0; i < Prerequisites.Num(); i++)
	{
		if (!TalentComponent->bContainAcquiredTalents(Prerequisites[i]))
			return false;
	}
	
	return true;	
}
