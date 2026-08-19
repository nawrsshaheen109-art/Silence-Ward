#include "InteractableDoorBase.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AInteractableDoorBase::AInteractableDoorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableDoorBase::PerformInteraction_Implementation()
{
	InteractDoor();
}

void AInteractableDoorBase::BeginPlay()
{
	Super::BeginPlay();
	ResolveDoorPivot();
}

void AInteractableDoorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DoorAnimationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AInteractableDoorBase::InteractDoor()
{
	if (!bDoorUnlocked)
	{
		return;
	}

	StartDoorAnimation(bNextInteractionOpens);
	bNextInteractionOpens = !bNextInteractionOpens;
}

void AInteractableDoorBase::UnlockDoor()
{
	bDoorUnlocked = true;
}

void AInteractableDoorBase::ResolveDoorPivot()
{
	DoorPivotComponent = nullptr;

	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (IsValid(SceneComponent) && SceneComponent->GetFName() == DoorPivotComponentName)
		{
			DoorPivotComponent = SceneComponent;
			return;
		}
	}

	UE_LOG(LogTemp, Warning,
		TEXT("%s could not find its BP_Door pivot component named '%s'."),
		*GetName(), *DoorPivotComponentName.ToString());
}

void AInteractableDoorBase::StartDoorAnimation(const bool bOpen)
{
	if (!IsValid(DoorPivotComponent))
	{
		ResolveDoorPivot();
	}

	if (!IsValid(DoorPivotComponent))
	{
		return;
	}

	TargetDoorAnimationAlpha = bOpen ? 1.0f : 0.0f;

	if (FMath::IsNearlyEqual(DoorAnimationAlpha, TargetDoorAnimationAlpha))
	{
		ApplyDoorRotation();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DoorAnimationTimerHandle,
			this,
			&AInteractableDoorBase::UpdateDoorAnimation,
			1.0f / 60.0f,
			true);
	}
}

void AInteractableDoorBase::UpdateDoorAnimation()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(DoorPivotComponent))
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(DoorAnimationTimerHandle);
		}
		return;
	}

	const float SafeDuration = FMath::Max(AnimationDuration, UE_SMALL_NUMBER);
	const float Direction = TargetDoorAnimationAlpha > DoorAnimationAlpha ? 1.0f : -1.0f;
	DoorAnimationAlpha = FMath::Clamp(
		DoorAnimationAlpha + Direction * (World->GetDeltaSeconds() / SafeDuration),
		0.0f,
		1.0f);

	ApplyDoorRotation();

	if (FMath::IsNearlyEqual(DoorAnimationAlpha, TargetDoorAnimationAlpha, KINDA_SMALL_NUMBER))
	{
		DoorAnimationAlpha = TargetDoorAnimationAlpha;
		ApplyDoorRotation();
		World->GetTimerManager().ClearTimer(DoorAnimationTimerHandle);
	}
}

void AInteractableDoorBase::ApplyDoorRotation()
{
	if (!IsValid(DoorPivotComponent))
	{
		return;
	}

	const float DoorYaw = FMath::Lerp(ClosedYaw, OpenYaw, DoorAnimationAlpha);
	DoorPivotComponent->SetRelativeRotation(FRotator(0.0f, DoorYaw, 0.0f));
}
