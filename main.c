#include "essencials.h"

// =========================================================================
// === CONSTANTES E VARIÁVEIS GLOBAIS - VERSÃO FINAL CORRIGIDA ===
// =========================================================================
enum TECLAS { CIMA, BAIXO, ESQUERDA, DIREITA };

const float PLAYER_SCALE = 6.0f;
const int FRAME_LARGURA = 16;
const int FRAME_ALTURA = 18;
const int MAX_FRAMES_POR_LINHA = 3;
const int FRAME_DELAY = 10;

float player_x, player_y;
float player_velocidade = 5.0f;
int   player_largura = 0, player_altura = 0;
bool  rodando = true;
bool  teclas[4] = { false, false, false, false };
int   cenario_atual = 1;
int   frame_atual = 1, frame_contador = 0;
int   player_direcao_y_offset = 0;

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
ALLEGRO_BITMAP* fundo_cenario = NULL;
ALLEGRO_BITMAP* player_sprite = NULL;

Rect saida_c1_para_c2, saida_c2_para_c3, saida_c3_fim;
Rect walls[MAX_WALLS];
int  wall_count = 0;

// Protótipos
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

// =========================================================================
// === CÓDIGO DO JOGO ===
// =========================================================================
int main(void) {
    if (!inicializar()) return -1;
    while (rodando) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) rodando = false;

        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {
            case ALLEGRO_KEY_W: teclas[CIMA] = true; break;
            case ALLEGRO_KEY_S: teclas[BAIXO] = true; break;
            case ALLEGRO_KEY_A: teclas[ESQUERDA] = true; break;
            case ALLEGRO_KEY_D: teclas[DIREITA] = true; break;
            }
        }
        if (event.type == ALLEGRO_EVENT_KEY_UP) {
            switch (event.keyboard.keycode) {
            case ALLEGRO_KEY_W: teclas[CIMA] = false; break;
            case ALLEGRO_KEY_S: teclas[BAIXO] = false; break;
            case ALLEGRO_KEY_A: teclas[ESQUERDA] = false; break;
            case ALLEGRO_KEY_D: teclas[DIREITA] = false; break;
            case ALLEGRO_KEY_ESCAPE: rodando = false; break;
            }
        }

        if (event.type == ALLEGRO_EVENT_TIMER) {
            bool esta_movendo = teclas[CIMA] || teclas[BAIXO] || teclas[ESQUERDA] || teclas[DIREITA];
            float nx = player_x, ny = player_y;

            if (teclas[CIMA])    ny -= player_velocidade;
            if (teclas[BAIXO])   ny += player_velocidade;
            if (teclas[ESQUERDA]) nx -= player_velocidade;
            if (teclas[DIREITA])  nx += player_velocidade;

            // --- CORREÇÃO FINAL DA LÓGICA DE ANIMAÇÃO ---
            // A ORDEM DAS LINHAS NA SUA SPRITESHEET É DIFERENTE. VAMOS CORRIGIR OS NÚMEROS.
            if (teclas[ESQUERDA]) {
                player_direcao_y_offset = FRAME_ALTURA * 2; // Esquerda está na 3ª linha (Y=36)
            }
            else if (teclas[DIREITA]) {
                player_direcao_y_offset = FRAME_ALTURA * 1; // Direita está na 2ª linha (Y=18)
            }
            else if (teclas[CIMA]) {
                player_direcao_y_offset = FRAME_ALTURA * 3; // Cima está na 4ª linha (Y=54)
            }
            else if (teclas[BAIXO]) {
                player_direcao_y_offset = FRAME_ALTURA * 0; // Baixo está na 1ª linha (Y=0)
            }

            if (esta_movendo) {
                frame_contador++;
                if (frame_contador >= FRAME_DELAY) {
                    frame_contador = 0;
                    frame_atual++;
                    if (frame_atual >= MAX_FRAMES_POR_LINHA) frame_atual = 0;
                }
            }
            else {
                frame_atual = 1;
            }

            resolver_colisao(&nx, &ny);
            player_x = nx; player_y = ny;

            Rect p = rect_player();
            if (cenario_atual == 1) { if (interseccao(p, saida_c1_para_c2)) carregar_cenario(2); }
            else if (cenario_atual == 2) { if (interseccao(p, saida_c2_para_c3)) carregar_cenario(3); }
            else if (cenario_atual == 3) { if (saida_c3_fim.w > 0 && interseccao(p, saida_c3_fim)) rodando = false; }

            al_draw_bitmap(fundo_cenario, 0, 0, 0);
            if (player_sprite) {
                al_draw_scaled_bitmap(player_sprite,
                    frame_atual * FRAME_LARGURA, player_direcao_y_offset,
                    FRAME_LARGURA, FRAME_ALTURA,
                    player_x, player_y,
                    FRAME_LARGURA * PLAYER_SCALE, FRAME_ALTURA * PLAYER_SCALE, 0);
            }
            al_flip_display();
        }
    }
    finalizar();
    return 0;
}

