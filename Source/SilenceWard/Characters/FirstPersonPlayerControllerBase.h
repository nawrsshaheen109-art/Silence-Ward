#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstPersonPlayerControllerBase.generated.h"

class UInputMappingContext;

/**
 * Native control flow for BP_FirstPersonPlayerController. The Blueprint child
 * retains ownership of touch-widget creation and visual configuration.
 */
UCLASS(Blueprintable)
class SILENCEWARD_API AFirstPersonPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AFirstPersonPlayerControllerBase();

protected:
	virtual void BeginPlay() override;

	/** Called only when this local controller should display mobile controls. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Touch Controls")
	void ShowTouchControls();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 MouseLookMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Touch Controls")
	bool bForceTouchControlsOverride = false;

private:
	void InitializeLocalPlayerInput();
	bool ShouldUseTouchControlsNative() const;
};
