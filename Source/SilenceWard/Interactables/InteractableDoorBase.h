#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableDoorBase.generated.h"

class USceneComponent;

/**
 * Native behavior for BP_Door. The Blueprint keeps ownership of its existing
 * components and Blueprint interface implementations.
 */
UCLASS(Blueprintable)
class SILENCEWARD_API AInteractableDoorBase : public AActor
{
	GENERATED_BODY()

public:
	AInteractableDoorBase();

	/** Compatibility entry point for BPI_interact::interact. */
	UFUNCTION(BlueprintCallable, Category = "Door", meta = (DisplayName = "Interact Door (Native)"))
	void InteractDoor();

	/** Compatibility entry point for BPI_Door::Pick Up Key. */
	UFUNCTION(BlueprintCallable, Category = "Door", meta = (DisplayName = "Unlock Door (Native)"))
	void UnlockDoor();

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsDoorUnlocked() const { return bDoorUnlocked; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Name of BP_Door's existing pivot component. No component is recreated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Components")
	FName DoorPivotComponentName = TEXT("Scene");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Animation")
	float ClosedYaw = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Animation")
	float OpenYaw = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Animation", meta = (ClampMin = "0.01", Units = "s"))
	float AnimationDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|State")
	bool bDoorUnlocked = false;

private:
	void ResolveDoorPivot();
	void StartDoorAnimation(bool bOpen);
	void UpdateDoorAnimation();
	void ApplyDoorRotation();

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> DoorPivotComponent;

	FTimerHandle DoorAnimationTimerHandle;
	float DoorAnimationAlpha = 0.0f;
	float TargetDoorAnimationAlpha = 0.0f;
	bool bNextInteractionOpens = true;
};
