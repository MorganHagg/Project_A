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
	UFUNCTION(BlueprintCallable)
	AEnemyUnit* SpawnUnit(UEnemyUnitData* SpawnData, const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	APlayerUnit* SpawnPlayerUnit(UPlayerUnitData* SpawnData, const FTransform& SpawnTransform);

private:

	AUnitBase* SpawnUnitInternal(
	   UClass* UnitClass,
	   UUnitDataBase* SpawnData,
	   const FTransform& SpawnTransform);
	
};