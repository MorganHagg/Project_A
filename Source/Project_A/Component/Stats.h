#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stats.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UStats : public UActorComponent
{
	GENERATED_BODY()

public:
	UStats();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
