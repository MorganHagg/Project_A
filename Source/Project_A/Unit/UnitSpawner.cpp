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
#include "Components/CapsuleComponent.h"

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

    // SpawnTransform's location is where the unit's feet go, but a Character's location is its
    // capsule center - lift by the half-height. Read after SetupUnit, which resizes the capsule.
    const float HalfHeight = NewUnit->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    NewUnit->SetActorLocation(SpawnTransform.GetLocation() + FVector(0.f, 0.f, HalfHeight),
        false, nullptr, ETeleportType::TeleportPhysics);

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