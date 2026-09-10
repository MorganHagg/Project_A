#include "UnitSpawner.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/PlayerUnitData.h"
#include "../DataAsset/EnemyUnitData.h"
#include "../Unit/UnitBase.h"
#include "../Unit/EnemyUnit.h"
#include "../Unit/PlayerUnit.h"
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
    NewUnit->SetupUnit(SpawnData);

    return NewUnit;
}

AEnemyUnit* UUnitSpawner::SpawnUnit(UEnemyUnitData* SpawnData, const FTransform& SpawnTransform)
{
    if (!SpawnData) return nullptr;

    UClass* ClassToSpawn = SpawnData->CustomUnitClass ? *SpawnData->CustomUnitClass : AEnemyUnit::StaticClass();
    return Cast<AEnemyUnit>(SpawnUnitInternal(ClassToSpawn, SpawnData, SpawnTransform));
}

APlayerUnit* UUnitSpawner::SpawnPlayerUnit(UPlayerUnitData* SpawnData, const FTransform& SpawnTransform)
{
    if (!SpawnData) return nullptr;

    UClass* ClassToSpawn = SpawnData->CustomUnitClass ? *SpawnData->CustomUnitClass : APlayerUnit::StaticClass();
    return Cast<APlayerUnit>(SpawnUnitInternal(ClassToSpawn, SpawnData, SpawnTransform));
}