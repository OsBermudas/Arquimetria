#include "essencials.h"
#include <allegro5/allegro_native_dialog.h> 

// ===================================================================
//
//                 VARIÁVEIS GLOBAIS E CONSTANTES
//
// ===================================================================

// A "Máquina de Estados" que controla o jogo
enum GAME_STATE {
    MENU,
    JOGANDO,
    QUIZ,
    MENSAGEM // Estado para exibir a mensagem de recompensa
};

// Variável global que diz em qual estado estamos
int estado_atual = MENU;

// Bitmap para a imagem de fundo do menu
ALLEGRO_BITMAP* fundo_menu = NULL;

// Retângulo clicável para o botão de start
Rect botao_start_rect;


// --- Suas variáveis ---

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

Rect saida_c1_para_c2, saida_c2_para_c3, saida_c3_fim;
Rect walls[MAX_WALLS];
int  wall_count = 0;

// --- Variáveis do Quiz 1 (Cenário 1) ---
ALLEGRO_BITMAP* quiz_imagem = NULL;           // Quiz 1: quizWord1.png
Rect quiz_area_c1 = { 0, 0, 0, 0 };           // Gatilho C1
bool quiz_resolvido_c1 = false;               // Flag de desbloqueio C1->C2

// --- Variáveis do Quiz 2 (Cenário 2) ---
ALLEGRO_BITMAP* quiz2_imagem = NULL;          // Quiz 2: quizWord2.png
ALLEGRO_BITMAP* mensagem_imagem = NULL;       // Mensagem de Recompensa
Rect quiz_area_c2 = { 0, 0, 0, 0 };           // Gatilho C2
bool quiz_resolvido_c2 = false;               // Flag de desbloqueio C2->C3

// Retângulos clicáveis para as opções do Quiz 1 (C1) ---
Rect quiz_op_C1_A = { 0, 0, 0, 0 };
Rect quiz_op_C1_B = { 0, 0, 0, 0 };
Rect quiz_op_C1_C = { 0, 0, 0, 0 }; // Resposta Correta do Quiz 1
Rect quiz_op_C1_D = { 0, 0, 0, 0 };
Rect quiz_op_C1_E = { 0, 0, 0, 0 };

// Retângulos clicáveis para as opções do Quiz 2 (C2) ---
Rect quiz_op_C2_A = { 0, 0, 0, 0 };
Rect quiz_op_C2_B = { 0, 0, 0, 0 }; // Resposta Correta do Quiz 2
Rect quiz_op_C2_C = { 0, 0, 0, 0 };
Rect quiz_op_C2_D = { 0, 0, 0, 0 };
Rect quiz_op_C2_E = { 0, 0, 0, 0 };


//                        PROTÓTIPOS
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
void logica_quiz(ALLEGRO_EVENT event);
void debug_draw_rect(Rect r, ALLEGRO_COLOR cor, float espessura);
void limpar_teclas();


