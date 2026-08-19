#include "UnitSpawner.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../DataAsset/UnitDataBase.h"
#include "../DataAsset/PlayerUnitData.h"
#include "../DataAsset/EnemyUnitData.h"
#include "../Actor/UnitBase.h"
#include "../Actor/EnemyUnit.h"
#include "../Actor/PlayerUnit.h"
#include "../Component/TalentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Component/AbilitySystem.h"

void UUnitSpawner::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UUnitSpawner::FixLocAndRot(AUnitBase* NewUnit)
{
    NewUnit->GetMesh()->SetRelativeLocationAndRotation(
        FVector(0.f, 0.f, -NewUnit->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
        FRotator(0.f, -90.f, 0.f));
}

void UUnitSpawner::ApplyCommonSpawnData(AUnitBase* NewUnit, UUnitDataBase* SpawnData)
{
    NewUnit->GetCapsuleComponent()->SetCapsuleHalfHeight(10.f, true);
    FixLocAndRot(NewUnit);
    NewUnit->GetMesh()->SetSkeletalMesh(SpawnData->Mesh);
    NewUnit->AbilitySystemComponent->GrantedAbilities = SpawnData->DefaultAbilities;

    if (SpawnData->AnimationBlueprint)
    {
        NewUnit->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        NewUnit->GetMesh()->SetAnimInstanceClass(SpawnData->AnimationBlueprint);
    }
}

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
    ApplyCommonSpawnData(NewUnit, SpawnData);

    return NewUnit;
}

AEnemyUnit* UUnitSpawner::SpawnUnit(UEnemyUnitData* SpawnData, const FTransform& SpawnTransform)
{
    AEnemyUnit* NewUnit = Cast<AEnemyUnit>(SpawnUnitInternal(AEnemyUnit::StaticClass(), SpawnData, SpawnTransform));
    if (!NewUnit) return nullptr;

    if (SpawnData->BehaviorTree)
        NewUnit->SetBehaviorTree(SpawnData->BehaviorTree);

    return NewUnit;
}

APlayerUnit* UUnitSpawner::SpawnPlayerUnit(UPlayerUnitData* SpawnData, FVector Location)
{
    FTransform SpawnTransform(FRotator::ZeroRotator, Location);
    APlayerUnit* NewUnit = Cast<APlayerUnit>(SpawnUnitInternal(APlayerUnit::StaticClass(), SpawnData, SpawnTransform));
    if (!NewUnit) return nullptr;
    
    NewUnit->TalentComponent->AcquiredTalents = SpawnData->Talents;

    return NewUnit;
}