#include "AtlasMapper.hpp"

ALLEGRO_BITMAP* Atlas::get(BITMAPS opt) const {
    auto it = m_map.find(opt);
    if (it != m_map.end()) return it->second;
    return nullptr;
}

ALLEGRO_BITMAP* Atlas::get_icon() const {
    return m_icon;
}

Atlas::Atlas() :
    m_atlas(b::embed<"resources/atlas.png">()),
    m_icon_src(b::embed<"resources/icon.png">()),
    m_fp(al_open_memfile((void*)m_atlas.data(), m_atlas.size(), "rb")),
    m_icon_fp(al_open_memfile((void*)m_icon_src.data(), m_icon_src.size(), "rb"))
{
    m_main = al_load_bitmap_f(m_fp, ".png");
    m_icon = al_load_bitmap_f(m_icon_fp, ".png");

    m_map[BITMAPS::BG_LIGHT] = al_create_sub_bitmap(m_main, 0, 0, 144, 256);
    m_map[BITMAPS::BG_DARK] = al_create_sub_bitmap(m_main, 146, 0, 144, 256);
}

Atlas::~Atlas() {
    al_destroy_bitmap(m_main);
    al_fclose(m_fp);
}