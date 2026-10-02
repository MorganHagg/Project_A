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
// "HealthText" is optional. Overlapping bars are stacked apart by
// UHealthBarLayoutSubsystem.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Binds to Unit's attribute changes and shows its current Health / MaxHealth immediately.
	void SetOwnerUnit(AUnitBase* Unit);

	// If true, the bar stays hidden until the unit first drops below full health, then stays shown
	// until the unit dies (healing back to full doesn't hide it again). If false, it's always shown.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health Bar")
	bool bShowOnlyWhenDamaged = false;

	// Whether UHealthBarLayoutSubsystem should place this bar: shown, and its unit's bar not hidden by death.
	bool ShouldBeLaidOut() const;

	// World location the bar is drawn at (its widget component).
	FVector GetAnchorLocation() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

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

	TWeakObjectPtr<AUnitBase> OwnerUnit;

	// Latched the first time Health is seen below MaxHealth - see bShowOnlyWhenDamaged.
	bool bHasBeenDamaged = false;
};
