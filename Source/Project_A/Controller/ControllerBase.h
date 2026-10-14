#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "../Component/AbilityComponent.h"
#include "ControllerBase.generated.h"

class APlayerUnit;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJECT_A_API AControllerBase : public APlayerController
{
	GENERATED_BODY()

	AControllerBase();


protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void PossessPlayerUnit(APlayerUnit* NewPlayerUnit);
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	APlayerUnit* PlayerUnit;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* MappingContext_Control;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Primary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Secondary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float HoldThreshold = 0.3f;

private:
	// Maps each bound InputAction to the ability input it casts (see UAbilityComponent::GetInputAbilities).
	TMap<TObjectPtr<UInputAction>, EAbilityInput> AbilityInputMap;

	bool ResolveAbilityInput(const FInputActionInstance& Instance, EAbilityInput& OutInput);

	// Tracks each input currently being held, from press until release/resolution.
	struct FAbilityPressState
	{
		FTimerHandle HoldTimerHandle;
		bool bHoldThresholdMet = false;
	};
	TMap<EAbilityInput, FAbilityPressState> PressedInputs;

	// The input whose hold ability is currently active, if any. Modify always acts on this input's Modify ability.
	TOptional<EAbilityInput> ActiveHoldInput;

	void Move(const FInputActionValue& Value);
	void FaceMouseCursor();

	void OnAbilityInputPressed(const FInputActionInstance& Instance);
	void OnAbilityInputReleased(const FInputActionInstance& Instance);
	void OnHoldThresholdMet(EAbilityInput Input);

	// Activates Input's hold ability, as if its threshold had just been met.
	void ActivateHold(EAbilityInput Input);

	// Applies the Modify ability of ActiveHoldInput.
	void ApplyModify();
};