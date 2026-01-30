#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_stdinc.h>
#include <cstddef>
#include <iostream>
#include <SDL2/SDL.h>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "bus.hh"
#include "cartridge.hh"

std::shared_ptr<Bus> nes;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
SDL_AudioDeviceID audio_device = 0;
bool running = true;

void audio_callback(void* userdata, Uint8* stream, int len) {
    Bus* bus = (Bus*) userdata;
    float* fstream = (float*) stream;
    int n_samples = len / sizeof(float);

    for (int i = 0; i < n_samples; i++) {
        fstream[i] = bus->pop_audio_sample();
    }
}

void main_loop() {
    if (!running) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        #endif
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
        }
    }

    // controller 
    const uint8_t* keys = SDL_GetKeyboardState(nullptr);
    uint8_t controller = 0x00;

    if (keys[SDL_SCANCODE_X])      controller |= 0x80; // A
    if (keys[SDL_SCANCODE_Z])      controller |= 0x40; // B
    if (keys[SDL_SCANCODE_A])      controller |= 0x20; // Select
    if (keys[SDL_SCANCODE_S])      controller |= 0x10; // Start
    if (keys[SDL_SCANCODE_UP])     controller |= 0x08;
    if (keys[SDL_SCANCODE_DOWN])   controller |= 0x04;
    if (keys[SDL_SCANCODE_LEFT])   controller |= 0x02;
    if (keys[SDL_SCANCODE_RIGHT])  controller |= 0x01;

    nes->rp->controller[0] = controller;

    // one frame
    while (!nes->ppu.frame_complete) {
        nes->clk();

        if (nes->sys_clocks % 122 == 0) {
            float sample = nes->rp->apu.get_audio_sample();
            nes->push_audio_sample(sample);
        }
    }
    nes->ppu.frame_complete = false;

    // rendering
    SDL_UpdateTexture(
        texture,
        nullptr,
        nes->ppu.screen_buffer,
        256 * sizeof(uint32_t)
    );

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return -1;
    }

    window = SDL_CreateWindow(
        "nes",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        256 * 3,
        240 * 3,
        SDL_WINDOW_SHOWN
    );

    if (!window) return -1;

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256,
        240
    );

    // preloaded rom
    // TODO: in browser rom loading
    std::string rom_path = "rom.nes"; 
    if (argc > 1) rom_path = argv[1];

    nes = std::make_shared<Bus>();
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(rom_path);

    if (!cart->valid) {
        std::cerr << "failed to load cartridge: " << rom_path << std::endl;
        return -1;
    }

    nes->insert_cartridge(cart);
    nes->reset();

    // audio setup
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    want.userdata = nes.get();
    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    SDL_PauseAudioDevice(audio_device, 0);

    #ifdef __EMSCRIPTEN__
    // 0 fps = let browser decide (60?), 1 = simulate infinite loop
    emscripten_set_main_loop(main_loop, 0, 1);
    #else
    while(running) {
        main_loop();
    }
    #endif

    if (audio_device) {
        SDL_CloseAudioDevice(audio_device);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
