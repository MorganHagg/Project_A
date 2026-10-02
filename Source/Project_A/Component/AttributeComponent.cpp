#include "AttributeComponent.h"
#include "../DataAsset/UnitDataBase.h"
#include "../Unit/UnitBase.h"
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
		StoreAttribute(Pair.Key, Pair.Value);
	}

	// UUnitDataBase only configures a single Health value; use it as the starting max as well.
	StoreAttribute(EAttributeType::MaxHealth, Attributes.FindRef(EAttributeType::Health));
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
		StoreAttribute(EAttributeType::MaxHealth, Value);
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Delta);
		return;
	}
	if (Type == EAttributeType::Speed)
	{
		if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
		{
			MovementComponent->MaxWalkSpeed = Value;
			OnAttributeChanged.Broadcast(EAttributeType::Speed, MovementComponent->MaxWalkSpeed);
		}
		return;
	}

	StoreAttribute(Type, Value);
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
		StoreAttribute(EAttributeType::MaxHealth, Attributes.FindRef(EAttributeType::MaxHealth) + Amount);
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Amount);
		return;
	}
	if (Type == EAttributeType::Speed)
	{
		if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
		{
			MovementComponent->MaxWalkSpeed += Amount;
			OnAttributeChanged.Broadcast(EAttributeType::Speed, MovementComponent->MaxWalkSpeed);
		}
		return;
	}

	StoreAttribute(Type, Attributes.FindRef(Type) + Amount);
}

void UAttributeComponent::StoreAttribute(EAttributeType Type, float Value)
{
	Attributes.Add(Type, Value);
	OnAttributeChanged.Broadcast(Type, Value);
}

UCharacterMovementComponent* UAttributeComponent::GetMovementComponent() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character ? Character->GetCharacterMovement() : nullptr;
}

void UAttributeComponent::SetHealthValue(float NewValue)
{
	if (bIsDead)
	{
		return;
	}

	const float MaxHealth = Attributes.FindRef(EAttributeType::MaxHealth);
	const float ClampedValue = FMath::Clamp(NewValue, 0.f, MaxHealth);
	StoreAttribute(EAttributeType::Health, ClampedValue);

	if (ClampedValue <= 0.f)
	{
		bIsDead = true;

		// Called here rather than from OnDeath_Implementation so a Blueprint override can't skip it.
		if (AUnitBase* Unit = Cast<AUnitBase>(GetOwner()))
		{
			Unit->HandleDeath();
		}

		OnDeath();
	}
}

void UAttributeComponent::OnDeath_Implementation()
{
}

