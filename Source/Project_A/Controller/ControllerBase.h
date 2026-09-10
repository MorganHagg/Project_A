#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
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
	// Maps each bound InputAction to its base slot (Tap slot; Hold = base+1, Modify = base+2)
	TMap<TObjectPtr<UInputAction>, int32> AbilityInputMap;

	bool ResolveAbilitySlot(const FInputActionInstance& Instance, int32& OutBaseSlot);

	// Tracks each base slot currently being held, from press until release/resolution.
	struct FAbilityPressState
	{
		FTimerHandle HoldTimerHandle;
		bool bHoldThresholdMet = false;
	};
	TMap<int32, FAbilityPressState> PressedSlots;

	// Base slot of the hold ability currently active, if any. Modify always acts on this slot (base+2).
	int32 ActiveHoldBaseSlot = -1;

	void Move(const FInputActionValue& Value);
	void FaceMouseCursor();

	void OnAbilityInputPressed(const FInputActionInstance& Instance);
	void OnAbilityInputReleased(const FInputActionInstance& Instance);
	void OnHoldThresholdMet(int32 BaseSlot);

	// Activates the hold ability at BaseSlot, as if its threshold had just been met.
	void ActivateHold(int32 BaseSlot);

	// Applies the Modify ability for whichever slot is in ActiveHoldBaseSlot.
	void ApplyModify();
};