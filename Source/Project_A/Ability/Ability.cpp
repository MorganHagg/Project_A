#include "Ability.h"
#include "Gameframework/Character.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Projectile.h"
#include "../GameplayEffect/GameplayEffect.h"
#include "../Component/EffectHandler.h"

UAbility::UAbility()
{
	AbilityUUID = FGuid::NewGuid().ToString();
	World = GetWorld();
}

FString UAbility::GetAbilityUUID()
{
	return AbilityUUID;
}

void UAbility::Initiate(ACharacter* NewCaster)
{
	MyCaster = NewCaster;
	World = MyCaster->GetWorld();

	check(MyCaster);
	check(World);

	ActivateAbility(MyCaster);
}

void UAbility::ActivateAbility(ACharacter* NewCaster)
{
	UE_LOG(LogTemp, Log, TEXT("UAbility::ActivateAbility()"));
	MyCaster = NewCaster;
	World = MyCaster ? MyCaster->GetWorld() : World;

	OnActivate();
}

void UAbility::EndAbility()
{
	UE_LOG(LogTemp, Warning, TEXT("End ability"))

	OnEnd();

	MyCaster = nullptr;
	MyTarget = nullptr;
}

// ============================================================================
// Effect library
// ============================================================================
void UAbility::RunEffect_Target(UGameplayEffect* Effect, ACharacter* Target)
{
	UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>();
	if (EffectHandler)
	{
		EffectHandler->AddEffect(Effect);
	}
}

TArray<ACharacter*> UAbility::RunEffect_AOE(UGameplayEffect* Effect, FVector Location, float Radius, ETargetSelection TargetSelection)
{
	TArray<ACharacter*> Targets;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;

	if (World && World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			ACharacter* TargetCharacter = Cast<ACharacter>(Overlap.GetActor());
			if (!TargetCharacter)
			{
				continue;
			}

			if (TargetSelection == ETargetSelection::All)
			{
				Targets.AddUnique(TargetCharacter);
			}
		}
	}

	return Targets;
}

void UAbility::RunEffect_Projectile(FLatentActionInfo LatentInfo, UGameplayEffect* Effect, UStaticMesh* Mesh, FVector Target, float Speed, int32 PenetrationCount, FVector& OutLocation)
{
	if (Speed == 0.f)
		UE_LOG(LogTemp, Warning, TEXT("Projectile has 0 speed"));
	check(Mesh)

	if (!MyCaster || !World)
	{
		return;
	}

	FLatentActionManager& LAM = World->GetLatentActionManager();
	FEffect_ProjectileAction* ProjectileAction = new FEffect_ProjectileAction(LatentInfo, OutLocation);
	LAM.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ProjectileAction);

	AProjectile* Projectile = World->SpawnActor<AProjectile>(
		AProjectile::StaticClass(),
		MyCaster->GetActorLocation(),
		MyCaster->GetActorRotation()
	);
	Projectile->SetMyAbility(this);
	Projectile->SetMyCaster(MyCaster);
	Projectile->Destination = Target;
	Projectile->Speed = Speed;
	Projectile->PenetrationCount = PenetrationCount;
	Projectile->MeshComponent->IgnoreActorWhenMoving(MyCaster, true);

	if (Mesh)
	{
		Projectile->MeshComponent->SetStaticMesh(Mesh);
	}

	TWeakObjectPtr<UAbility> WeakThis = this;
	Projectile->OnHit.BindLambda([WeakThis, ProjectileAction](FVector HitLocation)
	{
		if (WeakThis.IsValid())
		{
			ProjectileAction->Finish(HitLocation);
		}
	});
}