//                      FUNÇÃO MAIN (Principal)
int main(void) {

    if (!inicializar()) return -1;

    while (rodando) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        // --- Evento Global (funciona em todos os estados) ---
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }

        // --- Lógica da Máquina de Estados ---
        switch (estado_atual) {

            // ===================================
            //          ESTADO: MENU
            // ===================================
        case MENU: {
            // 1. Checar por cliques do mouse
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.mouse.button == 1) { // Botão esquerdo
                    int mx = event.mouse.x;
                    int my = event.mouse.y;

                    // Checa se o clique (mx, my) está DENTRO do botão
                    if (mx >= botao_start_rect.x &&
                        mx <= botao_start_rect.x + botao_start_rect.w &&
                        my >= botao_start_rect.y &&
                        my <= botao_start_rect.y + botao_start_rect.h)
                    {
                        // CLICOU! Muda o estado para JOGANDO
                        estado_atual = JOGANDO;
                    }
                }
            }

            // 2. Desenhar a tela de Menu
            if (event.type == ALLEGRO_EVENT_TIMER) {
                al_draw_bitmap(fundo_menu, 0, 0, 0);
                al_flip_display();
            }
        } break;


                 // ===================================
                 //          ESTADO: JOGANDO
                 // ===================================
        case JOGANDO: {

            // 1. Lógica de Teclado 
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

            // 2. Lógica do Jogo (Timer) 
            if (event.type == ALLEGRO_EVENT_TIMER) {
                bool esta_movendo = teclas[CIMA] || teclas[BAIXO] || teclas[ESQUERDA] || teclas[DIREITA];
                float nx = player_x, ny = player_y;

                if (teclas[CIMA])    ny -= player_velocidade;
                if (teclas[BAIXO])   ny += player_velocidade;
                if (teclas[ESQUERDA]) nx -= player_velocidade;
                if (teclas[DIREITA])  nx += player_velocidade;

                // Animação de direção
                if (teclas[BAIXO]) {
                    player_direcao_y_offset = FRAME_ALTURA * 2;
                }
                else if (teclas[DIREITA]) {
                    player_direcao_y_offset = FRAME_ALTURA * 1;
                }
                else if (teclas[ESQUERDA]) {
                    player_direcao_y_offset = FRAME_ALTURA * 3;
                }
                else if (teclas[CIMA]) {
                    player_direcao_y_offset = FRAME_ALTURA * 0;
                }

                // Animação de quadros (frames)
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

                // Colisão e movimento
                resolver_colisao(&nx, &ny);
                player_x = nx; player_y = ny;

                // Gatilhos de cenário
                Rect p = rect_player();

                // --- LÓGICA DO CENÁRIO 1 ---
                if (cenario_atual == 1) {
                    if (interseccao(p, saida_c1_para_c2) && saida_c1_para_c2.w > 0) carregar_cenario(2);

                    // Gatilho do Quiz 1
                    if (quiz_area_c1.w > 0 && interseccao(p, quiz_area_c1)) {
                        estado_atual = QUIZ;
                    }
                }
                // --- LÓGICA DO CENÁRIO 2 ---
                else if (cenario_atual == 2) {
                    if (interseccao(p, saida_c2_para_c3) && saida_c2_para_c3.w > 0) carregar_cenario(3);

                    // Gatilho do Quiz 2
                    if (quiz_area_c2.w > 0 && interseccao(p, quiz_area_c2)) {
                        estado_atual = QUIZ;
                    }
                }
                // --- LÓGICA DO CENÁRIO 3 ---
                else if (cenario_atual == 3) {
                    if (saida_c3_fim.w > 0 && interseccao(p, saida_c3_fim)) rodando = false;
                }

                // Desenho do jogo
                al_draw_bitmap(fundo_cenario, 0, 0, 0);

                // --- DEBUG DE GATILHOS (Comentado para ficar invisível) ---
                /*
                if (cenario_atual == 1 && quiz_area_c1.w > 0) {
                    debug_draw_rect(quiz_area_c1, al_map_rgb(255, 255, 0), 2); // Quiz 1 Amarelo
                }
                if (cenario_atual == 2 && quiz_area_c2.w > 0) {
                    debug_draw_rect(quiz_area_c2, al_map_rgb(0, 0, 255), 2); // Quiz 2 Azul
                }
                */
                // ---------------------------------------------------------

                if (player_sprite) {
                    al_draw_scaled_bitmap(player_sprite,
                        frame_atual * FRAME_LARGURA, player_direcao_y_offset,
                        FRAME_LARGURA, FRAME_ALTURA,
                        player_x, player_y,
                        FRAME_LARGURA * PLAYER_SCALE, FRAME_ALTURA * PLAYER_SCALE, 0);
                }
                al_flip_display();
            }

        } break; // Fim do case JOGANDO

                    // ===================================
                    //          ESTADO: QUIZ
                    // ===================================
        case QUIZ: {
            logica_quiz(event);

            if (event.type == ALLEGRO_EVENT_TIMER) {
                ALLEGRO_BITMAP* atual_quiz = NULL;

                // --- ORGANIZAÇÃO DE ASSETS DO QUIZ ---
                if (cenario_atual == 1) {
                    atual_quiz = quiz_imagem; // Usa Quiz 1
                }
                else if (cenario_atual == 2) {
                    atual_quiz = quiz2_imagem; // Usa Quiz 2
                }

                if (atual_quiz) {
                    float img_w = al_get_bitmap_width(atual_quiz);
                    float img_h = al_get_bitmap_height(atual_quiz);

                    al_draw_scaled_bitmap(atual_quiz, 0, 0, img_w, img_h,
                        0, 0, LARGURA_TELA, ALTURA_TELA, 0);
                }

                // --- DEBUG: DESENHO DOS BOTÕES DE RESPOSTA (Comentado para ficar invisível) ---
                /*
                float espessura = 2.0f;
                ALLEGRO_COLOR cor_erro = al_map_rgb(255, 0, 0); // Vermelho
                ALLEGRO_COLOR cor_certa = al_map_rgb(0, 255, 0); // Verde

                Rect *op_erradas[4];
                Rect op_certa;

                if (cenario_atual == 1) {
                    op_certa = quiz_op_C1_C;
                    op_erradas[0] = &quiz_op_C1_A; op_erradas[1] = &quiz_op_C1_B;
                    op_erradas[2] = &quiz_op_C1_D; op_erradas[3] = &quiz_op_C1_E;
                } else if (cenario_atual == 2) {
                    op_certa = quiz_op_C2_B; // Quiz 2: B é a correta
                    op_erradas[0] = &quiz_op_C2_A; op_erradas[1] = &quiz_op_C2_C;
                    op_erradas[2] = &quiz_op_C2_D; op_erradas[3] = &quiz_op_C2_E;
                }

                // Desenha as incorretas (Vermelho)
                for (int i = 0; i < 4; i++) {
                    debug_draw_rect(*op_erradas[i], cor_erro, espessura);
                }

                // Desenha a correta (Verde)
                debug_draw_rect(op_certa, cor_certa, espessura);
                */
                // ---------------------------------------------

                al_flip_display();
            }

        } break; // Fim do case QUIZ

                 // ===================================
                 //          ESTADO: MENSAGEM
                 // ===================================
        case MENSAGEM: {

            // 1. Lógica de Saída (Aguardando ENTER)
            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    estado_atual = JOGANDO; // Volta ao Cenário 2 (após a mensagem)
                    limpar_teclas();
                }
            }

            // 2. Desenho da Tela de Mensagem
            if (event.type == ALLEGRO_EVENT_TIMER) {
                if (mensagem_imagem) {
                    float img_w = al_get_bitmap_width(mensagem_imagem);
                    float img_h = al_get_bitmap_height(mensagem_imagem);

                    al_draw_scaled_bitmap(mensagem_imagem, 0, 0, img_w, img_h,
                        0, 0, LARGURA_TELA, ALTURA_TELA, 0);
                }
                al_flip_display();
            }

        } break; // Fim do case MENSAGEM
        } // Fim do switch

    } // Fim do while(rodando)

    finalizar();
    return 0;
}


