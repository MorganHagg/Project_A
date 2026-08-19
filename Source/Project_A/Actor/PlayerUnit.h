#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "PlayerUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UTalentComponent;

UCLASS()
class PROJECT_A_API APlayerUnit : public AUnitBase
{
	GENERATED_BODY()

public:
	APlayerUnit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTalentComponent* TalentComponent;

	virtual void Initiate(UUnitDataBase* SpawnData) override;
	

	void AdjustCamera();
};