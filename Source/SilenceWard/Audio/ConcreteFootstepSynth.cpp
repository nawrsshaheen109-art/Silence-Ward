#include "ConcreteFootstepSynth.h"

#include "../Characters/FirstPersonCharacterBase.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "UObject/StrongObjectPtrTemplates.h"

DEFINE_LOG_CATEGORY_STATIC(LogConcreteFootsteps, Log, All);

namespace SilenceWard::ConcreteFootsteps
{
	constexpr float WalkStepDistance = 94.0f;
	constexpr float SprintStepDistance = 150.0f;
	constexpr float WalkInitialStepDistance = 60.0f;
	constexpr float SprintInitialStepDistance = 115.0f;
	constexpr float WalkCadenceSpeed = 250.0f;
	constexpr float SprintCadenceSpeed = 450.0f;
	constexpr float MinimumAudibleSpeed = 22.0f;
	constexpr float SprintSpeedThreshold = 350.0f;
	constexpr float WalkVolume = 0.72f;
	constexpr float SprintVolume = 0.82f;
	const TCHAR* FootstepRecordingPaths[] =
	{
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_01.FS_Wood_Single_01"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_02.FS_Wood_Single_02"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_03.FS_Wood_Single_03"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_04.FS_Wood_Single_04"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_05.FS_Wood_Single_05"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_06.FS_Wood_Single_06"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_07.FS_Wood_Single_07"),
		TEXT("/Game/Audio/Footsteps/SingleImpact/FS_Wood_Single_08.FS_Wood_Single_08")
	};

	enum class ESurfaceProfile : uint8
	{
		OldWood
	};

	struct FSurfaceRoute
	{
		EPhysicalSurface PhysicalSurface;
		ESurfaceProfile Profile;
	};

	// Add future PhysicalSurface mappings here. Unmapped surfaces currently
	// use the project's old-wood fallback recording set.
	constexpr FSurfaceRoute SurfaceRoutes[] =
	{
		{SurfaceType_Default, ESurfaceProfile::OldWood}
	};

	struct FRuntimeState
	{
		FVector PreviousLocation = FVector::ZeroVector;
		float DistanceUntilNextStep = 0.0f;
		float PreviousStepDistance = 0.0f;
		bool bWasMovingGrounded = false;
		int32 StepCounter = 0;
		int32 LastSoundIndex = INDEX_NONE;
		TArray<TStrongObjectPtr<USoundWave>> FootstepRecordings;
		TArray<TWeakObjectPtr<UAudioComponent>> ActiveAudio;
		FTimerHandle UpdateTimer;
	};

	TMap<TWeakObjectPtr<AFirstPersonCharacterBase>, FRuntimeState> RuntimeStates;
	void Update(AFirstPersonCharacterBase& Character, float DeltaTime);

	void LoadFootstepRecordings(FRuntimeState& State)
	{
		State.FootstepRecordings.Reset();
		State.FootstepRecordings.Reserve(UE_ARRAY_COUNT(FootstepRecordingPaths));

		for (const TCHAR* RecordingPath : FootstepRecordingPaths)
		{
			USoundWave* Recording = LoadObject<USoundWave>(nullptr, RecordingPath);
			if (!IsValid(Recording))
			{
				UE_LOG(
					LogConcreteFootsteps,
					Error,
					TEXT("Could not load footstep recording %s."),
					RecordingPath);
				continue;
			}

			State.FootstepRecordings.Emplace(Recording);
		}
	}

	float GetGaitAlpha(const float HorizontalSpeed)
	{
		return FMath::Clamp(
			(HorizontalSpeed - WalkCadenceSpeed)
				/ (SprintCadenceSpeed - WalkCadenceSpeed),
			0.0f,
			1.0f);
	}

	float GetStepDistance(const float HorizontalSpeed)
	{
		return FMath::Lerp(
			WalkStepDistance,
			SprintStepDistance,
			GetGaitAlpha(HorizontalSpeed));
	}

	float GetInitialStepDistance(const float HorizontalSpeed)
	{
		return FMath::Lerp(
			WalkInitialStepDistance,
			SprintInitialStepDistance,
			GetGaitAlpha(HorizontalSpeed));
	}

