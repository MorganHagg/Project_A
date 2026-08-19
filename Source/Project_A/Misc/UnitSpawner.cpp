#include "UnitSpawner.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/PlayerUnitData.h"
#include "../DataAsset/EnemyUnitData.h"
#include "../Actor/UnitBase.h"
#include "../Actor/EnemyUnit.h"
#include "../Actor/PlayerUnit.h"
#include "GameFramework/CharacterMovementComponent.h"


AUnitBase* UUnitSpawner::SpawnUnitInternal(
    UClass* UnitClass,
    UUnitDataBase* SpawnData,
    const FTransform& SpawnTransform)
{
    if (!SpawnData) return nullptr;

    AUnitBase* NewUnit = Cast<AUnitBase>(
        GetWorld()->SpawnActorDeferred<AActor>(
            UnitClass, SpawnTransform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
    if (!NewUnit) return nullptr;

    NewUnit->FinishSpawning(SpawnTransform);
    NewUnit->Initiate(SpawnData);

    return NewUnit;
}

AEnemyUnit* UUnitSpawner::SpawnUnit(UEnemyUnitData* SpawnData, const FTransform& SpawnTransform)
{
    return Cast<AEnemyUnit>(SpawnUnitInternal(AEnemyUnit::StaticClass(), SpawnData, SpawnTransform));
}

APlayerUnit* UUnitSpawner::SpawnPlayerUnit(UPlayerUnitData* SpawnData, FVector Location)
{
    FTransform SpawnTransform(FRotator::ZeroRotator, Location);
    return Cast<APlayerUnit>(SpawnUnitInternal(APlayerUnit::StaticClass(), SpawnData, SpawnTransform));
}