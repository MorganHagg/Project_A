#include "Projectile.h"
#include "Gameframework/Character.h"
#include "../Component/EffectHandler.h"
#include "Components/StaticMeshComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	// No mass / no physics - query-only collision, overlap rather than block.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleComponentBeginOverlap);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Travel(DeltaTime);
}

void AProjectile::Travel(float DeltaTime)
{
	FVector Direction = (Destination - GetActorLocation()).GetSafeNormal();
	FVector NewLocation = GetActorLocation() + Direction * Speed * DeltaTime;

	// Sweep=true so fast-moving projectiles still generate overlaps along the
	// path instead of only checking the final landing point.
	SetActorLocation(NewLocation, true);
	SetActorRotation(Direction.Rotation());

	if (FVector::Dist(NewLocation, Destination) < 25.f)
	{
		OnFinished(NewLocation);
		Destroy();
	}
}

void AProjectile::SetMyAbility(UAbility* Ability)
{
	MyAbility = Ability;
}

void AProjectile::SetMyCaster(ACharacter* Caster)
{
	MyCaster = Caster;
}

void AProjectile::HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// TODO: Change this so that it applies effect on every target hit. Then checks whether it should continue
	// and then give location at where it dies
	// Use this:
	/*UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>();

	if (EffectHandler)
	{
		EffectHandler->YourFunction();
	}*/
	
	if (!bReactToImpact || OtherActor == MyCaster)
	{
		return;
	}

	if (AlreadyHitActors.Contains(OtherActor))
	{
		return;
	}
	AlreadyHitActors.Add(OtherActor);

	FVector OverlapLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	// Infinite penetration: report the hit (effects still apply) but never stop.
	if (PenetrationCount == -1)
	{
		OnFinished(OverlapLocation);
		return;
	}

	// Still have penetrations left: report the hit but keep flying.
	if (PenetrationsSoFar < PenetrationCount)
	{
		PenetrationsSoFar++;
		OnFinished(OverlapLocation);
		return;
	}

	// Out of penetrations (or PenetrationCount == 0): stop here.
	OnFinished(OverlapLocation);
	Destroy();
}

void AProjectile::OnFinished(FVector HitLocation)
{
	OnHit.ExecuteIfBound(HitLocation);
}