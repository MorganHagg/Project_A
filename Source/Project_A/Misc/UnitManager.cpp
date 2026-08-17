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


void UUnitManager::FixLocAndRot(AUnitBase* NewUnit)
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
    return SpawnUnitInternal<AEnemyUnit, FUnitSpawnDataRow>(
        UnitDataTable, RowName, SpawnTransform,
        [](AEnemyUnit* Unit, FUnitSpawnDataRow* Row)
        {   
            if (Row->BehaviorTree)
                Unit->SetBehaviorTree(Row->BehaviorTree);
        });
}

APlayerUnit* UUnitManager::SpawnPlayerUnit(FName RowName, FVector Location)
{
    FTransform SpawnTransform(FRotator::ZeroRotator, Location);
    return SpawnUnitInternal<APlayerUnit, FPlayerSpawnDataRow>(
        PlayerDataTable, RowName, SpawnTransform,
        [](APlayerUnit* Unit, FPlayerSpawnDataRow* Row)
        {
            Unit->GetCapsuleComponent()->SetCapsuleHalfHeight(10.f, true);

            if (Row->AnimationBlueprint)
            {
                Unit->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
                Unit->GetMesh()->SetAnimInstanceClass(Row->AnimationBlueprint);
            }
        });
}

template<typename UnitType, typename RowType>
UnitType* UUnitManager::SpawnUnitInternal(
    UDataTable* DataTable,
    FName RowName,
    const FTransform& SpawnTransform,
    TFunctionRef<void(UnitType*, RowType*)> ExtraSetup)
{
    if (!DataTable) return nullptr;
    
        RowType* Row = DataTable->FindRow<RowType>(RowName, TEXT("SpawnUnit"));
        if (!Row) return nullptr;
    
        UnitType* NewUnit = Cast<UnitType>(
            GetWorld()->SpawnActorDeferred<UnitType>(
                UnitType::StaticClass(), SpawnTransform, nullptr, nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
        if (!NewUnit) return nullptr;
    
        ExtraSetup(NewUnit, Row);
    
        NewUnit->FinishSpawning(SpawnTransform);
        ApplyCommonSpawnData(NewUnit, Row->Mesh, Row->DefaultAbilities);
    
        return NewUnit;
}