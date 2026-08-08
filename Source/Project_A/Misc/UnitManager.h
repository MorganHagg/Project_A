#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnitManager.generated.h"

class UAbility;
class UBehaviorTree;
struct FPlayerUnitParams;

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
	APlayerUnit* SpawnPlayerUnit(FPlayerUnitParams SpawnParams, FVector Location);

	UPROPERTY(BlueprintReadOnly)
	UDataTable* UnitDataTable;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetSpawnableArchetypes() const
	{
		if (!UnitDataTable) return {};
		return UnitDataTable->GetRowNames();
	}
};
