#include "FirstPersonPlayerControllerBase.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

AFirstPersonPlayerControllerBase::AFirstPersonPlayerControllerBase()
{
	MouseLookMappingContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")));
}

void AFirstPersonPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(
			this,
			&AFirstPersonPlayerControllerBase::InitializeLocalPlayerInput));
}

void AFirstPersonPlayerControllerBase::InitializeLocalPlayerInput()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (ShouldUseTouchControlsNative())
	{
		ShowTouchControls();
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s has no local player for mouse-look input setup."),
			*GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	UInputMappingContext* MappingContext = MouseLookMappingContext.LoadSynchronous();
	if (!IsValid(InputSubsystem) || !IsValid(MappingContext))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s could not initialize IMC_MouseLook."),
			*GetName());
		return;
	}

	InputSubsystem->AddMappingContext(
		MappingContext,
		MouseLookMappingPriority,
		FModifyContextOptions());
}

bool AFirstPersonPlayerControllerBase::ShouldUseTouchControlsNative() const
{
	const FString PlatformName = UGameplayStatics::GetPlatformName();
	if (PlatformName == TEXT("IOS") || PlatformName == TEXT("Android"))
	{
		return true;
	}

	return bForceTouchControlsOverride;
}
