#include "UnitManager.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../Misc/PlayerSpawnDataRow.h"
#include "../Misc/UnitSpawnDataRow.h"
#include "../Actor/UnitBase.h"
#include "../Actor/EnemyUnit.h"
#include "../Actor/PlayerUnit.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Component/AbilitySystem.h"

void UUnitManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UnitDataTable = LoadObject<UDataTable>(nullptr,
       TEXT("/Game/Framework/DataTable/DT_UnitSpawnData.DT_UnitSpawnData"));

    PlayerDataTable = LoadObject<UDataTable>(
    nullptr,
    TEXT("/Game/Framework/DataTable/DT_PlayerSpawnData.DT_PlayerSpawnData"));
}


void UUnitManager::FixLocAndRot(ACharacter* NewUnit)
{
    NewUnit->GetMesh()->SetRelativeLocationAndRotation(
        FVector(0.f, 0.f, -NewUnit->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
        FRotator(0.f, -90.f, 0.f));
}

void UUnitManager::ApplyCommonSpawnData(
    AUnitBase* NewUnit,
    USkeletalMesh* Mesh,
    const TArray<TSubclassOf<UAbility>>& DefaultAbilities)
{
    FixLocAndRot(NewUnit);
    NewUnit->GetMesh()->SetSkeletalMesh(Mesh);
    NewUnit->AbilitySystemComponent->GrantedAbilities = DefaultAbilities;
}

AEnemyUnit* UUnitManager::SpawnUnit(FName RowName, const FTransform& SpawnTransform)
{
    if (!UnitDataTable) return nullptr;

    FUnitSpawnDataRow* Row = UnitDataTable->FindRow<FUnitSpawnDataRow>(RowName, TEXT("SpawnUnit"));
    if (!Row) return nullptr;

    AEnemyUnit* NewUnit = Cast<AEnemyUnit>(
        GetWorld()->SpawnActorDeferred<AEnemyUnit>(
            AEnemyUnit::StaticClass(), SpawnTransform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
    if (!NewUnit) return nullptr;

    NewUnit->FinishSpawning(SpawnTransform);
    ApplyCommonSpawnData(NewUnit, Row->Mesh, Row->DefaultAbilities);

    if (Row->BehaviorTree)
        NewUnit->SetBehaviorTree(Row->BehaviorTree);

    return NewUnit;
}

APlayerUnit* UUnitManager::SpawnPlayerUnit(FName RowName, FVector Location)
{
    if (!PlayerDataTable) return nullptr;

    FPlayerSpawnDataRow* Row = PlayerDataTable->FindRow<FPlayerSpawnDataRow>(RowName, TEXT("SpawnPlayerUnit"));
    if (!Row) return nullptr;

    FTransform SpawnTransform(FRotator::ZeroRotator, Location);
    APlayerUnit* NewUnit = Cast<APlayerUnit>(
        GetWorld()->SpawnActorDeferred<APlayerUnit>(
            APlayerUnit::StaticClass(), SpawnTransform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
    if (!NewUnit) return nullptr;

    NewUnit->GetCapsuleComponent()->SetCapsuleHalfHeight(10.f, true);
    NewUnit->FinishSpawning(SpawnTransform);
    ApplyCommonSpawnData(NewUnit, Row->Mesh, Row->DefaultAbilities);

    if (Row->AnimationBlueprint)
    {
        NewUnit->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        NewUnit->GetMesh()->SetAnimInstanceClass(Row->AnimationBlueprint);
    }

    return NewUnit;
}