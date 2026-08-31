#include "Projectile.h"
#include "../Unit/UnitBase.h"
#include "Ability.h"
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

	SetActorLocation(NewLocation, true);
	SetActorRotation(Direction.Rotation());

	if (FVector::Dist(NewLocation, Destination) < 25.f)
	{
		Finish(NewLocation);
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
	if (!bReactToImpact || OtherActor == MyCaster || AlreadyHitActors.Contains(OtherActor))
	{
		return;
	}
	
	if (AUnitBase* HitUnit = Cast<AUnitBase>(OtherActor))
	{
		if (UEffectHandler* EffectHandler = HitUnit->FindComponentByClass<UEffectHandler>())
		{
			EffectHandler->AddEffect(MyEffect);
		}
		MyAbility->DelegateOnHit(HitUnit);
	}
	AlreadyHitActors.Add(OtherActor);

	FVector OverlapLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	if (PenetrationCount == -1 || PenetrationsSoFar < PenetrationCount)
	{
		PenetrationsSoFar++;
		OnHit.ExecuteIfBound(OverlapLocation);
		return;
	}

	Finish(OverlapLocation);
}

void AProjectile::Finish(FVector HitLocation)
{
	if (bHasFinished)
	{
		return;
	}
	bHasFinished = true;
	OnFinished.ExecuteIfBound(HitLocation);
	Destroy();
}