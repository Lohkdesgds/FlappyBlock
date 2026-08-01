#include "AtlasMapper.hpp"
#include <SDL3_image/SDL_image.h>

SDL_FRect Atlas::get_rect(BITMAPS opt) const {
    auto it = m_map.find(opt);
    if (it != m_map.end()) return it->second;
    return {0.0f, 0.0f, 0.0f, 0.0f};
}

SDL_Texture* Atlas::get_texture() const {
    return m_main;
}

SDL_Surface* Atlas::get_icon() const {
    return m_icon;
}

Atlas::Atlas(SDL_Renderer* renderer) :
    m_atlas(b::embed<"resources/atlas.png">()),
    m_icon_src(b::embed<"resources/icon.png">())
{
    // Load Icon to a CPU Surface
    SDL_IOStream* icon_io = SDL_IOFromConstMem(m_icon_src.data(), m_icon_src.size());
    m_icon = IMG_Load_IO(icon_io, true); // true = auto-close IO stream

    // Load Atlas to a Hardware Texture
    SDL_IOStream* atlas_io = SDL_IOFromConstMem(m_atlas.data(), m_atlas.size());
    SDL_Surface* atlas_surf = IMG_Load_IO(atlas_io, true);
    if (atlas_surf) {
        m_main = SDL_CreateTextureFromSurface(renderer, atlas_surf);
        SDL_DestroySurface(atlas_surf); // Free the CPU surface now that it's on the GPU
    }

    // Map sub-rectangles (x, y, w, h)
    m_map[BITMAPS::BG_LIGHT] = {0.0f, 0.0f, 144.0f, 256.0f};
    m_map[BITMAPS::BG_DARK]  = {146.0f, 0.0f, 144.0f, 256.0f};
}

Atlas::~Atlas() {
    if (m_main) SDL_DestroyTexture(m_main);
    if (m_icon) SDL_DestroySurface(m_icon);
}