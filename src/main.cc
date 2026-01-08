#include <iostream>
#include <SDL2/SDL.h>
#include <memory>

#include "bus.hh"
#include "cartridge.hh"

int main(int argc, char* argv[]) {

    // sdl init
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "nes",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        256 * 3,
        240 * 3,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256,
        240
    );

    //emulator init
    if (argc < 2) {
        return -1;
    }

    std::shared_ptr<Bus> nes = std::make_shared<Bus>();
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);

    if (!cart->valid) {
        return -1;
    }

    nes->insert_cartridge(cart);
    nes->reset();

    bool running = true;
    SDL_Event event;

    // loop
    while (running) {

        // events
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

    // clean
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
