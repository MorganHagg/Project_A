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
/*
*
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AttributeSet.generated.h"

USTRUCT(BlueprintType)
struct FResource
{
GENERATED_BODY()
FResource() {}
FResource(float InMax, FString InName)
: Current(InMax), Max(InMax), Name(InName) {}
	
	
UPROPERTY(BlueprintReadOnly)
float Current;
	
UPROPERTY(BlueprintReadOnly)
float Max;

UPROPERTY(BlueprintReadOnly)
FString Name;
	
float ReturnRatio() { return Current/Max;};
};

UCLASS()
class PROJECT_D_API UAttributeSet : public UActorComponent
{
GENERATED_BODY()
UAttributeSet(){
Health = FResource(100, TEXT("Health"));
};

public: 
UPROPERTY(BlueprintReadOnly)
FResource Health;
	
};*/