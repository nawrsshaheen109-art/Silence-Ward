#pragma once

#include "CoreMinimal.h"

class AFirstPersonCharacterBase;

/**
 * Runtime footstep playback using the project's imported old-wood recordings.
 *
 * Kept non-reflected so it can be compiled into an open editor without
 * restarting or saving the current level. Runtime state is keyed by character
 * and advanced by a world timer registered at BeginPlay.
 */
namespace SilenceWard::ConcreteFootsteps
{
	void Begin(AFirstPersonCharacterBase& Character);
	void End(AFirstPersonCharacterBase& Character);
}
