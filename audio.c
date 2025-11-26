#ifndef AUDIO_C
#define AUDIO_C

#include <SDL3_mixer/SDL_mixer.h>

typedef struct {
    MIX_Mixer *mixer;
    MIX_Audio *paddle_hit;
    MIX_Audio *wall_hit;
    MIX_Audio *score;
    MIX_Audio *music;
    MIX_Track *sfx_track;
    MIX_Track *music_track;
} Audio;

bool audio_init(Audio *audio) {
    if (!MIX_Init()) {
        SDL_Log("Failed to init SDL_mixer: %s", SDL_GetError());
        return false;
    }

    // Use default playback device
    audio->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!audio->mixer) {
        SDL_Log("Failed to create mixer: %s", SDL_GetError());
        return false;
    }

    // Load sound effects (predecode=true for sound effects, false for music)
    audio->paddle_hit = MIX_LoadAudio(audio->mixer, "assets/sounds/paddle_hit.ogg", true);
    audio->wall_hit = MIX_LoadAudio(audio->mixer, "assets/sounds/wall_hit.ogg", true);
    audio->score = MIX_LoadAudio(audio->mixer, "assets/sounds/score.ogg", true);
    audio->music = MIX_LoadAudio(audio->mixer, "assets/sounds/music.ogg", false);

    if (!audio->paddle_hit || !audio->wall_hit || !audio->score) {
        SDL_Log("Failed to load sound effects: %s", SDL_GetError());
        return false;
    }

    // Create tracks for playback
    audio->sfx_track = MIX_CreateTrack(audio->mixer);
    audio->music_track = MIX_CreateTrack(audio->mixer);

    if (!audio->sfx_track || !audio->music_track) {
        SDL_Log("Failed to create tracks: %s", SDL_GetError());
        return false;
    }

    // Start background music if loaded
    if (audio->music) {
        MIX_SetTrackAudio(audio->music_track, audio->music);
        MIX_SetTrackGain(audio->music_track, 0.3f);  // Lower volume for music
        MIX_PlayTrack(audio->music_track, 0);
    }

    return true;
}

void audio_play_paddle_hit(Audio *audio) {
    if (audio->paddle_hit) {
        MIX_SetTrackAudio(audio->sfx_track, audio->paddle_hit);
        MIX_PlayTrack(audio->sfx_track, 0);
    }
}

void audio_play_wall_hit(Audio *audio) {
    if (audio->wall_hit) {
        MIX_SetTrackAudio(audio->sfx_track, audio->wall_hit);
        MIX_PlayTrack(audio->sfx_track, 0);
    }
}

void audio_play_score(Audio *audio) {
    if (audio->score) {
        MIX_SetTrackAudio(audio->sfx_track, audio->score);
        MIX_PlayTrack(audio->sfx_track, 0);
    }
}

void audio_quit(Audio *audio) {
    if (audio->paddle_hit) MIX_DestroyAudio(audio->paddle_hit);
    if (audio->wall_hit) MIX_DestroyAudio(audio->wall_hit);
    if (audio->score) MIX_DestroyAudio(audio->score);
    if (audio->music) MIX_DestroyAudio(audio->music);
    if (audio->mixer) MIX_DestroyMixer(audio->mixer);
    MIX_Quit();
}

#endif
