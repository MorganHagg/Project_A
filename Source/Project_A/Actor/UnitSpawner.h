#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "UnitSpawner.generated.h"

class AUnitBase;
class UUnitManager;

UCLASS()
class PROJECT_A_API AUnitSpawner : public AActor
{
	GENERATED_BODY()

public:
	AUnitSpawner();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FDataTableRowHandle UnitToSpawn;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawning")
	void SpawnUnit();

protected:
	virtual void BeginPlay() override;

	UUnitManager* UnitManager;
};