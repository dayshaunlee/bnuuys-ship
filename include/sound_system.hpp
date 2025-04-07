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
    const std::array<std::string, 15> sound_paths_chunk = {
        audio_path("island-ship_collision.wav"),
        audio_path("ship-enemy_collision.wav"),
        audio_path("projectile-enemy_collision.wav"),
        audio_path("projectile-jail_collision.wav"),
        audio_path("game_over.wav"),
        audio_path("BnuuyIntro.wav"),
        audio_path("book_open.wav"),
        audio_path("bubble.wav"),
        audio_path("click.wav"),
        audio_path("laser.wav"),
        audio_path("object_placement.wav"),
        audio_path("raft.wav"),
        audio_path("cow_bullet.wav"),
        audio_path("projectile_shoot.wav"),
        audio_path("end_music.wav"),
    };

    std::array<Mix_Music*, 2> sound_mix_repeating;
    std::array<Mix_Chunk*, 15> sound_mix_chunk;
   public:

    bool init();
    void play();

    ~SoundSystem();
};
