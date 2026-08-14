#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnitManager.generated.h"

class UAbility;
class UBehaviorTree;
class ACharacter;
class AUnitBase;
class APlayerUnit;
class AEnemyUnit;
class UDataTable;
class USkeletalMesh;

UCLASS()
class PROJECT_A_API UUnitManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

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
	UFUNCTION(BlueprintCallable)
	AEnemyUnit* SpawnUnit(FName RowName, const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	APlayerUnit* SpawnPlayerUnit(FName RowName, FVector Location);

private:
	void FixLocAndRot(ACharacter* NewUnit);

	void ApplyCommonSpawnData(
		AUnitBase* NewUnit,
		USkeletalMesh* Mesh,
		const TArray<TSubclassOf<UAbility>>& DefaultAbilities);
};