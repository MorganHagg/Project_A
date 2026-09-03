#include "AttributeComponent.h"
#include "../DataAsset/UnitDataBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	for (uint8 Index = 0; Index < static_cast<uint8>(EAttributeType::Count); ++Index)
	{
		const EAttributeType Type = static_cast<EAttributeType>(Index);
		if (Type != EAttributeType::Speed)
		{
			Attributes.Add(Type, 0.f);
		}
	}
}

void UAttributeComponent::InstantiateAttributes(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (const TPair<EAttributeType, float>& Pair : UnitData->DefaultAttributes)
	{
		if (Pair.Key == EAttributeType::Speed)
		{
			SetAttribute(EAttributeType::Speed, Pair.Value);
			continue;
		}
		Attributes.Add(Pair.Key, Pair.Value);
	}

	// UUnitDataBase only configures a single Health value; use it as the starting max as well.
	Attributes.Add(EAttributeType::MaxHealth, Attributes.FindRef(EAttributeType::Health));
}

float UAttributeComponent::GetAttribute(EAttributeType Type) const
{
	if (Type == EAttributeType::Speed)
	{
		const UCharacterMovementComponent* MovementComponent = GetMovementComponent();
		return MovementComponent ? MovementComponent->MaxWalkSpeed : 0.f;
	}

	return Attributes.FindRef(Type);
}

void UAttributeComponent::SetAttribute(EAttributeType Type, float Value)
{
	if (Type == EAttributeType::Health)
	{
		SetHealthValue(Value);
		return;
	}
	if (Type == EAttributeType::MaxHealth)
	{
		const float Delta = Value - Attributes.FindRef(EAttributeType::MaxHealth);
		Attributes.Add(EAttributeType::MaxHealth, Value);
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Delta);
		return;
	}
	if (Type == EAttributeType::Speed)
	{
		if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
		{
			MovementComponent->MaxWalkSpeed = Value;
		}
		return;
	}

	Attributes.Add(Type, Value);
}

void UAttributeComponent::ModifyAttribute(EAttributeType Type, float Amount)
{
	if (Type == EAttributeType::Health)
	{
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Amount);
		return;
	}
	if (Type == EAttributeType::MaxHealth)
	{
		Attributes.Add(EAttributeType::MaxHealth, Attributes.FindRef(EAttributeType::MaxHealth) + Amount);
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Amount);
		return;
	}
	if (Type == EAttributeType::Speed)
	{
		if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
		{
			MovementComponent->MaxWalkSpeed += Amount;
		}
		return;
	}

	Attributes.Add(Type, Attributes.FindRef(Type) + Amount);
}

UCharacterMovementComponent* UAttributeComponent::GetMovementComponent() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character ? Character->GetCharacterMovement() : nullptr;
}

void UAttributeComponent::SetHealthValue(float NewValue)
{
	const float MaxHealth = Attributes.FindRef(EAttributeType::MaxHealth);
	const float ClampedValue = FMath::Clamp(NewValue, 0.f, MaxHealth);
	Attributes.Add(EAttributeType::Health, ClampedValue);

	if (ClampedValue <= 0.f)
	{
		if (!bIsDead)
		{
			bIsDead = true;
			OnDeath();
		}
	}
	else
	{
		bIsDead = false;
	}
}

void UAttributeComponent::OnDeath_Implementation()
{
}

