# Expects the library to be on the system

if(WIN32)
    message(STATUS "Finding Allegro via vcpkg/CMake lookup...")

    find_path(ALLEGRO_INCLUDE_DIRS NAMES allegro5/allegro.h REQUIRED)

    set(ALLEGRO_COMPONENTS
        allegro
        allegro_main
        allegro_image
        allegro_primitives
        allegro_color
        allegro_font
        allegro_ttf
        allegro_memfile
    )

    set(ALLEGRO_LIBRARIES "")
    foreach(comp ${ALLEGRO_COMPONENTS})
        find_library(ALLEGRO_${comp}_LIBRARY NAMES ${comp} REQUIRED)
        list(APPEND ALLEGRO_LIBRARIES ${ALLEGRO_${comp}_LIBRARY})
    endforeach()

else()
    find_package(PkgConfig REQUIRED)

    message(STATUS "Finding Allegro via PkgConfig...")

    pkg_check_modules(ALLEGRO REQUIRED
        allegro-5
        allegro_main-5
        allegro_image-5
        allegro_primitives-5
        #allegro_dialog-5
        allegro_color-5
        allegro_font-5
        allegro_ttf-5
        #allegro_audio-5
        #allegro_acodec-5
        allegro_memfile-5
        #allegro_physfs-5
    )
endif()

message(STATUS "Allegro ready.")