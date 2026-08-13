#include "UnitManager.h"
#include "Engine/World.h" 
#include "BehaviorTree/BehaviorTree.h"
#include "../AI/AIUnit.h"
#include "../Misc/FUnitSpawnDataRow.h"
#include "../Misc/PlayerSpawnDataRow.h"
#include "../Actor/UnitBase.h"
#include "../Actor/PlayerUnit.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Project_A/Component/AbilitySystem.h"

void UUnitManager::FixLocAndRot(ACharacter* NewUnit)
{
	NewUnit->GetMesh()->SetRelativeLocationAndRotation(
	FVector(											// offset down to feet
		0.f,
		0.f,
		-NewUnit->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
	FRotator(0.f, -90.f, 0.f)			// rotate to face forward
	);
}

void UUnitManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UnitDataTable = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/Framework/DataTable/DT_UnitSpawnData.DT_UnitSpawnData"));

	PlayerDataTable = LoadObject<UDataTable>(
	nullptr,
	TEXT("/Game/Framework/DataTable/DT_PlayerSpawnData.DT_PlayerSpawnData"));
}

AUnitBase* UUnitManager::SpawnUnit(FName RowName, const FTransform& SpawnTransform)
{
	if (!UnitDataTable) return nullptr;

	FFUnitSpawnDataRow* Row = UnitDataTable->FindRow<FFUnitSpawnDataRow>(RowName, TEXT("SpawnUnit"));
	if (!Row) return nullptr;

	AUnitBase* NewUnit = Cast<AUnitBase>(
		GetWorld()->SpawnActorDeferred<AUnitBase>(
			AUnitBase::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn));

	if (!NewUnit) return nullptr;
	NewUnit->FinishSpawning(SpawnTransform);
	FixLocAndRot(NewUnit);

	NewUnit->GetMesh()->SetSkeletalMesh(Row->Mesh);
	/*if (Row->AnimationBlueprint)
	{
		NewUnit->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		NewUnit->GetMesh()->SetAnimInstanceClass(Row->AnimationBlueprint);
	}*/
	
	/*NewUnit->BehaviorTree = Row->BehaviorTree;
	if (NewUnit->AIController && NewUnit->BehaviorTree)
		NewUnit->AIController->RunBehaviorTree(NewUnit->BehaviorTree);*/
		
	//NewUnit->GrantedAbilities = Row->DefaultAbilities;
	return NewUnit;
}

APlayerUnit* UUnitManager::SpawnPlayerUnit(FName RowName, FVector Location)
{
	if (!PlayerDataTable)
		return nullptr;

	FPlayerSpawnDataRow* Row = PlayerDataTable->FindRow<FPlayerSpawnDataRow>(
		RowName,
		TEXT("SpawnPlayerUnit"));

	if (!Row)
		return nullptr;

	FTransform SpawnTransform(FRotator::ZeroRotator, Location);

	APlayerUnit* NewUnit = Cast<APlayerUnit>(
		GetWorld()->SpawnActorDeferred<APlayerUnit>(
			APlayerUnit::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn));

	if (!NewUnit)
		return nullptr;

	NewUnit->GetCapsuleComponent()->SetCapsuleHalfHeight(10.f, true);

	NewUnit->FinishSpawning(SpawnTransform);
	FixLocAndRot(NewUnit);

	NewUnit->GetMesh()->SetSkeletalMesh(Row->Mesh);

	if (Row->AnimationBlueprint)
	{
		NewUnit->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		NewUnit->GetMesh()->SetAnimInstanceClass(Row->AnimationBlueprint);
	}

	NewUnit->AbilitySystemComponent->GrantedAbilities = Row->DefaultAbilities;

	return NewUnit;
}