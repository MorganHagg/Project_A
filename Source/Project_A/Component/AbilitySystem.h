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

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY()
    AUnitBase* MyOwner;
    
public:
    
    UPROPERTY(BlueprintReadOnly)
    UAbility* ActiveAbility = nullptr;

    void InstantiateAbilities(TArray<TSubclassOf<UAbility>> AbilityArray);

    UFUNCTION(BlueprintCallable)
    bool SwapAbility(TSubclassOf<UAbility> OldAbilityClass, TSubclassOf<UAbility> NewAbilityClass);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    TArray<UAbility*> GrantedAbilities;

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
    
};