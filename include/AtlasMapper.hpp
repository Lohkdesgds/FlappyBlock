#pragma once

#include <unordered_map>
 
#include <battery/embed.hpp>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_memfile.h>

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

    ALLEGRO_BITMAP* get(BITMAPS);
private:
    const b::EmbedInternal::EmbeddedFile& m_atlas;
    ALLEGRO_BITMAP* m_main = nullptr;
    ALLEGRO_FILE* m_fp = nullptr;
    
    std::unordered_map<BITMAPS, ALLEGRO_BITMAP*> m_map;
public:
    Atlas();
    ~Atlas();

    Atlas(Atlas&&) = delete;
    Atlas(const Atlas&) = delete;
    void operator=(Atlas&&) = delete;
    void operator=(const Atlas&) = delete;
};