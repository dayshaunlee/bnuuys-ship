#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include <array>

// System for loading and playing sounds
class SoundSystem {
    const std::array<std::string, 2> sound_paths_repeating = {
        audio_path("music.wav"),
        audio_path("music.wav"),
    };
    const std::array<std::string, 6> sound_paths_chunk = {
        audio_path("island-ship_collision.wav"),
        audio_path("ship-enemy_collision.wav"),
        audio_path("projectile-enemy_collision.wav"),
        audio_path("projectile-jail_collision.wav"),
        audio_path("game_over.wav"),
        audio_path("BnuuyIntro.wav"),
    };

    std::array<Mix_Music*, 2> sound_mix_repeating;
    std::array<Mix_Chunk*, 6> sound_mix_chunk;
   public:

    bool init();
    void play();

    ~SoundSystem();
};
