#pragma once

#include <unordered_map>
 
#include <battery/embed.hpp>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

class Atlas {
public:
    enum class BITMAPS {
        BG_LIGHT,
        BG_DARK,
        FLOOR,
        PIPE_GREEN,
        PIPE_GOLDEN,
        SIGN_GET_READY,
        SIGN_GAME_OVER,
    };

    // In SDL3, atlases are best handled by returning the source rectangle
    SDL_FRect get_rect(BITMAPS) const;
    SDL_Texture* get_texture() const;
    SDL_Surface* get_icon() const;
    TTF_Font* get_font() const;

    void draw_text(const std::string&, SDL_Renderer* renderer, const float, float) const;

private:
    void draw_single_line_text(const std::string_view&, SDL_Renderer* renderer, const float, const float) const;

    const b::EmbedInternal::EmbeddedFile& m_atlas;
    const b::EmbedInternal::EmbeddedFile& m_icon_src;
    const b::EmbedInternal::EmbeddedFile& m_ttf_src;
    
    TTF_Font* m_font = nullptr;
    SDL_Texture* m_main = nullptr;
    SDL_Surface* m_icon = nullptr;    
    
    std::unordered_map<BITMAPS, SDL_FRect> m_map;
public:
    // SDL requires the renderer to create hardware textures
    Atlas(SDL_Renderer* renderer);
    ~Atlas();

    Atlas(Atlas&&) = delete;
    Atlas(const Atlas&) = delete;
    void operator=(Atlas&&) = delete;
    void operator=(const Atlas&) = delete;
};