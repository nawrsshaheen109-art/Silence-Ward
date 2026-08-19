#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class SILENCEWARD_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/** Native gameplay contract for actors that can be interacted with. */
class SILENCEWARD_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Performs this actor's interaction. The distinct reflected name allows the
	 * legacy BPI_interact::interact event to coexist during migration.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction", meta = (DisplayName = "Perform Interaction (Native)"))
	void PerformInteraction();
};
