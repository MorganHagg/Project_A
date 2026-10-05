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
		Attributes.Add(Type, 0.f);
		BaseAttributes.Add(Type, 0.f);
		PercentModifiers.Add(Type, 1.f);
	}
}

void UAttributeComponent::InstantiateAttributes(const UUnitDataBase* UnitData)
{
	if (!UnitData)
	{
		return;
	}

	for (const TPair<EAttributeType, float>& Pair : Attributes)
	{
		BaseAttributes.Add(Pair.Key, Pair.Value);
	}

	// A unit whose data doesn't configure Speed keeps its movement component's walk speed.
	if (const UCharacterMovementComponent* MovementComponent = GetMovementComponent())
	{
		BaseAttributes.Add(EAttributeType::Speed, MovementComponent->MaxWalkSpeed);
	}

	for (const TPair<EAttributeType, float>& Pair : UnitData->DefaultAttributes)
	{
		BaseAttributes.Add(Pair.Key, Pair.Value);
	}

	// UUnitDataBase only configures a single Health value; use it as the starting max as well.
	const float StartingHealth = BaseAttributes.FindRef(EAttributeType::Health);
	BaseAttributes.Add(EAttributeType::MaxHealth, StartingHealth);

	for (uint8 Index = 0; Index < static_cast<uint8>(EAttributeType::Count); ++Index)
	{
		const EAttributeType Type = static_cast<EAttributeType>(Index);
		if (Type == EAttributeType::Health)
		{
			continue;
		}

		// Not UpdateFinalValue - starting values are written directly, without moving Health.
		const float FinalValue = BaseAttributes.FindRef(Type) * PercentModifiers.FindRef(Type);
		if (Type == EAttributeType::Speed)
		{
			if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
			{
				MovementComponent->MaxWalkSpeed = FinalValue;
			}
		}
		StoreAttribute(Type, FinalValue);
	}

	StoreAttribute(EAttributeType::Health, StartingHealth);
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

	BaseAttributes.Add(Type, Value);
	UpdateFinalValue(Type);
}

void UAttributeComponent::ModifyAttribute(EAttributeType Type, float Amount)
{
	if (Type == EAttributeType::Health)
	{
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Amount);
		return;
	}

	BaseAttributes.Add(Type, BaseAttributes.FindRef(Type) + Amount);
	UpdateFinalValue(Type);
}

void UAttributeComponent::ModifyAttributePercent(EAttributeType Type, float Percent)
{
	// A Health percentage scales MaxHealth; UpdateFinalValue then moves Health by the same bonus.
	const EAttributeType ModifiedType = Type == EAttributeType::Health ? EAttributeType::MaxHealth : Type;

	PercentModifiers.Add(ModifiedType, PercentModifiers.FindRef(ModifiedType) + Percent / 100.f);
	UpdateFinalValue(ModifiedType);
}

void UAttributeComponent::UpdateFinalValue(EAttributeType Type)
{
	const float FinalValue = BaseAttributes.FindRef(Type) * PercentModifiers.FindRef(Type);

	if (Type == EAttributeType::MaxHealth)
	{
		const float Delta = FinalValue - Attributes.FindRef(EAttributeType::MaxHealth);
		StoreAttribute(EAttributeType::MaxHealth, FinalValue);
		SetHealthValue(Attributes.FindRef(EAttributeType::Health) + Delta, /*bCanKill=*/false);
		return;
	}

	if (Type == EAttributeType::Speed)
	{
		if (UCharacterMovementComponent* MovementComponent = GetMovementComponent())
		{
			MovementComponent->MaxWalkSpeed = FinalValue;
		}
	}

	StoreAttribute(Type, FinalValue);
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

void UAttributeComponent::SetHealthValue(float NewValue, bool bCanKill)
{
	if (bIsDead)
	{
		return;
	}

	const float MaxHealth = Attributes.FindRef(EAttributeType::MaxHealth);
	const float ClampedValue = FMath::Clamp(NewValue, bCanKill ? 0.f : 1.f, MaxHealth);
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
