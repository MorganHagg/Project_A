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

UAbility* AProjectile::GetAbility()
{
	return MyAbility;
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

	AUnitBase* HitUnit = Cast<AUnitBase>(OtherActor);
	if (!HitUnit)
	{
		return;
	}

	if (UEffectHandler* EffectHandler = HitUnit->FindComponentByClass<UEffectHandler>())
	{
		EffectHandler->AddEffect(MyEffect);
	}

	FVector OverlapLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	FAbilityEventPayload Payload;
	Payload.Target = HitUnit;
	Payload.Location = OverlapLocation;
	Payload.AppliedEffect = MyEffect;
	Payload.Magnitude = MyEffect.Magnitude;
	ReportAbilityEvent(HitEventTag, Payload);

	AlreadyHitActors.Add(OtherActor);

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

	FAbilityEventPayload Payload;
	Payload.Location = HitLocation;
	Payload.AppliedEffect = MyEffect;
	ReportAbilityEvent(FinishEventTag, Payload);

	Destroy();
}

void AProjectile::ReportAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload)
{
	Payload.Projectile = this;
	if (MyAbility)
	{
		MyAbility->ReportAbilityEvent(EventTag, Payload);
	}
}