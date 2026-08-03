#include "Projectile.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	// Was "ignore all" before, which meant nothing could ever generate a hit.
	// Block the channels you want to react to (adjust to your project's setup).
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	MeshComponent->SetNotifyRigidBodyCollision(true);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->OnComponentHit.AddDynamic(this, &AProjectile::HandleComponentHit);
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

	SetActorLocation(NewLocation);
	SetActorRotation(Direction.Rotation());

	if (FVector::Dist(NewLocation, Destination) < 5.f)
	{
		OnFinished(NewLocation);
		Destroy();
	}
}

void AProjectile::SetMyAbility(UAbility* Ability)
{
	MyAbility = Ability;
}

void AProjectile::HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bReactToImpact)
	{
		return;
	}

	OnFinished(Hit.ImpactPoint);
	Destroy();
}

void AProjectile::OnFinished(FVector HitLocation)
{
	OnHit.ExecuteIfBound(HitLocation);
}