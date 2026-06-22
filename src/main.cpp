#include <allegro5/allegro.h>


int main() {
    al_init();
    ALLEGRO_DISPLAY* dp = al_create_display(1280, 720);

    al_rest(3.0);

    al_destroy_display(dp);

    return 0;
}