#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem.generated.h"

class UAbility;
class AUnitBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAbilitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilitySystem();
    
    UPROPERTY(BlueprintReadOnly)
    UAbility* ActiveAbility = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UAbility>> GrantedAbilities;

    UFUNCTION(BlueprintCallable)
    void AddAbility(int32 Slot, TSubclassOf<UAbility> AbilityClass);

    UFUNCTION(BlueprintCallable)
    void RemoveAbility(int32 Slot);

    UAbility* InitiateAbility(int32 Slot);
    
    UFUNCTION(BlueprintCallable)
    void SetActiveAbility(UAbility* NewActiveAbility);

    virtual void TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction
) override;

    UFUNCTION(BlueprintCallable)
    void EndActiveAbility();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    AUnitBase* MyOwner;
};