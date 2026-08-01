#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerUnit.generated.h"

class UAbilitySystem;
class UAttributeSet;
class UGameplayEffect;
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
	
	// Custom Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystem* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FString, UGameplayEffect*> GameplayEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAttributeSet *Attributes;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	

};
