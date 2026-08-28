#include "AttributeComponent.h"
#include "../DataAsset/UnitDataBase.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	Attributes.SetNumZeroed(static_cast<int32>(EAttributeType::Count));
}

void UAttributeComponent::InstantiateAttributes(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	Attributes = UnitData->Attributes;
}

float UAttributeComponent::GetAttribute(EAttributeType Type) const
{
	const int32 Index = static_cast<int32>(Type);

	if (!Attributes.IsValidIndex(Index))
	{
		return 0.f;
	}

	return Attributes[Index];
}

void UAttributeComponent::SetAttribute(EAttributeType Type, float Value)
{
	const int32 Index = static_cast<int32>(Type);

	if (!Attributes.IsValidIndex(Index))
	{
		return;
	}

	Attributes[Index] = Value;
}

void UAttributeComponent::ModifyAttribute(EAttributeType Type, float Amount)
{
	// Test comment to verify the build command

	const int32 Index = static_cast<int32>(Type);

	if (!Attributes.IsValidIndex(Index))
	{
		return;
	}

	Attributes[Index] += Amount;
}

