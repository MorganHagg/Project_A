#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AUnitBase;
class UUnitSpawner;
class UEnemyUnitData;

UCLASS()
class PROJECT_A_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	UEnemyUnitData* UnitData;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawning")
	void SpawnUnit();

protected:
	virtual void BeginPlay() override;

	UUnitSpawner* UnitSpawner;
};