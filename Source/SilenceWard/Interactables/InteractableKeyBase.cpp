#include "InteractableKeyBase.h"

#include "InteractableDoorBase.h"

AInteractableKeyBase::AInteractableKeyBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableKeyBase::CollectKey(AInteractableDoorBase* DoorToUnlock)
{
	if (IsValid(DoorToUnlock))
	{
		DoorToUnlock->UnlockDoor();
	}

	Destroy();
}
