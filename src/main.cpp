#include <utility>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "AtlasMapper.hpp"
#include "OSD.hpp"

constexpr int default_screen_size[] = {720, 1280};
constexpr int pixeled_screen_size[] = {144, 256};

void refresh_transform(ALLEGRO_DISPLAY* dp);
Atlas::BITMAPS get_random_bg();

int main() {
    al_init();
    al_init_font_addon();
    al_init_image_addon();
    
    al_install_keyboard();
    al_install_mouse();

    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_RESIZABLE);
    al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);

    ALLEGRO_DISPLAY* dp = al_create_display(default_screen_size[0], default_screen_size[1]);
    ALLEGRO_EVENT_QUEUE* ev_qu = al_create_event_queue();
    
    OSD osd;
    Atlas atlas;
    Atlas::BITMAPS bg = get_random_bg();

    al_set_app_name("Flappy Block");
    al_set_display_icon(dp, atlas.get_icon());

    double frametime_smooth = 0.0;

    al_register_event_source(ev_qu, al_get_display_event_source(dp));
    al_register_event_source(ev_qu, al_get_keyboard_event_source());
    al_register_event_source(ev_qu, al_get_mouse_event_source());

    bool must_resize = true;
    auto ft_now = al_get_time();

    for(bool running = true; running;) {
        if (must_resize) {
            must_resize = false;
            al_acknowledge_resize(dp);
            refresh_transform(dp);
            bg = get_random_bg();
        }

        ALLEGRO_EVENT ev;
        while(running && al_get_next_event(ev_qu, &ev)) {
            switch(ev.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                must_resize = true;
                break;
            case ALLEGRO_EVENT_KEY_UP:
            {
                switch (ev.keyboard.keycode)
                {
                case ALLEGRO_KEY_R:
                    bg = get_random_bg();
                    break;
                default:
                    break;
                }
            }
                break;
            }
        }

        auto ft_later = al_get_time();
        const auto delta = ft_later - std::exchange(ft_now, ft_later);
        frametime_smooth = ((frametime_smooth * 9.0) + delta) / 10.0;

        al_draw_bitmap(atlas.get(bg), 0, 0, 0);

        osd.show(std::to_string(frametime_smooth) + " ms\n" + std::to_string(1.0 / (frametime_smooth == 0.0 ? 1e-12 : frametime_smooth)) + " FPS");
        al_flip_display();
    }

    al_destroy_display(dp);
    al_destroy_event_queue(ev_qu);

    return 0;
}

void refresh_transform(ALLEGRO_DISPLAY* dp)
{
    //ALLEGRO_DISPLAY* dp = al_get_current_display();
    ALLEGRO_TRANSFORM tf;

    al_identity_transform(&tf);
    al_scale_transform(&tf,
        al_get_display_width(dp) * 1.0f / pixeled_screen_size[0],
        al_get_display_height(dp) * 1.0f / pixeled_screen_size[1]);

    al_use_transform(&tf);

    //al_use_transform();
}

Atlas::BITMAPS get_random_bg()
{
    return rand() % 2 == 0 ? Atlas::BITMAPS::BG_LIGHT : Atlas::BITMAPS::BG_DARK;
}