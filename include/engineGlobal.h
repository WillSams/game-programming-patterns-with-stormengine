#pragma once

// ── The engine's names, in the global namespace — this repo's own bridge ─────
//
// Storm! Engine v2.0.0 moved every engine type into `namespace storm`. The
// patterns were written against engine 1.x and name them unqualified --
// `class PlayState : public GameState` -- so against any 2.x engine they do not
// compile at all, which is what building this repo found:
//
//     src/states/playState.h:18: error: expected class-name before '{' token
//     src/states/playState.h:34: error: 'AssetStore_Ptr' does not name a type
//
// There are ten patterns plus a template, each a self-contained folder with no
// shared translation unit to put a `using` in, so the migration is a
// force-include from the build:
//
//     CCFLAGS += -include engineGlobal.h
//
// **THIS IS DELIBERATELY NOT `stormengine2/compat/global.h`.** The engine ships
// its own bridge and it is the wrong tool here, for the reason the sibling
// project (center-ice-hockey) recorded: the engine's shim includes the WHOLE
// engine by design, because "a compatibility shim cannot know which parts a game
// uses". That drags ecs.h, tilemapLoader.h, xmlLoader.h and every net and
// component header into all ~113 files, so a demo that needs four names pays for
// the whole engine -- and xmlLoader.h brings tinyxml2 with it.
//
// So this header names ONLY the four engine headers the patterns include and the
// seven names they use.
//
// THE LIST IS DERIVED, NOT REMEMBERED -- `Entity` is the trap:
//
//     # the four headers
//     grep -rhoE '#include <stormengine2/[^>]+>' patterns/ | sort | uniq -c
//     # the names, each checked against the engine rather than assumed
//     for n in MILLISECS_PER_FRAME LogType SpriteComponent Entity ...; do
//       grep -rl "\b$n\b" patterns/ --include='*.h' --include='*.cpp'; done
//
// `Entity` appears in FOUR files and is NOT here: `update_method` defines its own
// `class Entity` under `src/entities/`, which is the point of that pattern. A
// bridge that exported the engine's `Entity` would collide with it.
//
// **This header exists to be deleted.** The migration ends by qualifying the
// names or adding `using namespace storm;` per file, and then removing the
// force-include from common.mk.

#include <stormengine2/assetStore.h>
#include <stormengine2/gameStateMachine.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

using storm::AssetStore;
using storm::AssetStore_Ptr;
using storm::GameState;
using storm::GameStateMachine;
using storm::Logger;
using storm::Logger_Ptr;
using storm::MILLISECS_PER_FRAME;   // the frame-pacing constant, used in 10 files
