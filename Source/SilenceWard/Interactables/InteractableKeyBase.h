#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableKeyBase.generated.h"

class AInteractableDoorBase;

/**
 * Native pickup behavior for BP_Key. The Blueprint keeps ownership of its
 * existing components, prompt logic, and BPI_interact implementation.
 */
UCLASS(Blueprintable)
class SILENCEWARD_API AInteractableKeyBase : public AActor
{
	GENERATED_BODY()

public:
	AInteractableKeyBase();

	/** Unlocks the assigned native door base, then consumes this key actor. */
	UFUNCTION(BlueprintCallable, Category = "Key", meta = (DisplayName = "Collect Key (Native)"))
	void CollectKey(AInteractableDoorBase* DoorToUnlock);
};
