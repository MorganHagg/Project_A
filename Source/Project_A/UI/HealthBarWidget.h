#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Misc/AttributeSet.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAttributeComponent;
class AUnitBase;

// ============================================================================
// UHealthBarWidget
// Floating health bar shown above a unit (via AUnitBase::HealthBarComponent).
// All behavior lives here; a Widget Blueprint subclass (WBP_HealthBar) only
// supplies the layout and styling, and must contain a Progress Bar named
// "HealthBar" - BindWidget fails its compile otherwise. A Text Block named
// "HealthText" is optional.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Binds to Unit's attribute changes and shows its current Health / MaxHealth immediately.
	void SetOwnerUnit(AUnitBase* Unit);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// Optional "Health / MaxHealth" readout - only set if the Widget Blueprint has a Text Block named "HealthText".
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

private:
	UFUNCTION()
	void HandleAttributeChanged(EAttributeType Attribute, float NewValue);

	void RefreshHealth();

	UPROPERTY()
	TObjectPtr<UAttributeComponent> AttributeComponent;
};
