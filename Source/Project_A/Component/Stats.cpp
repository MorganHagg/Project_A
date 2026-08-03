#include "Stats.h"


UStats::UStats()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UStats::BeginPlay()
{
	Super::BeginPlay();
}

void UStats::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

