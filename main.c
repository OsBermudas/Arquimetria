#include <allegro5/allegro.h>

int main() {
	ALLEGRO_DISPLAY* janela = NULL;
	al_init();
	janela = al_create_display(640, 480);
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_flip_display();
    al_rest(10.0);
    al_destroy_display(janela);
}