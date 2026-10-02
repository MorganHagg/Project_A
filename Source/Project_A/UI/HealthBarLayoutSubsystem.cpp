#include "HealthBarLayoutSubsystem.h"
#include "HealthBarWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

void UHealthBarLayoutSubsystem::RegisterHealthBar(UHealthBarWidget* HealthBar)
{
	HealthBars.AddUnique(HealthBar);
}

void UHealthBarLayoutSubsystem::UnregisterHealthBar(UHealthBarWidget* HealthBar)
{
	HealthBars.Remove(HealthBar);
}

bool UHealthBarLayoutSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UHealthBarLayoutSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHealthBarLayoutSubsystem, STATGROUP_Tickables);
}

void UHealthBarLayoutSubsystem::Tick(float DeltaTime)
{
	HealthBars.RemoveAll([](const TWeakObjectPtr<UHealthBarWidget>& Bar) { return !Bar.IsValid(); });

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	// Projection is in screen pixels, widget sizes and render translations are in UI units scaled by DPI.
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (!PlayerController || ViewportScale <= 0.f)
	{
		return;
	}

	struct FBarLayout
	{
		UHealthBarWidget* Bar;
		FVector2D Center;	// Unstacked on-screen center, UI units
		FVector2D Size;
		float TargetOffsetY;
	};

	TArray<FBarLayout> Layouts;
	for (const TWeakObjectPtr<UHealthBarWidget>& WeakBar : HealthBars)
	{
		UHealthBarWidget* Bar = WeakBar.Get();
		FVector2D ScreenPosition;
		if (!Bar->ShouldBeLaidOut() ||
			!PlayerController->ProjectWorldLocationToScreen(Bar->GetAnchorLocation(), ScreenPosition, /*bPlayerViewportRelative=*/true))
		{
			Bar->SetRenderTranslation(FVector2D::ZeroVector);
			continue;
		}
		Layouts.Add({ Bar, ScreenPosition / ViewportScale, Bar->GetDesiredSize(), 0.f });
	}

	// Lowest on screen first (largest Y), so stacks build upward from the bar nearest the ground.
	// Y is rounded and ties broken by object ID, so bars at the same height keep a stable order
	// instead of swapping (and visibly jittering) from frame to frame.
	Layouts.Sort([](const FBarLayout& A, const FBarLayout& B)
	{
		const int32 AY = FMath::RoundToInt(A.Center.Y);
		const int32 BY = FMath::RoundToInt(B.Center.Y);
		return AY != BY ? AY > BY : A.Bar->GetUniqueID() < B.Bar->GetUniqueID();
	});

	for (int32 Index = 0; Index < Layouts.Num(); ++Index)
	{
		FBarLayout& Current = Layouts[Index];

		// Push up past any already-placed bar it overlaps. Each push moves it strictly above that
		// bar, and it only ever moves up, so this settles within Index pushes.
		bool bPushed = true;
		while (bPushed)
		{
			bPushed = false;
			const float CurrentY = Current.Center.Y + Current.TargetOffsetY;
			for (int32 PlacedIndex = 0; PlacedIndex < Index; ++PlacedIndex)
			{
				const FBarLayout& Placed = Layouts[PlacedIndex];
				const float PlacedY = Placed.Center.Y + Placed.TargetOffsetY;
				const float MinGapY = (Current.Size.Y + Placed.Size.Y) * 0.5f + StackPadding;
				const bool bOverlapsX = FMath::Abs(Current.Center.X - Placed.Center.X) < (Current.Size.X + Placed.Size.X) * 0.5f;
				if (bOverlapsX && FMath::Abs(CurrentY - PlacedY) < MinGapY)
				{
					Current.TargetOffsetY = PlacedY - MinGapY - Current.Center.Y;
					bPushed = true;
					break;
				}
			}
		}

		const float CurrentOffsetY = Current.Bar->GetRenderTransform().Translation.Y;
		Current.Bar->SetRenderTranslation(FVector2D(0.f, FMath::FInterpTo(CurrentOffsetY, Current.TargetOffsetY, DeltaTime, SlideSpeed)));
	}
}
