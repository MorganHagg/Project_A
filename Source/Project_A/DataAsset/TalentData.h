#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TalentData.generated.h"

class UTalentComponent;

UCLASS()
class PROJECT_A_API UTalentData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	
	UFUNCTION( BlueprintCallable )
	bool bPrerequisitesMet(UTalentComponent* TalentComponent);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UTalentData*> Prerequisites;
};
