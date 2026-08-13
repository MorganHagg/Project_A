#include "UnitSpawner.h"
#include "../Misc/UnitManager.h"
#include "Kismet/GameplayStatics.h"

AUnitSpawner::AUnitSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUnitSpawner::BeginPlay()
{
	UnitManager = GetGameInstance()->GetSubsystem<UUnitManager>();
	Super::BeginPlay();
	SpawnUnit();
}

void AUnitSpawner::SpawnUnit()
{
	if (UnitToSpawn.RowName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitSpawner: no row selected"));
		return;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance) return;

	UnitManager->SpawnUnit(UnitToSpawn.RowName, GetActorTransform());
}