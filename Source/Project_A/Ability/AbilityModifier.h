#pragma once
#include "CoreMinimal.h"
#include "Ability.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityModifier.generated.h"

// ============================================================================
// AAbilityModifier
// An Ability that acts on another Ability instead of existing in the world - a
// Modify ability that sends a command to the held ability (e.g. telling a
// Firewall to push forward). Spawned at the caster, runs OnActivate, and
// finishes right after. No ticking, collision or spawn placement. Anything
// that needs to last (e.g. "push, then pull back after 2 seconds") belongs on
// the modified Ability, which the modifier only tells what to do.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API AAbilityModifier : public AAbility, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AAbilityModifier();

protected:
	// Moves to the caster, reports Cast, runs OnActivate (read ModifiedAbility there), then finishes.
	virtual void BeginPlay() override;

public:
	// Notifies (reports FinishEventTags, broadcasts OnFinish), fires OnEnd, then destroys - same
	// ordering as AAbilityActor::Finish().
	virtual void Finish() override;
};
