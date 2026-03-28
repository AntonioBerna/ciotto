#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#define CHIP8_IMPLEMENTATION
#include "chip8.h"

// Default settings
#define FPS 60
#define MILLISECS_PER_FRAME (1000 / FPS)
#define INSTRUCTIONS_PER_FRAME 15

#define MODIFIER 15
#define SCREEN_WIDTH ((MODIFIER) * (CHIP8_WIDTH))
#define SCREEN_HEIGHT ((MODIFIER) * (CHIP8_HEIGHT))

#define AMPLITUDE 3000
#define FREQUENCY 440

void SDL_init(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR_ARGS("SDL Could not initialize! SDL_Error: %s", SDL_GetError());
        exit(1);
    }

    *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!(*window)) {
        LOG_ERROR_ARGS("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!(*renderer)) {
        LOG_ERROR_ARGS("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        exit(1);
    }
}

void draw_screen(chip8_t *chip8, SDL_Renderer *renderer) {
    SDL_RenderClear(renderer);

    for (int y = 0; y < CHIP8_HEIGHT; y++) {
        for (int x = 0; x < CHIP8_WIDTH; x++) {
            if (chip8->gfx[(y * CHIP8_WIDTH) + x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            
            SDL_Rect rect = {
                .x = x * MODIFIER,
                .y = y * MODIFIER,
                .w = MODIFIER,
                .h = MODIFIER
            };      
            SDL_RenderFillRect(renderer, &rect);      
        } 
    }

    SDL_RenderPresent(renderer);
}

void audio_callback(void *userdata, uint8_t *stream, int len) {
    chip8_t *chip8 = (chip8_t *)userdata;

    int16_t *buffer = (int16_t *)stream;
    int length = len / 2;

    static uint32_t sample_index = 0;
    for (int i = 0; i < length; i++) {
        if (chip8->sound_timer == 0) {
            buffer[i] = 0;
            sample_index = 0;
        } else { // Play sound
            // Here we consider the total samples in one cycle and we divide
            // it by two, since in a square wave is "high" for half the time
            // and "low" for half the time.
            //
            // (sample frequency / pitch frequency) * (1/2) = samples per cycle / (frequency * 2)
            int half_period = 44100 / (FREQUENCY * 2);

            // Depending on the position of the index we compute the
            // appropriate value. If we are in the "high" half_period we
            // maximize the value, and if we are in the "low"
            // half_period we minimize the value.
            buffer[i] = ((sample_index++ / half_period) % 2) ? AMPLITUDE : -AMPLITUDE;
        }
    }
}

static chip8_key_t keycode_to_chip8_key(SDL_Keycode keycode) {
    switch (keycode) {
        case SDLK_1: return CHIP8_KEY_1;
        case SDLK_2: return CHIP8_KEY_2;
        case SDLK_3: return CHIP8_KEY_3;
        case SDLK_4: return CHIP8_KEY_4;
        case SDLK_q: return CHIP8_KEY_Q;
        case SDLK_w: return CHIP8_KEY_W;
        case SDLK_e: return CHIP8_KEY_E;
        case SDLK_r: return CHIP8_KEY_R;
        case SDLK_a: return CHIP8_KEY_A;
        case SDLK_s: return CHIP8_KEY_S;
        case SDLK_d: return CHIP8_KEY_D;
        case SDLK_f: return CHIP8_KEY_F;
        case SDLK_z: return CHIP8_KEY_Z;
        case SDLK_x: return CHIP8_KEY_X;
        case SDLK_c: return CHIP8_KEY_C;
        case SDLK_v: return CHIP8_KEY_V;
        default: return CHIP8_KEY_UNDEFINED;
    }
}

void handle_input(chip8_t *chip8) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT: {
                chip8->running = 0;      
            } break;

            case SDL_KEYDOWN: {
                chip8_key_t key_index = keycode_to_chip8_key(event.key.keysym.sym);
                if (key_index != CHIP8_KEY_UNDEFINED) {
                    chip8->key[key_index] = CHIP8_KEY_DOWN;
                }
            } break;

            case SDL_KEYUP: {
                chip8_key_t key_index = keycode_to_chip8_key(event.key.keysym.sym);
                if (key_index != CHIP8_KEY_UNDEFINED) {
                    chip8->key[key_index] = CHIP8_KEY_UP;
                }
            } break;
        }
    }
}

static int run_emu(const char *rom) {
    LOG_INFO_ARGS("Loading: '%s'", rom);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_init(&window, &renderer);

    chip8_t *chip8 = chip8_init();
    LOG_INFO_ARGS("Allocated a CHIP-8 at 0x%p", (void *)chip8);

    chip8_load_program(chip8, rom);
    LOG_INFO_ARGS("Loaded ROM '%s' (%zu bytes)", rom, chip8->rom_size);

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq     = 44100;          // Standard CD Quality frequency
    want.format   = AUDIO_S16SYS;   // 16-bit signed samples
    want.channels = 1;              // Mono
    want.samples  = 4096;           // Buffer size
    want.userdata = chip8;          // Pass chip8 so that the callback can check sound_timer
    want.callback = audio_callback; // Function that will generate the sound wave

    if (SDL_OpenAudio(&want, &have) < 0) {
        LOG_ERROR_ARGS("Failed to open audio: %s", SDL_GetError());
    }

    // By default the audio device starts as paused.
    SDL_PauseAudio(0);

    // Emulation loop
    while (chip8->running) {
        uint32_t start_frame_time = SDL_GetTicks();

        handle_input(chip8);

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
            chip8_step(chip8);
        }

        chip8_update_timers(chip8);

        if (chip8->draw) {
            draw_screen(chip8, renderer);
            chip8->draw = false;
        }

        uint32_t frame_duration = SDL_GetTicks() - start_frame_time;
        if (frame_duration < MILLISECS_PER_FRAME) {
            SDL_Delay(MILLISECS_PER_FRAME - frame_duration);
        }
    }

    // Cleanup
    SDL_CloseAudio();
    chip8_destroy(chip8);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "   %s --emu <rom.ch8>\n", *argv);
        return 1;
    }

    if (strcmp(argv[1], "--emu") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s --emu <rom.ch8>\n", *argv);
            return 1;
        }
        return run_emu(argv[2]);
    }

    fprintf(stderr, "Unknown flag '%s'. Use --emu.\n", argv[1]);
    return 1;
}
