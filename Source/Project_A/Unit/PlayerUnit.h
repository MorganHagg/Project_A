#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "PlayerUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UTalentComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReceiveDamage, float, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReceiveHeal, float, Amount);

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

	virtual void SetupUnit(UUnitDataBase* SpawnData) override;


	void AdjustCamera();

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnPlayerReceiveDamage OnReceiveDamage;

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnPlayerReceiveHeal OnReceiveHeal;

	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveHeal(float Amount);

	void DelegateOnReceiveDamage(float Amount);
	void DelegateOnReceiveHeal(float Amount);
};