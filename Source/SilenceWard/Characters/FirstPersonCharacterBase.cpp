#include "FirstPersonCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "../Audio/ConcreteFootstepSynth.h"
#include "../Interaction/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Interface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	const FName InteractFunctionName(TEXT("interact"));
}

AFirstPersonCharacterBase::AFirstPersonCharacterBase()
{
	InteractionInterface = TSoftClassPtr<UInterface>(FSoftObjectPath(
		TEXT("/Game/Blueprints/Interactables/BPI_interact.BPI_interact_C")));
	DefaultMappingContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(
		TEXT("/Game/Input/IMC_Default.IMC_Default")));
	MoveAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move")));
	LookAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look")));
	MouseLookAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")));
	JumpAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump")));
	InteractAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_Interact.IA_Interact")));
	SprintAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(
		TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint")));
}

void AFirstPersonCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SilenceWard::ConcreteFootsteps::Begin(*this);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = WalkSpeed;
	}

	const FPlatformUserId PlatformUserId =
		FPlatformUserId::CreateFromInternalId(PlatformUserInternalId);
	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerControllerFromPlatformUser(this, PlatformUserId);
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s could not find a player controller for platform user %d."),
			*GetName(), PlatformUserInternalId);
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s has no local player for Enhanced Input setup."),
			*GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	UInputMappingContext* MappingContext = DefaultMappingContext.LoadSynchronous();
	if (!IsValid(InputSubsystem) || !IsValid(MappingContext))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s could not initialize IMC_Default for Enhanced Input."),
			*GetName());
		return;
	}

	InputSubsystem->AddMappingContext(
		MappingContext,
		MappingContextPriority,
		FModifyContextOptions());
}

void AFirstPersonCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SilenceWard::ConcreteFootsteps::End(*this);
	Super::EndPlay(EndPlayReason);
}

void AFirstPersonCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s requires an Enhanced Input component."),
			*GetName());
		return;
	}

	if (const UInputAction* LoadedMoveAction = MoveAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedMoveAction, ETriggerEvent::Triggered,
			this, &AFirstPersonCharacterBase::HandleMoveInput);
	}

	if (const UInputAction* LoadedLookAction = LookAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedLookAction, ETriggerEvent::Triggered,
			this, &AFirstPersonCharacterBase::HandleLookInput);
	}

	if (const UInputAction* LoadedMouseLookAction = MouseLookAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedMouseLookAction, ETriggerEvent::Triggered,
			this, &AFirstPersonCharacterBase::HandleLookInput);
	}

	if (const UInputAction* LoadedJumpAction = JumpAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedJumpAction, ETriggerEvent::Started,
			this, &AFirstPersonCharacterBase::HandleJumpStarted);
		EnhancedInputComponent->BindAction(
			LoadedJumpAction, ETriggerEvent::Completed,
			this, &AFirstPersonCharacterBase::HandleJumpCompleted);
	}

	if (const UInputAction* LoadedInteractAction = InteractAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedInteractAction, ETriggerEvent::Started,
			this, &AFirstPersonCharacterBase::HandleInteractStarted);
	}

	if (const UInputAction* LoadedSprintAction = SprintAction.LoadSynchronous())
	{
		EnhancedInputComponent->BindAction(
			LoadedSprintAction, ETriggerEvent::Triggered,
			this, &AFirstPersonCharacterBase::HandleSprintStarted);
		EnhancedInputComponent->BindAction(
			LoadedSprintAction, ETriggerEvent::Completed,
			this, &AFirstPersonCharacterBase::HandleSprintCompleted);
	}
}

void AFirstPersonCharacterBase::MoveCharacterNative(
	const float LeftRight,
	const float ForwardBackward)
{
	AddMovementInput(GetActorRightVector(), LeftRight);
	AddMovementInput(GetActorForwardVector(), ForwardBackward);
}

void AFirstPersonCharacterBase::AimNative(const float Yaw, const float Pitch)
{
	AddControllerYawInput(Yaw);
	AddControllerPitchInput(Pitch);
}

void AFirstPersonCharacterBase::HandleMoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementAxis = Value.Get<FVector2D>();
	MoveCharacterNative(MovementAxis.X, MovementAxis.Y);
}

void AFirstPersonCharacterBase::HandleLookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AimNative(LookAxis.X, LookAxis.Y);
}

void AFirstPersonCharacterBase::HandleJumpStarted(const FInputActionValue& Value)
{
	Jump();
}

void AFirstPersonCharacterBase::HandleJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}

void AFirstPersonCharacterBase::HandleInteractStarted(const FInputActionValue& Value)
{
	InteractWithOverlappingActor();
}

void AFirstPersonCharacterBase::HandleSprintStarted(const FInputActionValue& Value)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = SprintSpeed;
	}
}

void AFirstPersonCharacterBase::HandleSprintCompleted(const FInputActionValue& Value)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = WalkSpeed;
	}
}

void AFirstPersonCharacterBase::InteractWithOverlappingActor()
{
	UClass* InterfaceClass = InteractionInterface.LoadSynchronous();
	if (!IsValid(InterfaceClass) || !InterfaceClass->IsChildOf(UInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s could not load a valid interaction interface."),
			*GetName());
		return;
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!IsValid(OverlappingActor))
		{
			continue;
		}

		if (OverlappingActor->GetClass()->ImplementsInterface(
			UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_PerformInteraction(OverlappingActor);
			break;
		}

		if (!OverlappingActor->GetClass()->ImplementsInterface(InterfaceClass))
		{
			continue;
		}

		if (UFunction* InteractFunction = OverlappingActor->FindFunction(InteractFunctionName))
		{
			OverlappingActor->ProcessEvent(InteractFunction, nullptr);
		}

		// The existing Blueprint uses ForEachLoopWithBreak and stops after the
		// first actor that passes the interface check.
		break;
	}
}
