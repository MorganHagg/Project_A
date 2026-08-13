#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnitManager.generated.h"

class UAbility;
class UBehaviorTree;
class ACharacter;
class AUnitBase;
class APlayerUnit;
class UDataTable;

struct PlayerSpawnDataRow;

UCLASS()
class PROJECT_A_API UUnitManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void FixLocAndRot(ACharacter* NewUnit);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	AUnitBase* SpawnUnit(FName RowName, const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	APlayerUnit* SpawnPlayerUnit(FName RowName, FVector Location);

	UPROPERTY(BlueprintReadOnly)
	UDataTable* UnitDataTable;

	UPROPERTY(BlueprintReadOnly)
	UDataTable* PlayerDataTable;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetSpawnableArchetypes() const
	{
		if (!UnitDataTable) return {};
		return UnitDataTable->GetRowNames();
	}
};