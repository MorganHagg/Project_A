#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnitSpawner.generated.h"

class UAbility;
class ACharacter;
class AUnitBase;
class APlayerUnit;
class AEnemyUnit;
class USkeletalMesh;
class UUnitDataBase;
class UEnemyUnitData;
class UPlayerUnitData;

UCLASS()
class PROJECT_A_API UUnitSpawner : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	AEnemyUnit* SpawnUnit(UEnemyUnitData* SpawnData, const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	APlayerUnit* SpawnPlayerUnit(UPlayerUnitData* SpawnData, FVector Location);

private:
	void FixLocAndRot(AUnitBase* NewUnit);

	AUnitBase* SpawnUnitInternal(
	   UClass* UnitClass,
	   UUnitDataBase* SpawnData,
	   const FTransform& SpawnTransform);

	void ApplyCommonSpawnData(
	   AUnitBase* NewUnit,
	   UUnitDataBase* SpawnData);
};