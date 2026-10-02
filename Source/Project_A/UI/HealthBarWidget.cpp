#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "HealthBarLayoutSubsystem.h"
#include "../Component/AttributeComponent.h"
#include "../Unit/UnitBase.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Null outside game worlds (e.g. the Widget Blueprint designer preview).
	if (UHealthBarLayoutSubsystem* Layout = GetWorld() ? GetWorld()->GetSubsystem<UHealthBarLayoutSubsystem>() : nullptr)
	{
		Layout->RegisterHealthBar(this);
	}
}

void UHealthBarWidget::NativeDestruct()
{
	if (UHealthBarLayoutSubsystem* Layout = GetWorld() ? GetWorld()->GetSubsystem<UHealthBarLayoutSubsystem>() : nullptr)
	{
		Layout->UnregisterHealthBar(this);
	}

	Super::NativeDestruct();
}

void UHealthBarWidget::SetOwnerUnit(AUnitBase* Unit)
{
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributeChanged.RemoveDynamic(this, &UHealthBarWidget::HandleAttributeChanged);
	}

	OwnerUnit = Unit;
	AttributeComponent = Unit ? Unit->AttributeComponent : nullptr;
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributeChanged.AddUniqueDynamic(this, &UHealthBarWidget::HandleAttributeChanged);
	}

	RefreshHealth();
}

bool UHealthBarWidget::ShouldBeLaidOut() const
{
	return IsVisible() && OwnerUnit.IsValid() && OwnerUnit->HealthBarComponent && OwnerUnit->HealthBarComponent->IsVisible();
}

FVector UHealthBarWidget::GetAnchorLocation() const
{
	return OwnerUnit.IsValid() && OwnerUnit->HealthBarComponent
		? OwnerUnit->HealthBarComponent->GetComponentLocation()
		: FVector::ZeroVector;
}

void UHealthBarWidget::HandleAttributeChanged(EAttributeType Attribute, float NewValue)
{
	if (Attribute == EAttributeType::Health || Attribute == EAttributeType::MaxHealth)
	{
		RefreshHealth();
	}
}

void UHealthBarWidget::RefreshHealth()
{
	if (!HealthBar || !AttributeComponent)
	{
		return;
	}

	const float MaxHealth = AttributeComponent->GetAttribute(EAttributeType::MaxHealth);
	const float Health = AttributeComponent->GetAttribute(EAttributeType::Health);
	HealthBar->SetPercent(MaxHealth > 0.f ? Health / MaxHealth : 0.f);

	if (HealthText)
	{
		HealthText->SetText(FText::Format(INVTEXT("{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(Health)), FText::AsNumber(FMath::RoundToInt(MaxHealth))));
	}

	if (Health < MaxHealth)
	{
		bHasBeenDamaged = true;
	}

	// HitTestInvisible: the bar is display-only and shouldn't swallow mouse input aimed at the world.
	SetVisibility(!bShowOnlyWhenDamaged || bHasBeenDamaged ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
