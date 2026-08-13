#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/AbilityInputID.h"
#include "AbilitySystem.generated.h"

//Forward declaration
class UAbility;
class ACharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAbilitySystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilitySystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	EAbilityInputID ActiveAbilityInputID = EAbilityInputID::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EAbilityInputID, TSubclassOf<UAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAbility* ActiveAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ACharacter *MyOwner = nullptr;

	UFUNCTION(BlueprintCallable)
	void AddAbility(TSubclassOf<UAbility> AbilityClass, EAbilityInputID InputID);

	UFUNCTION(BlueprintCallable)  
	void RemoveAbility(TSubclassOf<UAbility> AbilityClass);

	UFUNCTION(BlueprintCallable)
	void RemoveAbilityAtSlot(EAbilityInputID InputID);
	
	void InitializeAbility(EAbilityInputID InputID);
	void OnAbilityInputReleased(EAbilityInputID ReleasedInputID);
	void HandleModifyInput();
};