	ESurfaceProfile ResolveSurface(
		AFirstPersonCharacterBase& Character,
		FVector& OutSoundLocation)
	{
		const float CapsuleHalfHeight = IsValid(Character.GetCapsuleComponent())
			? Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 88.0f;
		const FVector Start = Character.GetActorLocation()
			- FVector(0.0f, 0.0f, CapsuleHalfHeight - 12.0f);
		const FVector EndLocation = Start - FVector(0.0f, 0.0f, 55.0f);

		FCollisionQueryParams QueryParams(
			SCENE_QUERY_STAT(RecordedFootstepSurface),
			false,
			&Character);
		QueryParams.bReturnPhysicalMaterial = true;

		FHitResult Hit;
		EPhysicalSurface PhysicalSurface = SurfaceType_Default;
		if (Character.GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			EndLocation,
			ECC_Visibility,
			QueryParams))
		{
			OutSoundLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, 4.0f);
			PhysicalSurface = UPhysicalMaterial::DetermineSurfaceType(
				Hit.PhysMaterial.Get());
		}

		for (const FSurfaceRoute& Route : SurfaceRoutes)
		{
			if (Route.PhysicalSurface == PhysicalSurface)
			{
				return Route.Profile;
			}
		}
		return ESurfaceProfile::OldWood;
	}

	int32 SelectRecordingIndex(
		const FRuntimeState& State,
		FRandomStream& Random)
	{
		const int32 RecordingCount = State.FootstepRecordings.Num();
		if (RecordingCount <= 1 || State.LastSoundIndex == INDEX_NONE)
		{
			return RecordingCount > 0 ? Random.RandRange(0, RecordingCount - 1) : INDEX_NONE;
		}

		// Select uniformly from every recording except the immediately previous one.
		int32 SelectedIndex = Random.RandRange(0, RecordingCount - 2);
		if (SelectedIndex >= State.LastSoundIndex)
		{
			++SelectedIndex;
		}
		return SelectedIndex;
	}

	void TriggerStep(
		AFirstPersonCharacterBase& Character,
		FRuntimeState& State,
		const bool bSprinting,
		const float HorizontalSpeed)
	{
		FVector SoundLocation = Character.GetActorLocation();
		if (ResolveSurface(Character, SoundLocation) != ESurfaceProfile::OldWood
			|| State.FootstepRecordings.IsEmpty())
		{
			return;
		}

		FRandomStream Random(
			GetTypeHash(&Character)
			^ static_cast<uint32>((State.StepCounter + 1) * 196613));
		const int32 RecordingIndex = SelectRecordingIndex(State, Random);
		if (!State.FootstepRecordings.IsValidIndex(RecordingIndex))
		{
			return;
		}

		USoundWave* Recording = State.FootstepRecordings[RecordingIndex].Get();
		if (!IsValid(Recording))
		{
			return;
		}

		State.LastSoundIndex = RecordingIndex;
		++State.StepCounter;

		const bool bLeftFoot = (State.StepCounter & 1) != 0;
		const float FootPitchOffset = bLeftFoot ? -0.002f : 0.002f;
		const float Pitch = FMath::Clamp(
			Random.FRandRange(0.982f, 1.018f) + FootPitchOffset,
			0.98f,
			1.02f);
		const float Volume = (bSprinting ? SprintVolume : WalkVolume)
			* Random.FRandRange(0.95f, 1.05f);

		UAudioComponent* AudioComponent = NewObject<UAudioComponent>(&Character);
		AudioComponent->bAutoActivate = false;
		AudioComponent->bAutoDestroy = true;
		AudioComponent->bStopWhenOwnerDestroyed = true;
		AudioComponent->bAllowSpatialization = false;
		AudioComponent->RegisterComponentWithWorld(Character.GetWorld());
		AudioComponent->SetWorldLocation(SoundLocation);
		AudioComponent->SetSound(Recording);
		AudioComponent->SetVolumeMultiplier(Volume);
		AudioComponent->SetPitchMultiplier(Pitch);
		AudioComponent->Play();
		State.ActiveAudio.Add(AudioComponent);

		UE_LOG(
			LogConcreteFootsteps,
			Log,
			TEXT("STEP %d foot=%s recording=FS_Wood_%02d gait=%s speed=%.1f pitch=%.3f volume=%.3f surface=OldWood"),
			State.StepCounter,
			bLeftFoot ? TEXT("Left") : TEXT("Right"),
			RecordingIndex + 1,
			bSprinting ? TEXT("Sprint") : TEXT("Walk"),
			HorizontalSpeed,
			Pitch,
			Volume);
	}

	void Begin(AFirstPersonCharacterBase& Character)
	{
		for (auto Iterator = RuntimeStates.CreateIterator(); Iterator; ++Iterator)
		{
			if (!Iterator.Key().IsValid())
			{
				Iterator.RemoveCurrent();
			}
		}

		FRuntimeState& State = RuntimeStates.FindOrAdd(&Character);
		State = FRuntimeState();
		State.PreviousLocation = Character.GetActorLocation();
		LoadFootstepRecordings(State);
		const TWeakObjectPtr<AFirstPersonCharacterBase> WeakCharacter(&Character);
		FTimerDelegate UpdateDelegate;
		UpdateDelegate.BindLambda([WeakCharacter]()
		{
			if (AFirstPersonCharacterBase* LiveCharacter = WeakCharacter.Get())
			{
				const UWorld* World = LiveCharacter->GetWorld();
				Update(*LiveCharacter, IsValid(World) ? World->GetDeltaSeconds() : 0.01f);
			}
		});
		Character.GetWorldTimerManager().SetTimer(
			State.UpdateTimer,
			UpdateDelegate,
			0.01f,
			true);

		UE_LOG(
			LogConcreteFootsteps,
			Log,
			TEXT("Recorded old-wood footsteps ready on %s: %d/8 SoundWaves loaded (walk %.0f cm, sprint %.0f cm)."),
			*Character.GetName(),
			State.FootstepRecordings.Num(),
			WalkStepDistance,
			SprintStepDistance);
	}

	void End(AFirstPersonCharacterBase& Character)
	{
		FRuntimeState* State = RuntimeStates.Find(&Character);
		if (State == nullptr)
		{
			return;
		}

		Character.GetWorldTimerManager().ClearTimer(State->UpdateTimer);

		for (const TWeakObjectPtr<UAudioComponent>& AudioComponent : State->ActiveAudio)
		{
			if (AudioComponent.IsValid())
			{
				AudioComponent->Stop();
			}
		}
		State->ActiveAudio.Reset();

		RuntimeStates.Remove(&Character);
	}

	void Update(AFirstPersonCharacterBase& Character, const float DeltaTime)
	{
		FRuntimeState* State = RuntimeStates.Find(&Character);
		if (State == nullptr)
		{
			return;
		}

		State->ActiveAudio.RemoveAllSwap(
			[](const TWeakObjectPtr<UAudioComponent>& AudioComponent)
			{
				return !AudioComponent.IsValid() || !AudioComponent->IsPlaying();
			},
			EAllowShrinking::No);

		UCharacterMovementComponent* Movement = Character.GetCharacterMovement();
		if (!IsValid(Movement))
		{
			return;
		}
		const FVector CurrentLocation = Character.GetActorLocation();
		const float TravelDistance = FVector::Dist2D(
			CurrentLocation,
			State->PreviousLocation);
		State->PreviousLocation = CurrentLocation;
		const float HorizontalSpeed = Movement->Velocity.Size2D();
		const bool bHasMovementIntent =
			!Character.GetLastMovementInputVector().IsNearlyZero()
			|| !Movement->GetCurrentAcceleration().IsNearlyZero();
		const bool bMovingGrounded = Movement->IsMovingOnGround()
			&& !Movement->IsFalling()
			&& bHasMovementIntent
			&& HorizontalSpeed >= MinimumAudibleSpeed;

		if (!bMovingGrounded)
		{
			State->bWasMovingGrounded = false;
			State->DistanceUntilNextStep = 0.0f;
			State->PreviousStepDistance = 0.0f;
			return;
		}

		const bool bSprinting = HorizontalSpeed >= SprintSpeedThreshold;
		const float StepDistance = GetStepDistance(HorizontalSpeed);
		if (!State->bWasMovingGrounded)
		{
			State->bWasMovingGrounded = true;
			State->DistanceUntilNextStep = GetInitialStepDistance(HorizontalSpeed);
			State->PreviousStepDistance = StepDistance;
			return;
		}

		if (State->PreviousStepDistance > 0.0f)
		{
			State->DistanceUntilNextStep *= StepDistance / State->PreviousStepDistance;
		}
		State->PreviousStepDistance = StepDistance;

		const float PlausibleTravel = FMath::Min(
			TravelDistance,
			HorizontalSpeed * FMath::Max(DeltaTime, 0.0f) * 1.75f + 2.0f);
		State->DistanceUntilNextStep -= PlausibleTravel;
		if (State->DistanceUntilNextStep <= 0.0f)
		{
			TriggerStep(Character, *State, bSprinting, HorizontalSpeed);
			State->DistanceUntilNextStep += StepDistance;
			if (State->DistanceUntilNextStep <= 0.0f)
			{
				State->DistanceUntilNextStep = StepDistance;
			}
		}
	}
}
