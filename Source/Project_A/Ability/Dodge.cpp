#include "Dodge.h"
#include "Gameframework/Character.h"
#include "Gameframework/CharacterMovementComponent.h"

void UDodge::OnInstant()
{
	Super::OnInstant();
	if (MyCaster)
	{
		MyCaster->GetCharacterMovement()->StopMovementImmediately();
        
		// Get player controller
		APlayerController* PC = Cast<APlayerController>(MyCaster->GetController());
		if (!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("Controller is not APlayerController"));
			return;
		}
        
		// Get cursor location in world
		FHitResult HitResult;
		PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		FVector CursorLocation = HitResult.Location;
        
		// Get player location
		FVector PlayerLocation = MyCaster->GetActorLocation();
        
		// Calculate direction FROM cursor TO player (backwards from cursor)
		FVector Direction = (PlayerLocation - CursorLocation).GetSafeNormal();
        
		// Launch backwards
		float Distance = 1000.f;
		MyCaster->LaunchCharacter(Direction * Distance, true, true);
	}
}
