#include "TalentComponent.h"

UTalentComponent::UTalentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTalentComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UTalentComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UTalentComponent::bContainAcquiredTalents(UTalentData* Talents)
{
	return AcquiredTalents.Contains(Talents);
}

