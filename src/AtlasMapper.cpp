#include "AtlasMapper.hpp"

#include <battery/embed.hpp>

Atlas::Atlas() :
    m_atlas(b::embed<"resources/atlas.png">()),
    m_fp(al_open_memfile(m_atlas.data(), m_atlas.size(), "r"))
{ 
}