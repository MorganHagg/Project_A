#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "ControllerBase.generated.h"

// Ability input enum
UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	Ability0 = 0, 
	Ability1 = 1,
	Ability2 = 2
};

// Forward declaration
class APlayerUnit;

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
	
	// Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* MappingContext_Control;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Dodge;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Ability1;
	 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Ability2;

	

private:
	// Used for auto assigning abilities to right and left clicks
	TMap<TObjectPtr<UInputAction>, EAbilityInputID> AbilityInputMap;

	// Input handling functions
	void Move(const FInputActionValue& Value);
	void FaceMouseCursor();

	void OnAbilityInputPressed(const FInputActionInstance& Instance);
	void OnAbilityInputReleased(const FInputActionInstance& Instance);
	
};