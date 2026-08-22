#include "AtlasMapper.hpp"
#include <SDL3_image/SDL_image.h>

#include <ranges>
#include <string_view>

constexpr float c_font_size = 10.0f;

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

TTF_Font* Atlas::get_font() const {
    return m_font;
}

void Atlas::draw_text(const std::string& text, SDL_Renderer* renderer, const float x, float y) const {
    // split text on breakline individually
    for(auto lines: text | std::views::split('\n')) {
        std::string_view line_view{lines};
        draw_single_line_text(line_view, renderer, x, y);
        y += c_font_size;
    }
}

void Atlas::draw_single_line_text(const std::string_view& text, SDL_Renderer* renderer, const float x, const float y) const {
    SDL_Surface* textSurface = TTF_RenderText_Solid(
        m_font,
        text.data(),
        text.length(),
        SDL_Color{255, 255, 255, 255} // white color
    );
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_SetTextureScaleMode(textTexture, SDL_SCALEMODE_NEAREST);

    SDL_FRect textRect = {
        x,
        y,
        static_cast<float>(textSurface->w),
        static_cast<float>(textSurface->h)
    };

    SDL_DestroySurface(textSurface); // Free the surface now that we have a texture

    // Render the texture
    SDL_RenderTexture(renderer, textTexture, NULL, &textRect);

    // Clean up
    SDL_DestroyTexture(textTexture);  
}

Atlas::Atlas(SDL_Renderer* renderer) :
    m_atlas(b::embed<"resources/atlas.png">()),
    m_icon_src(b::embed<"resources/icon.png">()),
    m_ttf_src(b::embed<"resources/Artifika_font.ttf">())
{
    // Load Icon to a CPU Surface
    SDL_IOStream* icon_io = SDL_IOFromConstMem(m_icon_src.data(), m_icon_src.size());
    m_icon = IMG_Load_IO(icon_io, true); // true = auto-close IO stream

    // Load TTF Font
    SDL_IOStream* ttf_io = SDL_IOFromConstMem(m_ttf_src.data(), m_ttf_src.size());
    m_font = TTF_OpenFontIO(ttf_io, true, c_font_size); // autoclose
    
    // Check if font loaded successfully
    if (!m_font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        throw std::runtime_error(std::format("Failed to load font: {}", SDL_GetError()));
    }

    // Load Atlas to a Hardware Texture
    SDL_IOStream* atlas_io = SDL_IOFromConstMem(m_atlas.data(), m_atlas.size());
    SDL_Surface* atlas_surf = IMG_Load_IO(atlas_io, true);

    if (atlas_surf) {
        m_main = SDL_CreateTextureFromSurface(renderer, atlas_surf);
        SDL_SetTextureScaleMode(m_main, SDL_SCALEMODE_NEAREST);
        SDL_DestroySurface(atlas_surf); // Free the CPU surface now that it's on the GPU
    }

    // Map sub-rectangles (x, y, w, h)
    m_map[BITMAPS::BG_LIGHT] = {0.0f, 0.0f, 144.0f, 256.0f};
    m_map[BITMAPS::BG_DARK]  = {146.0f, 0.0f, 144.0f, 256.0f};
}

Atlas::~Atlas() {
    if (m_main) SDL_DestroyTexture(m_main);
    if (m_icon) SDL_DestroySurface(m_icon);
    if (m_font) TTF_CloseFont(m_font);
}