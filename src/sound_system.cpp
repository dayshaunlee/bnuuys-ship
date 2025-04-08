// internal
#include "sound_system.hpp"

#include <iostream>
#include "tinyECS/registry.hpp"
#include "world_init.hpp"

bool SoundSystem::init() {
    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }
    for (uint i = 0; i < sound_paths_repeating.size(); i++) {
        Mix_Music* music = Mix_LoadMUS(sound_paths_repeating[i].c_str());
        if (music == nullptr) {
            fprintf(stderr,
                    "Failed to load sound\n %s make sure the data "
                    "directory is present",
                    sound_paths_repeating[i].c_str());
            return false;
        }
        sound_mix_repeating[i] = music;
    }
    for (uint i = 0; i < sound_paths_chunk.size(); i++) {
        Mix_Chunk* sound = Mix_LoadWAV(sound_paths_chunk[i].c_str());
        if (sound == nullptr) {
            fprintf(stderr,
                    "Failed to load sound\n %s make sure the data "
                    "directory is present",
                    sound_paths_chunk[i].c_str());
            return false;
        }
        sound_mix_chunk[i] = sound;
    }

    Mix_Volume(-1, 128);

    return true;
}

SoundSystem::~SoundSystem() {
    for (uint i = 0; i < sound_mix_repeating.size(); i++) {
        if (sound_mix_repeating[i] != nullptr) Mix_FreeMusic(sound_mix_repeating[i]);
    }
    for (uint i = 0; i < sound_paths_chunk.size(); i++) {
        if (sound_mix_chunk[i] != nullptr) Mix_FreeChunk(sound_mix_chunk[i]);
    }
    Mix_CloseAudio();
}

void SoundSystem::play() {
    for (Entity entity : registry.sounds.entities) {
        Sound& sound = registry.sounds.get(entity);
        if (sound.is_repeating) {
            Mix_VolumeMusic(sound.volume);
            Mix_PlayMusic(sound_mix_repeating[(int) sound.sound_type], -1);
        } else {
            Mix_VolumeChunk(sound_mix_chunk[(int) sound.sound_type - sound_paths_repeating.size()], sound.volume);
            Mix_PlayChannel(
                (int) sound.sound_type % 8, sound_mix_chunk[(int) sound.sound_type - sound_paths_repeating.size()], 0);
        }
        registry.remove_all_components_of(entity);
    }
}