// ===================================================================
//
//                   FUNÇÕES DE INICIALIZAÇÃO E FINALIZAÇÃO
//
// ===================================================================

int inicializar() {
    al_init();
    al_init_image_addon();
    al_install_keyboard();
    al_init_primitives_addon();
    al_install_mouse();

    // Inicialização do Addon de Diálogos Nativos (necessário para caixas de erro)
    if (!al_init_native_dialog_addon()) {
        fprintf(stderr, "Falha ao inicializar o addon de diálogos nativos!\n");
        return 0;
    }

    // Inicialização de Fontes (para estabilidade)
    if (!al_init_font_addon()) { error_msg("Falha ao inicializar o addon de fontes."); return 0; }
    if (!al_init_ttf_addon()) { error_msg("Falha ao inicializar o addon TTF."); return 0; }

    timer = al_create_timer(1.0 / FPS);
    display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    al_set_window_title(display, "Archimetria");
    event_queue = al_create_event_queue();

    // --- Carregar assets do JOGO ---
    player_sprite = al_load_bitmap("imagens/imagem-sprite.png");
    if (!player_sprite) { error_msg("NAO ACHEI 'imagens/imagem-sprite.png'."); return 0; }

    // Carregar assets do MENU ---
    fundo_menu = al_load_bitmap("imagens/menuJogo.png"); // O nome da sua imagem
    if (!fundo_menu) { error_msg("NAO ACHEI 'Design sem nome.png'."); return 0; }

    // --- Carrega as imagens dos Quizzes ---
    quiz_imagem = al_load_bitmap("imagens/quizWord1.png");
    if (!quiz_imagem) { error_msg("NAO ACHEI 'imagens/quizWord1.png'."); return 0; }

    quiz2_imagem = al_load_bitmap("imagens/quizWord2.png");
    if (!quiz2_imagem) { error_msg("NAO ACHEI 'imagens/quizWord2.png'."); return 0; }

    mensagem_imagem = al_load_bitmap("imagens/mensagemWord2.png");
    if (!mensagem_imagem) { error_msg("NAO ACHEI 'imagens/mensagemWord2.png'."); return 0; }

    player_largura = FRAME_LARGURA * PLAYER_SCALE;
    player_altura = FRAME_ALTURA * PLAYER_SCALE;

    // Carrega o cenário 1 (para estar pronto quando o jogo começar)
    carregar_cenario(1);

    // Registra as fontes de eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    // Registra a fonte de eventos do mouse
    al_register_event_source(event_queue, al_get_mouse_event_source());
    botao_start_rect = (Rect){ 1180, 200, 310, 90 };

    // --- Configuração dos retângulos de clique do QUIZ 1 (C1) ---
    const float LARG_C1 = 1200.0f;
    const float ALT_C1 = 80.0f;
    const float X_C1 = 690.0f;
    quiz_op_C1_A = (Rect){ X_C1, 400, LARG_C1, ALT_C1 };
    quiz_op_C1_B = (Rect){ X_C1, 500, LARG_C1, ALT_C1 };
    quiz_op_C1_C = (Rect){ X_C1, 600, LARG_C1, ALT_C1 }; // CORRETA
    quiz_op_C1_D = (Rect){ X_C1, 700, LARG_C1, ALT_C1 };
    quiz_op_C1_E = (Rect){ X_C1, 800, LARG_C1, ALT_C1 };

    // --- Configuração dos retângulos de clique do QUIZ 2 (C2) ---
    // (Ajustadas para as opções no canto direito da imagem quizWord2.png)
    const float LARG_C2 = 300.0f;
    const float ALT_C2 = 40.0f;
    const float X_C2 = 1150.0f;
    quiz_op_C2_A = (Rect){ X_C2, 565, LARG_C2, ALT_C2 };
    quiz_op_C2_B = (Rect){ X_C2, 645, LARG_C2, ALT_C2 }; // CORRETA
    quiz_op_C2_C = (Rect){ X_C2, 715, LARG_C2, ALT_C2 };
    quiz_op_C2_D = (Rect){ X_C2, 790, LARG_C2, ALT_C2 };
    quiz_op_C2_E = (Rect){ X_C2, 865, LARG_C2, ALT_C2 };


    al_start_timer(timer);
    return 1;
}

