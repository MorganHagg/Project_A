#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HealthBarLayoutSubsystem.generated.h"

class UHealthBarWidget;

// ============================================================================
// UHealthBarLayoutSubsystem
// Keeps screen-space health bars from overlapping. Every frame it projects each
// shown bar to the screen and, lowest on screen first, pushes any bar that
// overlaps an already-placed one upward until it's clear - crowded bars form a
// column above the group. The push is applied as the widget's render
// translation (smoothed), so the widget component itself is untouched.
// Bars register themselves (UHealthBarWidget::NativeConstruct/NativeDestruct).
// ============================================================================

UCLASS()
class PROJECT_A_API UHealthBarLayoutSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterHealthBar(UHealthBarWidget* HealthBar);
	void UnregisterHealthBar(UHealthBarWidget* HealthBar);

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	// Game and PIE only - no layout work in editor worlds or widget designer previews.
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TArray<TWeakObjectPtr<UHealthBarWidget>> HealthBars;

	// Vertical gap between stacked bars, in UI (Slate) units.
	static constexpr float StackPadding = 2.f;

	// How quickly bars slide toward their stacked position (FInterpTo speed).
	static constexpr float SlideSpeed = 15.f;
};
