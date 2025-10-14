#ifndef ESSENCIALS_H
#define ESSENCIALS_H

// 1. BIBLIOTECAS PADRÃO
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>
#include <stdbool.h>

// 2. ESTRUTURAS GLOBAIS
typedef struct { float x, y, w, h; } Rect;

// 3. DEFINES GERAIS (SÓ O QUE NÃO MUDA NUNCA)
#define LARGURA_TELA    1920
#define ALTURA_TELA     1080
#define FPS             60.0
#define MAX_WALLS       32

// 4. DECLARAÇÃO DAS VARIÁVEIS GLOBAIS (AVISA QUE ELAS EXISTEM)
extern ALLEGRO_DISPLAY* display;
extern ALLEGRO_TIMER* timer;
extern ALLEGRO_EVENT_QUEUE* event_queue;
extern ALLEGRO_BITMAP* fundo_cenario;
extern ALLEGRO_BITMAP* player_sprite;

// 5. PROTÓTIPOS DAS FUNÇÕES (AVISA QUE AS FUNÇÕES EXISTEM)
void error_msg(const char* texto);
int  inicializar();
void finalizar();
void carregar_cenario(int cenario);
void configurar_gatilhos();
void configurar_walls_do_cenario_1();
void configurar_walls_do_cenario_2();
void configurar_walls_do_cenario_3();
bool interseccao(Rect a, Rect b);
Rect rect_player();
void resolver_colisao(float* nx, float* ny);

#endif // ESSENCIALS_H
