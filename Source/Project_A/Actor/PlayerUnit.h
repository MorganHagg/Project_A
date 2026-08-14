#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "PlayerUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;

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

	void AdjustCamera();
};