#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Talent/TalentBase.h"
#include "TalentComponent.generated.h"

class UTalentData;
class UAbilitySystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UTalentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTalentComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UTalentData*> AcquiredTalents;

	bool bContainAcquiredTalents(UTalentData* Talents);

	UPROPERTY()
	UAbilitySystem* AbilitySystemComponent;
};
