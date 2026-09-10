#pragma once
#include "CoreMinimal.h"
#include "UnitDataBase.h"
#include "PlayerUnitData.generated.h"

class UTalentData;
class APlayerUnit;

UCLASS(BlueprintType)
class PROJECT_A_API UPlayerUnitData : public UUnitDataBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UTalentData*> Talents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APlayerUnit> CustomUnitClass;
};