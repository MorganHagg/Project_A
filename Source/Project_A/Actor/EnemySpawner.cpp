#include "EnemySpawner.h"
#include "../Misc/UnitSpawner.h"
#include "Kismet/GameplayStatics.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	UnitSpawner = GetGameInstance()->GetSubsystem<UUnitSpawner>();
	Super::BeginPlay();
	SpawnUnit();
}

void AEnemySpawner::SpawnUnit()
{
	
}