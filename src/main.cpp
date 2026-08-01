#include <utility>
#include <string>
#include <cstdlib>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
// Note: If your OSD class uses fonts, you will likely need #include <SDL3_ttf/SDL_ttf.h>

#include "AtlasMapper.hpp"
#include "OSD.hpp"
#include "DiscordSDK.hpp"

constexpr int default_screen_size[] = {720, 1280};
constexpr int pixeled_screen_size[] = {144, 256};

Atlas::BITMAPS get_random_bg();

int main(int argc, char* argv[]) {
    DiscordManager discord;

    // Initialize SDL3 (Video automatically includes Events)
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return -1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Create window and renderer simultaneously (replaces al_create_display)
    if (!SDL_CreateWindowAndRenderer("Flappy Block", 
                                     default_screen_size[0], 
                                     default_screen_size[1], 
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL, 
                                     &window, &renderer)) {
        return -1;
    }

    // 1 = Standard VSync
    SDL_SetRenderVSync(renderer, 1); 

    // This completely replaces `refresh_transform()`. 
    // SDL handles all the scaling math automatically, even when resizing!
    SDL_SetRenderLogicalPresentation(renderer, 
                                     pixeled_screen_size[0], 
                                     pixeled_screen_size[1], 
                                     SDL_LOGICAL_PRESENTATION_STRETCH);

    //OSD osd; 
    // Note: You will likely need to pass `renderer` to your OSD class so it can draw text
    
    Atlas atlas(renderer);
    Atlas::BITMAPS bg = get_random_bg();

    if (atlas.get_icon()) {
        SDL_SetWindowIcon(window, atlas.get_icon());
    }

    double frametime_smooth = 0.0;
    
    // SDL_GetTicksNS returns nanoseconds. Dividing by 1e9 gives double seconds (same as al_get_time)
    auto ft_now = SDL_GetTicksNS() / 1e9; 

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            // No need for a WINDOW_RESIZED event handler because SDL_LOGICAL_PRESENTATION_STRETCH handles it!
            case SDL_EVENT_KEY_UP:
                if (ev.key.key == SDLK_R) {
                    bg = get_random_bg();
                }
                break;
            }
        }

        auto ft_later = SDL_GetTicksNS() / 1e9;
        const auto delta = ft_later - std::exchange(ft_now, ft_later);
        frametime_smooth = ((frametime_smooth * 9.0) + delta) / 10.0;

        // --- Rendering Phase ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer); // Clears the screen

        // Draw the background
        SDL_FRect src_rect = atlas.get_rect(bg);
        SDL_FRect dst_rect = {0.0f, 0.0f, src_rect.w, src_rect.h};
        SDL_RenderTexture(renderer, atlas.get_texture(), &src_rect, &dst_rect);

        //osd.show(std::to_string(frametime_smooth) + " ms\n" + std::to_string(1.0 / (frametime_smooth == 0.0 ? 1e-12 : frametime_smooth)) + " FPS");
        
        SDL_RenderPresent(renderer); // Flips the display
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

Atlas::BITMAPS get_random_bg()
{
    return rand() % 2 == 0 ? Atlas::BITMAPS::BG_LIGHT : Atlas::BITMAPS::BG_DARK;
}