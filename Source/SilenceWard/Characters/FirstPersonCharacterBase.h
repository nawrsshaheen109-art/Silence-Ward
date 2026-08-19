#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FirstPersonCharacterBase.generated.h"

class UInterface;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Native interaction selection for BP_FirstPersonCharacter. The Blueprint
 * keeps ownership of input binding and all other character behavior.
 */
UCLASS(Blueprintable)
class SILENCEWARD_API AFirstPersonCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AFirstPersonCharacterBase();

	/** Invokes the first overlapping native or legacy interactable. */
	UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (DisplayName = "Interact With Overlapping Actor (Native)"))
	void InteractWithOverlappingActor();

	/** Shared movement implementation for Enhanced Input and mobile touch. */
	UFUNCTION(BlueprintCallable, Category = "Input|Movement", meta = (DisplayName = "Move Character (Native)"))
	void MoveCharacterNative(float LeftRight, float ForwardBackward);

	/** Shared aim implementation for Enhanced Input and mobile touch. */
	UFUNCTION(BlueprintCallable, Category = "Input|Look", meta = (DisplayName = "Aim (Native)"))
	void AimNative(float Yaw, float Pitch);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Blueprint-only interface used by the project's existing interactables. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	TSoftClassPtr<UInterface> InteractionInterface;

	/** Mapping context registered by the original Blueprint BeginPlay graph. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	int32 MappingContextPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	int32 PlatformUserInternalId = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input|Actions")
	TSoftObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint", meta = (ClampMin = "0.0", Units = "cm/s"))
	float WalkSpeed = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint", meta = (ClampMin = "0.0", Units = "cm/s"))
	float SprintSpeed = 450.0f;

private:
	void HandleMoveInput(const FInputActionValue& Value);
	void HandleLookInput(const FInputActionValue& Value);
	void HandleJumpStarted(const FInputActionValue& Value);
	void HandleJumpCompleted(const FInputActionValue& Value);
	void HandleInteractStarted(const FInputActionValue& Value);
	void HandleSprintStarted(const FInputActionValue& Value);
	void HandleSprintCompleted(const FInputActionValue& Value);
};
