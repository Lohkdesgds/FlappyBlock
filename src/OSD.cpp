#include "OSD.hpp"

void OSD::show(const std::string& src) const {
    al_draw_multiline_text(m_bf, al_map_rgb(255,255,255), 0, 0, 144, 10, 0, src.c_str());
}

OSD::OSD()
    : m_bf(al_create_builtin_font())
{}

OSD::~OSD() {
    al_destroy_font(m_bf);
}