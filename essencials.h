#ifndef ESSENCIALS
#define ESSENCIALS

/*ALLEGRO BIBILIOTECAS*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>

/*ALLEGRO VARIÁVEIS AMBIENTES*/

#define larguraTela 1280
#define alturaTela 720
#define FPS 60.0

/*ALLEGRO VARIÁVEIS GLOBAIS*/

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_BITMAP* sprite = NULL;
ALLEGRO_BITMAP* fundo = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
ALLEGRO_TIMER* timer = NULL;

#endif