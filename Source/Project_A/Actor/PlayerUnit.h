#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerUnit.generated.h"

class UAbilitySystem;
class UEffectHandler;
class UStats;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJECT_A_API APlayerUnit : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerUnit();

	void AdjustCamera();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	
	// System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAbilitySystem* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UEffectHandler* EffectHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStats* StatsComponent;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	

};
