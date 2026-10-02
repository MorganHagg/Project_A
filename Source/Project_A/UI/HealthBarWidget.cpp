#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "../Component/AttributeComponent.h"
#include "../Unit/UnitBase.h"

void UHealthBarWidget::SetOwnerUnit(AUnitBase* Unit)
{
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributeChanged.RemoveDynamic(this, &UHealthBarWidget::HandleAttributeChanged);
	}

	AttributeComponent = Unit ? Unit->AttributeComponent : nullptr;
	if (AttributeComponent)
	{
		AttributeComponent->OnAttributeChanged.AddUniqueDynamic(this, &UHealthBarWidget::HandleAttributeChanged);
	}

	RefreshHealth();
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
}
