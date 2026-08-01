#pragma once
#include "CoreMinimal.h"
#include "Gameframework/Character.h"
#include "UnitBase.generated.h"

UCLASS()
class PROJECT_A_API AUnitBase : public ACharacter
{
	GENERATED_BODY()

public:
	AUnitBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