void finalizar() {
    if (player_sprite)  al_destroy_bitmap(player_sprite);
    if (fundo_cenario)  al_destroy_bitmap(fundo_cenario);
    if (fundo_menu)     al_destroy_bitmap(fundo_menu);

    // Destrói os assets dos Quizzes
    if (quiz_imagem)    al_destroy_bitmap(quiz_imagem);
    if (quiz2_imagem)   al_destroy_bitmap(quiz2_imagem);
    if (mensagem_imagem) al_destroy_bitmap(mensagem_imagem);

    al_shutdown_native_dialog_addon();
    if (timer)          al_destroy_timer(timer);
    if (event_queue)    al_destroy_event_queue(event_queue);
    if (display)        al_destroy_display(display);
}


// ===================================================================
//
//                    FUNÇÕES AUXILIARES
//
// ===================================================================

void error_msg(const char* texto) {
    al_show_native_message_box(display, "ERRO", "Ocorreu um erro:", texto, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}

void carregar_cenario(int cenario) {
    if (fundo_cenario) { al_destroy_bitmap(fundo_cenario); fundo_cenario = NULL; }
    const char* caminho = NULL;
    cenario_atual = cenario;

    // Zera os gatilhos por padrão
    quiz_area_c1 = (Rect){ 0, 0, 0, 0 };
    quiz_area_c2 = (Rect){ 0, 0, 0, 0 };

    // --- CONFIGURAÇÃO POR FASE ---
    if (cenario == 1) {
        caminho = "imagens/cenarios/cenario1_1.png";
        player_x = 530; player_y = 250;
        player_direcao_y_offset = FRAME_ALTURA * 2;
        configurar_gatilhos(); configurar_walls_do_cenario_1();

        // GATILHO QUIZ 1 (Cenário 1)
        quiz_area_c1 = (Rect){ 920, 500, 80, 80 };
    }
    else if (cenario == 2) {
        caminho = "imagens/cenarios/cenario2_1.png"; player_x = 1696; player_y = 260;
        configurar_gatilhos(); configurar_walls_do_cenario_2();

        // GATILHO QUIZ 2 (Cenário 2)
        quiz_area_c2 = (Rect){ 270, 890, 50, 50 };
    }
    else { // Cenário 3
        caminho = "imagens/cenarios/cenario3.png"; player_x = 880; player_y = 700;
        configurar_gatilhos(); configurar_walls_do_cenario_3();
    }

    fundo_cenario = al_load_bitmap(caminho);
    if (!fundo_cenario) {
        char msg[256]; snprintf(msg, sizeof(msg), "Falha ao carregar a imagem: %s", caminho);
        error_msg(msg); rodando = false;
    }
}


void configurar_gatilhos() {
    saida_c1_para_c2 = (Rect){ 0,0,0,0 };
    saida_c2_para_c3 = (Rect){ 0,0,0,0 };
    saida_c3_fim = (Rect){ 0,0,0,0 };

    if (cenario_atual == 1) {
        // Saída C1->C2: SÓ ATIVA SE QUIZ 1 FOR RESOLVIDO
        if (quiz_resolvido_c1) {
            saida_c1_para_c2 = (Rect){ 1100, 900, 150, 150 };
        }
    }
    else if (cenario_atual == 2) {
        // Saída C2->C3: SÓ ATIVA SE QUIZ 2 FOR RESOLVIDO
        if (quiz_resolvido_c2) {
            saida_c2_para_c3 = (Rect){ 880, 540, 150, 80 };
        }
    }
    else if (cenario_atual == 3) { saida_c3_fim = (Rect){ 1200, 900, 180, 160 }; }
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
    float final_x = *nx;
    float final_y = *ny;

    // --- 1. Resolução da Colisão no Eixo X (Horizontal) ---
    Rect testX = rect_player_xy(final_x, player_y);

    for (int i = 0; i < wall_count; ++i) {
        if (interseccao(testX, walls[i])) {
            if (final_x > player_x) {
                final_x = walls[i].x - player_largura;
            }
            else if (final_x < player_x) {
                final_x = walls[i].x + walls[i].w;
            }
            testX = rect_player_xy(final_x, player_y);
        }
    }

    // --- 2. Resolução da Colisão no Eixo Y (Vertical) ---
    Rect testY = rect_player_xy(final_x, final_y);

    for (int i = 0; i < wall_count; ++i) {
        if (interseccao(testY, walls[i])) {
            if (final_y > player_y) {
                final_y = walls[i].y - player_altura;
            }
            else if (final_y < player_y) {
                final_y = walls[i].y + walls[i].h;
            }
            testY = rect_player_xy(final_x, final_y);
        }
    }

    // --- 3. Limites de Tela ---
    if (final_x < 0) final_x = 0;
    if (final_y < 0) final_y = 0;
    if (final_x > LARGURA_TELA - player_largura) final_x = LARGURA_TELA - player_largura;
    if (final_y > ALTURA_TELA - player_altura) final_y = ALTURA_TELA - player_altura;

    // --- 4. Aplica as novas coordenadas ---
    *nx = final_x;
    *ny = final_y;
}

// ===================================================================
//
//               FUNÇÃO DE LÓGICA DO QUIZ (Controle de Respostas)
//
// ===================================================================

void logica_quiz(ALLEGRO_EVENT event) {

    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouse.button == 1) {
            int mx = event.mouse.x;
            int my = event.mouse.y;
            Rect clique = { (float)mx, (float)my, 1.0f, 1.0f };

            // Ponteiros para o conjunto de retângulos do quiz atual
            Rect* op_certa, * op_erradas[4];

            // --- SELEÇÃO DE LÓGICA E RETÂNGULOS POR CENÁRIO ---
            if (cenario_atual == 1) {
                // Quiz 1: C é correta
                op_certa = &quiz_op_C1_C;
                op_erradas[0] = &quiz_op_C1_A;
                op_erradas[1] = &quiz_op_C1_B;
                op_erradas[2] = &quiz_op_C1_D;
                op_erradas[3] = &quiz_op_C1_E;

            }
            else if (cenario_atual == 2) {
                // Quiz 2: B é correta
                op_certa = &quiz_op_C2_B;
                op_erradas[0] = &quiz_op_C2_A;
                op_erradas[1] = &quiz_op_C2_C;
                op_erradas[2] = &quiz_op_C2_D;
                op_erradas[3] = &quiz_op_C2_E;
            }
            else {
                return; // Ignora cliques se não estiver nos Cenários 1 ou 2.
            }
            // ----------------------------------------------------

            // --- 1. Checa Resposta Correta ---
            if (interseccao(clique, *op_certa)) {

                if (cenario_atual == 1) {
                    quiz_resolvido_c1 = true;
                    quiz_area_c1.w = 0; // Desativa o gatilho C1
                }
                else if (cenario_atual == 2) {
                    quiz_resolvido_c2 = true;
                    quiz_area_c2.w = 0; // Desativa o gatilho C2

                    limpar_teclas();
                    configurar_gatilhos();
                    estado_atual = MENSAGEM; // VAI PARA O ESTADO DE MENSAGEM
                    return; // Transição imediata
                }

                // Ações comuns após acerto (Cenário 1)
                limpar_teclas();
                configurar_gatilhos(); // Ativa a saída
                estado_atual = JOGANDO;
            }

            // --- 2. Checa Respostas Incorretas ---
            else {
                for (int i = 0; i < 4; i++) {
                    if (interseccao(clique, *op_erradas[i])) {
                        // Resposta incorreta: Volta para o jogo (porta continua bloqueada)
                        limpar_teclas();
                        estado_atual = JOGANDO;
                        return;
                    }
                }
            }
        }
    }
    // Permite sair do quiz com ESCAPE 
    else if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            limpar_teclas();
            estado_atual = JOGANDO;
        }
    }
}

// ===================================================================
//
//               FUNÇÃO AUXILIAR DE DEBUG (Desenho de Retângulo)
//
// ===================================================================

void debug_draw_rect(Rect r, ALLEGRO_COLOR cor, float espessura) {
    // Desenha apenas se o retângulo tiver dimensões válidas
    if (r.w > 0 && r.h > 0) {
        al_draw_rectangle(
            r.x,
            r.y,
            r.x + r.w,
            r.y + r.h,
            cor,
            espessura
        );
    }
}

// ===================================================================
//
//               FUNÇÃO AUXILIAR PARA LIMPAR MOVIMENTO
//
// ===================================================================

void limpar_teclas() {
    teclas[CIMA] = false;
    teclas[BAIXO] = false;
    teclas[ESQUERDA] = false;
    teclas[DIREITA] = false;
}