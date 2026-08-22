#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <utility>

#include "AtlasMapper.hpp"
#include "DiscordSDK.hpp"

constexpr int default_screen_size[] = {720, 1280};
constexpr int pixeled_screen_size[] = {144, 256};

static DiscordManager* discord = nullptr;
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static Atlas* atlas = nullptr;
static Atlas::BITMAPS bg{};
static double frametime_smooth = 0.0, ft_now{};

Atlas::BITMAPS get_random_bg();

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    discord = new DiscordManager();

    int i;

    SDL_SetAppMetadata("FlappyBlock", "1.0", "com.lsw.flappyblock");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (TTF_Init() == -1) {
        SDL_Log("Couldn't initialize SDL_ttf: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Flappy Block", 
                                     default_screen_size[0], 
                                     default_screen_size[1], 
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN, 
                                     &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(renderer, 
                                     pixeled_screen_size[0], 
                                     pixeled_screen_size[1], 
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    atlas = new Atlas(renderer);
    bg = get_random_bg();

    if (const auto icon = atlas->get_icon()) {
        SDL_SetWindowIcon(window, icon);
    }
    
    ft_now = SDL_GetTicksNS() / 1e9;

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_KEY_UP) {
        switch(event->key.scancode) {
        case SDL_SCANCODE_F11:
            // toggle fullscreen
            SDL_SetWindowFullscreen(window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN));
            break;
        case SDL_SCANCODE_ESCAPE:
            return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
        }
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto ft_later = SDL_GetTicksNS() / 1e9;
    const auto delta = ft_later - std::exchange(ft_now, ft_later);
    frametime_smooth = ((frametime_smooth * 9.0) + delta) / 10.0;

    // --- Rendering Phase ---
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer); // Clears the screen

    // Draw the background
    SDL_FRect src_rect = atlas->get_rect(bg);
    SDL_FRect dst_rect = {0.0f, 0.0f, src_rect.w, src_rect.h};
    SDL_RenderTexture(renderer, atlas->get_texture(), &src_rect, &dst_rect);

    //osd.show(std::to_string(frametime_smooth) + " ms\n" + std::to_string(1.0 / (frametime_smooth == 0.0 ? 1e-12 : frametime_smooth)) + " FPS");
    
    
    const auto tmp_text = (std::to_string(frametime_smooth) + " ms\n" + std::to_string(1.0 / (frametime_smooth == 0.0 ? 1e-12 : frametime_smooth)) + " FPS");
    atlas->draw_text(tmp_text, renderer, 0, 0);
  

    SDL_RenderPresent(renderer); // Flips the display    

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete discord;
    TTF_Quit();
    /* SDL will clean up the window/renderer for us. */
}





Atlas::BITMAPS get_random_bg()
{
    return rand() % 2 == 0 ? Atlas::BITMAPS::BG_LIGHT : Atlas::BITMAPS::BG_DARK;
}