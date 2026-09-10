#pragma once
#include "Runtime/Core/Public/CoreMinimal.h"
#include "AttributeSet.generated.h"

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	Health,
	MaxHealth,
	Strength,
	Intellect,
	Armour,
	MagicResist,
	Speed,
	Crit,

	Count	UMETA(Hidden)	// Gives the number of entries - Used in AUnitBase's constructor
};