int inicializar() {
    al_init(); al_init_image_addon(); al_install_keyboard(); al_init_primitives_addon();
    timer = al_create_timer(1.0 / FPS);
    display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    al_set_window_title(display, "RESOLVIDO, CARALHO!");
    event_queue = al_create_event_queue();
    player_sprite = al_load_bitmap("imagens/imagem-sprite.png");
    if (!player_sprite) { error_msg("NAO ACHEI 'imagens/imagem-sprite.png'."); return 0; }
    player_largura = FRAME_LARGURA * PLAYER_SCALE;
    player_altura = FRAME_ALTURA * PLAYER_SCALE;
    carregar_cenario(1);
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);
    return 1;
}

void configurar_gatilhos() {
    saida_c1_para_c2 = (Rect){ 0,0,0,0 }; saida_c2_para_c3 = (Rect){ 0,0,0,0 }; saida_c3_fim = (Rect){ 0,0,0,0 };
    if (cenario_atual == 1) { saida_c1_para_c2 = (Rect){ 1150, 900, 150, 150 }; }
    else if (cenario_atual == 2) { saida_c2_para_c3 = (Rect){ 880, 540, 150, 80 }; }
    else if (cenario_atual == 3) { saida_c3_fim = (Rect){ 1200, 900, 180, 160 }; }
}

void finalizar() {
    if (player_sprite)  al_destroy_bitmap(player_sprite);
    if (fundo_cenario)  al_destroy_bitmap(fundo_cenario);
    if (timer)          al_destroy_timer(timer);
    if (event_queue)    al_destroy_event_queue(event_queue);
    if (display)        al_destroy_display(display);
}
void error_msg(const char* texto) {
    al_show_native_message_box(display, "ERRO", "Ocorreu um erro:", texto, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}
void carregar_cenario(int cenario) {
    if (fundo_cenario) { al_destroy_bitmap(fundo_cenario); fundo_cenario = NULL; }
    const char* caminho = NULL;
    cenario_atual = cenario;
    if (cenario == 1) {
        caminho = "imagens/cenarios/cenario1_1.png"; player_x = 572; player_y = 294;
        configurar_gatilhos(); configurar_walls_do_cenario_1();
    }
    else if (cenario == 2) {
        caminho = "imagens/cenarios/cenario2_1.png"; player_x = 1696; player_y = 260;
        configurar_gatilhos(); configurar_walls_do_cenario_2();
    }
    else {
        caminho = "imagens/cenarios/cenario3.png"; player_x = 880; player_y = 700;
        configurar_gatilhos(); configurar_walls_do_cenario_3();
    }
    fundo_cenario = al_load_bitmap(caminho);
    if (!fundo_cenario) {
        char msg[256]; snprintf(msg, sizeof(msg), "Falha ao carregar a imagem: %s", caminho);
        error_msg(msg); rodando = false;
    }
}
void add_wall(float x, float y, float w, float h) { if (wall_count < MAX_WALLS) walls[wall_count++] = (Rect){ x,y,w,h }; }
void clear_walls() { wall_count = 0; }
void configurar_walls_do_cenario_1() {
    clear_walls(); add_wall(460, 150, 1856, 45); add_wall(460, 150, 60, 1016); add_wall(1401, 190, 40, 1016);
    add_wall(518, 897, 640, 48); add_wall(1247, 902, 1856, 48);
}
void configurar_walls_do_cenario_2() {
    clear_walls(); add_wall(16, 16, 1888, 24); add_wall(16, 1040, 1888, 24); add_wall(16, 16, 24, 1048);
    add_wall(1880, 16, 24, 1048);
}
void configurar_walls_do_cenario_3() {
    clear_walls(); add_wall(480, 180, 1824, 32); add_wall(480, 180, 32, 920); add_wall(1408, 209, 32, 920);
    add_wall(57, 866, 1100, 32); add_wall(1243, 867, 936, 32);
}
bool interseccao(Rect a, Rect b) {
    if (b.w <= 0 || b.h <= 0) return false;
    return !(a.x + a.w <= b.x || a.x >= b.x + b.w || a.y + a.h <= b.y || a.y >= b.y + b.h);
}
Rect rect_player_xy(float x, float y) { return (Rect) { x, y, (float)player_largura, (float)player_altura }; }
Rect rect_player() { return rect_player_xy(player_x, player_y); }
void resolver_colisao(float* nx, float* ny) {
    float x = *nx, y = *ny;
    Rect testX = rect_player_xy(x, player_y);
    for (int i = 0; i < wall_count; ++i) {
        if (interseccao(testX, walls[i])) {
            if (x > player_x) x = walls[i].x - player_largura; else if (x < player_x) x = walls[i].x + walls[i].w;
            testX = rect_player_xy(x, player_y);
        }
    }
    Rect testY = rect_player_xy(x, y);
    for (int i = 0; i < wall_count; ++i) {
        if (interseccao(testY, walls[i])) {
            if (y > player_y) y = walls[i].y - player_altura; else if (y < player_y) y = walls[i].y + walls[i].h;
            testY = rect_player_xy(x, y);
        }
    }
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x > LARGURA_TELA - player_largura) x = LARGURA_TELA - player_largura;
    if (y > ALTURA_TELA - player_altura)   y = ALTURA_TELA - player_altura;
    *nx = x; *ny = y;
}
