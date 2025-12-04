#include "essencials.h"
#include <allegro5/allegro_native_dialog.h>

//                   ESTRUTURAS E DEFINIÇÕES


// Estados da máquina de estados
enum GAME_STATE {
    MENU,
    INSTRUCOES,
    JOGANDO,
    QUIZ,
    MENSAGEM,
    RESPOSTA_ERRADA,
    TELA_FINAL
};

// Teclas de controle
enum TECLAS {
    CIMA,
    BAIXO,
    ESQUERDA,
    DIREITA
};

// Configuração de cenário
typedef struct {
    const char* caminho_imagem;
    float player_start_x, player_start_y;
    int player_direcao_inicial;
    Rect quiz_area;
    Rect saida_area;
} ConfigCenario;

// Estrutura para gerenciar recursos do jogo
typedef struct {
    ALLEGRO_BITMAP* fundo_menu;
    ALLEGRO_BITMAP* imagem_instrucoes;
    ALLEGRO_BITMAP* fundo_cenarios[4];
    ALLEGRO_BITMAP* quiz_imagens[4];
    ALLEGRO_BITMAP* player_sprite;
    ALLEGRO_BITMAP* mensagem_imagem;
    ALLEGRO_BITMAP* resposta_errada_imagem;
    ALLEGRO_BITMAP* tela_final_imagem;
} RecursosJogo;


//                   VARIÁVEIS GLOBAIS


// Estado atual do jogo
int estado_atual = MENU;

// Recursos do jogo
RecursosJogo recursos = { 0 };

// Variável para controlar exibição das bordas debug
bool debug_bordas_visivel = false;

// Constantes do player
const float PLAYER_SCALE = 6.0f;
const int FRAME_LARGURA = 16;
const int FRAME_ALTURA = 18;
const int MAX_FRAMES_POR_LINHA = 3;
const int FRAME_DELAY = 10;

// Variáveis do player
float player_x, player_y;
float player_velocidade = 5.0f;
int player_largura = 0, player_altura = 0;
bool rodando = true;
bool teclas[4] = { false, false, false, false };
int cenario_atual = 1;
int frame_atual = 1, frame_contador = 0;
int player_direcao_y_offset = 0;

// Sistema de colisão e áreas
Rect saida_c1_para_c2, saida_c2_para_c3, saida_c3_fim;
Rect walls[MAX_WALLS];
int wall_count = 0;

// Estados dos quizzes
Rect quiz_area_c1 = { 0, 0, 0, 0 };
Rect quiz_area_c2 = { 0, 0, 0, 0 };
Rect quiz_area_c3 = { 0, 0, 0, 0 };
bool quiz_resolvido_c1 = false;
bool quiz_resolvido_c2 = false;
bool quiz_resolvido_c3 = false;

// Interface do menu
Rect botao_start_rect;

// Retângulos clicáveis dos quizzes
Rect quiz_op_C1_A, quiz_op_C1_B, quiz_op_C1_C, quiz_op_C1_D, quiz_op_C1_E;
Rect quiz_op_C2_A, quiz_op_C2_B, quiz_op_C2_C, quiz_op_C2_D, quiz_op_C2_E;
Rect quiz_op_C3_A, quiz_op_C3_B, quiz_op_C3_C, quiz_op_C3_D, quiz_op_C3_E;

// Botões da tela de resposta errada
Rect botao_reset_rect;
Rect botao_exit_rect;


//                   PROTÓTIPOS DE FUNÇÕES

// Inicialização e finalização
int inicializar();
void finalizar();
bool carregar_todos_recursos();
void liberar_todos_recursos();

// Funções de cada estado
void processar_estado_menu(ALLEGRO_EVENT event);
void processar_estado_instrucoes(ALLEGRO_EVENT event);
void processar_estado_jogando(ALLEGRO_EVENT event);
void processar_estado_quiz(ALLEGRO_EVENT event);
void processar_estado_mensagem(ALLEGRO_EVENT event);
void processar_estado_resposta_errada(ALLEGRO_EVENT event);
void processar_estado_tela_final(ALLEGRO_EVENT event);

// Funções de cenário
void carregar_cenario(int cenario);
void configurar_gatilhos();
void configurar_walls_do_cenario_1();
void configurar_walls_do_cenario_2();
void configurar_walls_do_cenario_3();

// Funções do player
void atualizar_player();
void desenhar_player();
void limpar_teclas();

// Funções de colisão
bool interseccao(Rect a, Rect b);
Rect rect_player();
Rect rect_player_xy(float x, float y);
void resolver_colisao(float* nx, float* ny);

// Funções de wall
void add_wall(float x, float y, float w, float h);
void clear_walls();
void desenhar_walls();
void desenhar_elementos_debug();

// Funções de quiz
void logica_quiz(ALLEGRO_EVENT event);
void configurar_quiz_rectangles();
void configurar_botoes_resposta_errada();


// Função para toggle debug
void toggle_debug_bordas();

// Funções auxiliares
void error_msg(const char* texto);
void debug_draw_rect(Rect r, ALLEGRO_COLOR cor, float espessura);


//                   CONFIGURAÇÕES DE CENÁRIOS

ConfigCenario obter_config_cenario(int cenario) {
    ConfigCenario config = { 0 };

    switch (cenario) {
    case 1:
        config.caminho_imagem = "imagens/cenarios/cenario1_1.png";
        config.player_start_x = 530;
        config.player_start_y = 250;
        config.player_direcao_inicial = FRAME_ALTURA * 2;
        config.quiz_area = (Rect){ 920, 500, 80, 80 };
        config.saida_area = (Rect){ 1100, 900, 150, 150 };
        break;
    case 2:
        config.caminho_imagem = "imagens/cenarios/cenario2_1.png";
        config.player_start_x = 1696;
        config.player_start_y = 260;
        config.player_direcao_inicial = 0;
        config.quiz_area = (Rect){ 270, 890, 50, 50 };
        config.saida_area = (Rect){ 880, 540, 150, 80 };
        break;
    case 3:
        config.caminho_imagem = "imagens/cenarios/cenario3.png";
        config.player_start_x = 880;
        config.player_start_y = 700;
        config.player_direcao_inicial = 0;
        config.quiz_area = (Rect){ 1300, 500, 80, 80 };
        config.saida_area = (Rect){ 1200, 900, 180, 160 };
        break;
    }

    return config;
}

//                   FUNÇÃO MAIN (Principal)

int main(void) {
    if (!inicializar()) {
        return -1;
    }

    while (rodando) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        // Evento global - fechar janela
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
            continue;
        }

        // Verificar tecla F11 globalmente (em qualquer estado)
        if (event.type == ALLEGRO_EVENT_KEY_UP) {
            if (event.keyboard.keycode == ALLEGRO_KEY_F11) {
                toggle_debug_bordas();
                continue; // Pular processamento normal do evento
            }
        }

        // Processar estado atual
        switch (estado_atual) {
        case MENU:
            processar_estado_menu(event);
            break;
        case INSTRUCOES:
            processar_estado_instrucoes(event);
            break;
        case JOGANDO:
            processar_estado_jogando(event);
            break;
        case QUIZ:
            processar_estado_quiz(event);
            break;
        case MENSAGEM:
            processar_estado_mensagem(event);
            break;
        case RESPOSTA_ERRADA:
            processar_estado_resposta_errada(event);
            break;
        case TELA_FINAL:
            processar_estado_tela_final(event);
            break;
        }
    }

    finalizar();
    return 0;
}

//                   FUNÇÃO TOGGLE DEBUG


void toggle_debug_bordas() {
    debug_bordas_visivel = !debug_bordas_visivel;

    // Feedback no console
    if (debug_bordas_visivel) {
        printf("DEBUG: Bordas coloridas ATIVADAS (F11 para desativar)\n");
    }
    else {
        printf("DEBUG: Bordas coloridas DESATIVADAS (F11 para ativar)\n");
    }
}

//                   FUNÇÕES DE ESTADO - MENU

void processar_estado_menu(ALLEGRO_EVENT event) {
    // Processar cliques do mouse
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouse.button == 1) { // Botão esquerdo
            int mx = event.mouse.x;
            int my = event.mouse.y;

            // Verificar clique no botão de start
            if (mx >= botao_start_rect.x &&
                mx <= botao_start_rect.x + botao_start_rect.w &&
                my >= botao_start_rect.y &&
                my <= botao_start_rect.y + botao_start_rect.h) {

                estado_atual = INSTRUCOES;
            }
        }
    }

    // Desenhar menu
    if (event.type == ALLEGRO_EVENT_TIMER) {
        if (recursos.fundo_menu) {
            al_draw_bitmap(recursos.fundo_menu, 0, 0, 0);
        }
        al_flip_display();
    }
}


//                  FUNÇÕES DE ESTADO - INSTRUCOES


void processar_estado_instrucoes(ALLEGRO_EVENT event) {
    // Aguardar ENTER ou ESPAÇO para continuar
    if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_ENTER ||
            event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
            estado_atual = JOGANDO;
            limpar_teclas();
        }
        // Permitir ESC para voltar ao menu
        else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            estado_atual = MENU;
        }
    }

    // Também permitir clique do mouse para continuar
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouse.button == 1) { // Botão esquerdo
            estado_atual = JOGANDO;
            limpar_teclas();
        }
    }

    // Desenhar tela de instruções
    if (event.type == ALLEGRO_EVENT_TIMER) {
        if (recursos.imagem_instrucoes) {
            float img_w = al_get_bitmap_width(recursos.imagem_instrucoes);
            float img_h = al_get_bitmap_height(recursos.imagem_instrucoes);

            al_draw_scaled_bitmap(recursos.imagem_instrucoes, 0, 0, img_w, img_h,
                0, 0, LARGURA_TELA, ALTURA_TELA, 0);
        }

        al_flip_display();
    }
}

//                   FUNÇÕES DE ESTADO - JOGANDO


void processar_estado_jogando(ALLEGRO_EVENT event) {
    // Processar entrada do teclado
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

    // Lógica do jogo (timer)
    if (event.type == ALLEGRO_EVENT_TIMER) {
        atualizar_player();

        // Verificar gatilhos
        Rect p = rect_player();

        if (cenario_atual == 1) {
            if (interseccao(p, saida_c1_para_c2) && saida_c1_para_c2.w > 0) {
                carregar_cenario(2);
            }

            if (quiz_area_c1.w > 0 && interseccao(p, quiz_area_c1)) {
                estado_atual = QUIZ;
            }
        }
        else if (cenario_atual == 2) {
            if (interseccao(p, saida_c2_para_c3) && saida_c2_para_c3.w > 0) {
                carregar_cenario(3);
            }

            if (quiz_area_c2.w > 0 && interseccao(p, quiz_area_c2)) {
                estado_atual = QUIZ;
            }
        }
        else if (cenario_atual == 3) {
            // Verificar quiz do cenário 3 antes da saída final
            if (quiz_area_c3.w > 0 && interseccao(p, quiz_area_c3)) {
                estado_atual = QUIZ;
            }
            else if (saida_c3_fim.w > 0 && interseccao(p, saida_c3_fim)) {
                // Vai para tela final
                estado_atual = TELA_FINAL;
            }
        }

        // Desenhar jogo
        if (recursos.fundo_cenarios[cenario_atual]) {
            al_draw_bitmap(recursos.fundo_cenarios[cenario_atual], 0, 0, 0);
        }

        // Desenhar elementos debug apenas se habilitado
        if (debug_bordas_visivel) {
            desenhar_elementos_debug();
        }

        desenhar_player();
        al_flip_display();
    }
}

//                   FUNÇÕES DE ESTADO - QUIZ

void processar_estado_quiz(ALLEGRO_EVENT event) {
    logica_quiz(event);

    if (event.type == ALLEGRO_EVENT_TIMER) {
        ALLEGRO_BITMAP* atual_quiz = NULL;

        if (cenario_atual == 1 && recursos.quiz_imagens[1]) {
            atual_quiz = recursos.quiz_imagens[1];
        }
        else if (cenario_atual == 2 && recursos.quiz_imagens[2]) {
            atual_quiz = recursos.quiz_imagens[2];
        }
        else if (cenario_atual == 3 && recursos.quiz_imagens[3]) {
            atual_quiz = recursos.quiz_imagens[3];
        }

        if (atual_quiz) {
            float img_w = al_get_bitmap_width(atual_quiz);
            float img_h = al_get_bitmap_height(atual_quiz);

            al_draw_scaled_bitmap(atual_quiz, 0, 0, img_w, img_h,
                0, 0, LARGURA_TELA, ALTURA_TELA, 0);
        }

        // Desenhar botões do quiz apenas se debug habilitado
        if (debug_bordas_visivel) {
            float espessura = 3.0f;
            ALLEGRO_COLOR cor_erro = al_map_rgb(255, 100, 100);
            ALLEGRO_COLOR cor_certa = al_map_rgb(100, 255, 100);

            if (cenario_atual == 1) {
                debug_draw_rect(quiz_op_C1_A, cor_erro, espessura);
                debug_draw_rect(quiz_op_C1_B, cor_erro, espessura);
                debug_draw_rect(quiz_op_C1_D, cor_erro, espessura);
                debug_draw_rect(quiz_op_C1_E, cor_erro, espessura);
                debug_draw_rect(quiz_op_C1_C, cor_certa, espessura);
            }
            else if (cenario_atual == 2) {
                debug_draw_rect(quiz_op_C2_A, cor_erro, espessura);
                debug_draw_rect(quiz_op_C2_C, cor_erro, espessura);
                debug_draw_rect(quiz_op_C2_D, cor_erro, espessura);
                debug_draw_rect(quiz_op_C2_E, cor_erro, espessura);
                debug_draw_rect(quiz_op_C2_B, cor_certa, espessura);
            }
            else if (cenario_atual == 3) {
                debug_draw_rect(quiz_op_C3_B, cor_erro, espessura);
                debug_draw_rect(quiz_op_C3_C, cor_erro, espessura);
                debug_draw_rect(quiz_op_C3_D, cor_erro, espessura);
                debug_draw_rect(quiz_op_C3_E, cor_erro, espessura);
                debug_draw_rect(quiz_op_C3_A, cor_certa, espessura);
            }
        }

        al_flip_display();
    }
}

//                   FUNÇÕES DE ESTADO - MENSAGEM

void processar_estado_mensagem(ALLEGRO_EVENT event) {
    // Aguardar ENTER para continuar
    if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            estado_atual = JOGANDO;
            limpar_teclas();
        }
    }

    // Desenhar mensagem
    if (event.type == ALLEGRO_EVENT_TIMER) {
        if (recursos.mensagem_imagem) {
            float img_w = al_get_bitmap_width(recursos.mensagem_imagem);
            float img_h = al_get_bitmap_height(recursos.mensagem_imagem);

            al_draw_scaled_bitmap(recursos.mensagem_imagem, 0, 0, img_w, img_h,
                0, 0, LARGURA_TELA, ALTURA_TELA, 0);
        }
        al_flip_display();
    }
}


//                   FUNÇÃO DE ESTADO - RESPOSTA ERRADA (CORRIGIDA)

void processar_estado_resposta_errada(ALLEGRO_EVENT event) {
    // Processar cliques do mouse
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouse.button == 1) { // Botão esquerdo
            int mx = event.mouse.x;
            int my = event.mouse.y;

            // Verificar clique no botão Reset
            if (mx >= botao_reset_rect.x &&
                mx <= botao_reset_rect.x + botao_reset_rect.w &&
                my >= botao_reset_rect.y &&
                my <= botao_reset_rect.y + botao_reset_rect.h) {

                // Resetar o jogo para o cenário 1
                quiz_resolvido_c1 = false;
                quiz_resolvido_c2 = false;
                quiz_resolvido_c3 = false;
                carregar_cenario(1);
                estado_atual = JOGANDO;
                limpar_teclas();
                return; // IMPORTANTE: Sair da função após processar
            }

            // Verificar clique no botão Exit
            if (mx >= botao_exit_rect.x &&
                mx <= botao_exit_rect.x + botao_exit_rect.w &&
                my >= botao_exit_rect.y &&
                my <= botao_exit_rect.y + botao_exit_rect.h) {

                // Sair do jogo
                rodando = false;
                return; // IMPORTANTE: Sair da função após processar
            }
        }
    }

    // Permitir teclas alternativas
    if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_R) {
            // Tecla R para Reset
            quiz_resolvido_c1 = false;
            quiz_resolvido_c2 = false;
            quiz_resolvido_c3 = false;
            carregar_cenario(1);
            estado_atual = JOGANDO;
            limpar_teclas();
            return;
        }
        else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            // ESC para sair
            rodando = false;
            return;
        }
    }

    // DESENHAR TELA DE RESPOSTA ERRADA - SEMPRE QUANDO FOR TIMER
    if (event.type == ALLEGRO_EVENT_TIMER) {
        // PRIMEIRA PRIORIDADE: Desenhar a imagem de resposta errada
        if (recursos.resposta_errada_imagem) {
            float img_w = al_get_bitmap_width(recursos.resposta_errada_imagem);
            float img_h = al_get_bitmap_height(recursos.resposta_errada_imagem);

            al_draw_scaled_bitmap(recursos.resposta_errada_imagem, 0, 0, img_w, img_h,
                0, 0, LARGURA_TELA, ALTURA_TELA, 0);
        }

        // SEGUNDA PRIORIDADE: Desenhar bordas dos botões (apenas se debug ativo)
        if (debug_bordas_visivel) {
            debug_draw_rect(botao_reset_rect, al_map_rgb(0, 255, 0), 3); // Verde para Reset
            debug_draw_rect(botao_exit_rect, al_map_rgb(255, 0, 0), 3);  // Vermelho para Exit
        }

        al_flip_display();
    }
}

//                   FUNÇÃO DE ESTADO - TELA FINAL

void processar_estado_tela_final(ALLEGRO_EVENT event) {
    if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            estado_atual = MENU;
            limpar_teclas();
        }
        else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            rodando = false;
        }
        else if (event.keyboard.keycode == ALLEGRO_KEY_R) {
            quiz_resolvido_c1 = false;
            quiz_resolvido_c2 = false;
            quiz_resolvido_c3 = false;
            carregar_cenario(1);
            estado_atual = JOGANDO;
            limpar_teclas();
        }
    }

    // Permitir clique do mouse para voltar ao menu
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouse.button == 1) { // Botão esquerdo
            estado_atual = MENU;
            limpar_teclas();
        }
    }

    // Desenhar tela final
    if (event.type == ALLEGRO_EVENT_TIMER) {
        if (recursos.tela_final_imagem) {
            float img_w = al_get_bitmap_width(recursos.tela_final_imagem);
            float img_h = al_get_bitmap_height(recursos.tela_final_imagem);

            al_draw_scaled_bitmap(recursos.tela_final_imagem, 0, 0, img_w, img_h,
                0, 0, LARGURA_TELA, ALTURA_TELA, 0);
        }

        al_flip_display();
    }
}

//                   FUNÇÕES DO PLAYER


void atualizar_player() {
    bool esta_movendo = teclas[CIMA] || teclas[BAIXO] ||
        teclas[ESQUERDA] || teclas[DIREITA];
    float nx = player_x, ny = player_y;

    // Calcular nova posição
    if (teclas[CIMA])     ny -= player_velocidade;
    if (teclas[BAIXO])    ny += player_velocidade;
    if (teclas[ESQUERDA]) nx -= player_velocidade;
    if (teclas[DIREITA])  nx += player_velocidade;

    // Atualizar direção da animação
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

    // Atualizar animação
    if (esta_movendo) {
        frame_contador++;
        if (frame_contador >= FRAME_DELAY) {
            frame_contador = 0;
            frame_atual++;
            if (frame_atual >= MAX_FRAMES_POR_LINHA) {
                frame_atual = 0;
            }
        }
    }
    else {
        frame_atual = 1;
    }

    // Resolver colisão e aplicar movimento
    resolver_colisao(&nx, &ny);
    player_x = nx;
    player_y = ny;
}

void desenhar_player() {
    if (!recursos.player_sprite) {
        return;
    }

    al_draw_scaled_bitmap(recursos.player_sprite,
        frame_atual * FRAME_LARGURA, player_direcao_y_offset,
        FRAME_LARGURA, FRAME_ALTURA,
        player_x, player_y,
        FRAME_LARGURA * PLAYER_SCALE, FRAME_ALTURA * PLAYER_SCALE, 0);
}

void limpar_teclas() {
    for (int i = 0; i < 4; i++) {
        teclas[i] = false;
    }
}


//                   FUNÇÕES DE INICIALIZAÇÃO


int inicializar() {
    // Inicializar Allegro
    if (!al_init()) {
        error_msg("Falha ao inicializar Allegro");
        return 0;
    }

    // Inicializar addons
    if (!al_init_image_addon()) {
        error_msg("Falha ao inicializar addon de imagens");
        return 0;
    }

    if (!al_install_keyboard()) {
        error_msg("Falha ao instalar teclado");
        return 0;
    }

    if (!al_init_primitives_addon()) {
        error_msg("Falha ao inicializar primitivas");
        return 0;
    }

    if (!al_install_mouse()) {
        error_msg("Falha ao instalar mouse");
        return 0;
    }

    if (!al_init_native_dialog_addon()) {
        error_msg("Falha ao inicializar diálogos nativos");
        return 0;
    }

    if (!al_init_font_addon()) {
        error_msg("Falha ao inicializar addon de fontes");
        return 0;
    }

    if (!al_init_ttf_addon()) {
        error_msg("Falha ao inicializar addon TTF");
        return 0;
    }

    // Criar componentes principais
    timer = al_create_timer(1.0 / FPS);
    display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    event_queue = al_create_event_queue();

    if (!timer || !display || !event_queue) {
        error_msg("Falha ao criar componentes do Allegro");
        return 0;
    }

    al_set_window_title(display, "Archimetria");

    // Carregar recursos
    if (!carregar_todos_recursos()) {
        return 0;
    }

    // Configurar player
    player_largura = FRAME_LARGURA * PLAYER_SCALE;
    player_altura = FRAME_ALTURA * PLAYER_SCALE;

    // Carregar cenário inicial
    carregar_cenario(1);

    // Configurar interface
    botao_start_rect = (Rect){ 1180, 200, 310, 90 };
    configurar_quiz_rectangles();
    configurar_botoes_resposta_errada();

    // Registrar eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    al_start_timer(timer);
    return 1;
}

//                   CARREGAMENTO DE RECURSOS (MELHORADO)

bool carregar_todos_recursos() {
    // Carregar menu
    recursos.fundo_menu = al_load_bitmap("imagens/menuJogo.png");
    if (!recursos.fundo_menu) {
        error_msg("Falha ao carregar menu");
        return false;
    }

    // Carregar imagem de instruções
    recursos.imagem_instrucoes = al_load_bitmap("imagens/inicio.png");
    if (!recursos.imagem_instrucoes) {
        error_msg("Falha ao carregar imagem de instruções");
        return false;
    }

    // Carregar cenários
    const char* caminhos_cenarios[] = {
        NULL,
        "imagens/cenarios/cenario1_1.png",
        "imagens/cenarios/cenario2_1.png",
        "imagens/cenarios/cenario3.png"
    };

    for (int i = 1; i <= 3; i++) {
        recursos.fundo_cenarios[i] = al_load_bitmap(caminhos_cenarios[i]);
        if (!recursos.fundo_cenarios[i]) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Falha ao carregar cenário %d: %s", i, caminhos_cenarios[i]);
            error_msg(msg);
            return false;
        }
    }

    // Carregar sprites do player
    recursos.player_sprite = al_load_bitmap("imagens/imagem-sprite.png");
    if (!recursos.player_sprite) {
        error_msg("Falha ao carregar sprite do player");
        return false;
    }

    // Carregar quizzes
    recursos.quiz_imagens[1] = al_load_bitmap("imagens/quizWord1.png");
    recursos.quiz_imagens[2] = al_load_bitmap("imagens/quizWord2.png");
    recursos.quiz_imagens[3] = al_load_bitmap("imagens/quizWord3.png");

    if (!recursos.quiz_imagens[1] || !recursos.quiz_imagens[2] || !recursos.quiz_imagens[3]) {
        error_msg("Falha ao carregar imagens dos quizzes");
        return false;
    }

    // Carregar mensagem
    recursos.mensagem_imagem = al_load_bitmap("imagens/mensagemWord2.png");
    if (!recursos.mensagem_imagem) {
        error_msg("Falha ao carregar imagem de mensagem");
        return false;
    }

    // CRÍTICO: Carregar imagem de resposta errada COM VERIFICAÇÃO MELHORADA
    recursos.resposta_errada_imagem = al_load_bitmap("imagens/resposta_erada.png");
    recursos.resposta_errada_imagem = al_load_bitmap("imagens/resposta_erada.png");
    if (!recursos.resposta_errada_imagem) {
        error_msg("Falha ao carregar imagem de resposta errada");
        return false;
    }

    // Carregar imagem da tela final
    recursos.tela_final_imagem = al_load_bitmap("imagens/mensagen_final.png");
    if (!recursos.tela_final_imagem) {
        error_msg("Falha ao carregar imagem da tela final");
        return false;
    }

    printf("Recursos carregados com sucesso!\n");
    return true;
}

void finalizar() {
    liberar_todos_recursos();

    if (timer)       al_destroy_timer(timer);
    if (event_queue) al_destroy_event_queue(event_queue);
    if (display)     al_destroy_display(display);

    al_shutdown_native_dialog_addon();
}

void liberar_todos_recursos() {
    if (recursos.fundo_menu) {
        al_destroy_bitmap(recursos.fundo_menu);
        recursos.fundo_menu = NULL;
    }

    if (recursos.imagem_instrucoes) {
        al_destroy_bitmap(recursos.imagem_instrucoes);
        recursos.imagem_instrucoes = NULL;
    }

    for (int i = 1; i <= 3; i++) {
        if (recursos.fundo_cenarios[i]) {
            al_destroy_bitmap(recursos.fundo_cenarios[i]);
            recursos.fundo_cenarios[i] = NULL;
        }
    }

    for (int i = 1; i <= 3; i++) {
        if (recursos.quiz_imagens[i]) {
            al_destroy_bitmap(recursos.quiz_imagens[i]);
            recursos.quiz_imagens[i] = NULL;
        }
    }

    if (recursos.player_sprite) {
        al_destroy_bitmap(recursos.player_sprite);
        recursos.player_sprite = NULL;
    }

    if (recursos.mensagem_imagem) {
        al_destroy_bitmap(recursos.mensagem_imagem);
        recursos.mensagem_imagem = NULL;
    }

    if (recursos.resposta_errada_imagem) {
        al_destroy_bitmap(recursos.resposta_errada_imagem);
        recursos.resposta_errada_imagem = NULL;
    }

    if (recursos.tela_final_imagem) {
        al_destroy_bitmap(recursos.tela_final_imagem);
        recursos.tela_final_imagem = NULL;
    }
}

//                   FUNÇÕES DE CENÁRIO

void carregar_cenario(int cenario) {
    if (cenario < 1 || cenario > 3) {
        return;
    }

    cenario_atual = cenario;
    ConfigCenario config = obter_config_cenario(cenario);

    // Configurar posição inicial do player
    player_x = config.player_start_x;
    player_y = config.player_start_y;
    player_direcao_y_offset = config.player_direcao_inicial;

    // Resetar gatilhos
    quiz_area_c1 = (Rect){ 0, 0, 0, 0 };
    quiz_area_c2 = (Rect){ 0, 0, 0, 0 };
    quiz_area_c3 = (Rect){ 0, 0, 0, 0 };

    // Configurar gatilhos específicos do cenário
    if (cenario == 1) {
        quiz_area_c1 = config.quiz_area;
        configurar_walls_do_cenario_1();
    }
    else if (cenario == 2) {
        quiz_area_c2 = config.quiz_area;
        configurar_walls_do_cenario_2();
    }
    else if (cenario == 3) {
        quiz_area_c3 = config.quiz_area;
        configurar_walls_do_cenario_3();
    }

    configurar_gatilhos();
}

void configurar_gatilhos() {
    // Resetar gatilhos
    saida_c1_para_c2 = (Rect){ 0, 0, 0, 0 };
    saida_c2_para_c3 = (Rect){ 0, 0, 0, 0 };
    saida_c3_fim = (Rect){ 0, 0, 0, 0 };

    if (cenario_atual == 1 && quiz_resolvido_c1) {
        ConfigCenario config = obter_config_cenario(1);
        saida_c1_para_c2 = config.saida_area;
    }
    else if (cenario_atual == 2 && quiz_resolvido_c2) {
        ConfigCenario config = obter_config_cenario(2);
        saida_c2_para_c3 = config.saida_area;
    }
    else if (cenario_atual == 3 && quiz_resolvido_c3) {
        ConfigCenario config = obter_config_cenario(3);
        saida_c3_fim = config.saida_area;
    }
}

//                   FUNÇÕES DE COLISÃO E WALLS


void add_wall(float x, float y, float w, float h) {
    if (wall_count < MAX_WALLS) {
        walls[wall_count++] = (Rect){ x, y, w, h };
    }
}

void clear_walls() {
    wall_count = 0;
}

void desenhar_walls() {
    // Definir cores diferentes para cada cenário
    ALLEGRO_COLOR cor_wall;

    switch (cenario_atual) {
    case 1:
        cor_wall = al_map_rgb(255, 0, 0);    // Vermelho para cenário 1
        break;
    case 2:
        cor_wall = al_map_rgb(0, 255, 255);  // Ciano para cenário 2
        break;
    case 3:
        cor_wall = al_map_rgb(255, 165, 0);  // Laranja para cenário 3
        break;
    default:
        cor_wall = al_map_rgb(255, 255, 255); // Branco padrão
        break;
    }

    // Desenhar todas as paredes com bordas coloridas
    for (int i = 0; i < wall_count; i++) {
        debug_draw_rect(walls[i], cor_wall, 2.0f);
    }
}

void desenhar_elementos_debug() {
    // Desenhar bordas das paredes
    desenhar_walls();

    // Desenhar quadrados de debug para gatilhos dos quizzes
    if (cenario_atual == 1 && quiz_area_c1.w > 0) {
        debug_draw_rect(quiz_area_c1, al_map_rgb(255, 255, 0), 3); // Amarelo para Quiz 1
    }
    if (cenario_atual == 2 && quiz_area_c2.w > 0) {
        debug_draw_rect(quiz_area_c2, al_map_rgb(0, 100, 255), 3); // Azul para Quiz 2
    }
    if (cenario_atual == 3 && quiz_area_c3.w > 0) {
        debug_draw_rect(quiz_area_c3, al_map_rgb(255, 0, 255), 3); // Magenta para Quiz 3
    }

    // Desenhar saídas/portais
    if (saida_c1_para_c2.w > 0) {
        debug_draw_rect(saida_c1_para_c2, al_map_rgb(0, 255, 0), 3); // Verde para saída C1->C2
    }
    if (saida_c2_para_c3.w > 0) {
        debug_draw_rect(saida_c2_para_c3, al_map_rgb(0, 255, 0), 3); // Verde para saída C2->C3
    }
    if (saida_c3_fim.w > 0) {
        debug_draw_rect(saida_c3_fim, al_map_rgb(255, 0, 255), 3); // Magenta para saída final
    }
}

void configurar_walls_do_cenario_1() {
    clear_walls();
    add_wall(460, 150, 1856, 45);
    add_wall(460, 150, 60, 1016);
    add_wall(1401, 190, 40, 1016);
    add_wall(518, 897, 640, 48);
    add_wall(1247, 902, 1856, 48);
}

void configurar_walls_do_cenario_2() {
    clear_walls();
    add_wall(16, 16, 1888, 24);
    add_wall(16, 1040, 1888, 24);
    add_wall(16, 16, 24, 1048);
    add_wall(1880, 16, 24, 1048);
}

void configurar_walls_do_cenario_3() {
    clear_walls();
    add_wall(480, 180, 1824, 32);
    add_wall(480, 180, 32, 920);
    add_wall(1408, 209, 32, 920);
    add_wall(57, 866, 1080, 32);
    add_wall(1253, 867, 936, 32);
}

bool interseccao(Rect a, Rect b) {
    if (b.w <= 0 || b.h <= 0) {
        return false;
    }
    return !(a.x + a.w <= b.x ||
        a.x >= b.x + b.w ||
        a.y + a.h <= b.y ||
        a.y >= b.y + b.h);
}

Rect rect_player_xy(float x, float y) {
    return (Rect) { x, y, (float)player_largura, (float)player_altura };
}

Rect rect_player() {
    return rect_player_xy(player_x, player_y);
}

void resolver_colisao(float* nx, float* ny) {
    float final_x = *nx;
    float final_y = *ny;

    // Resolução no eixo X
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

    // Resolução no eixo Y
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

    // Limites da tela
    if (final_x < 0) final_x = 0;
    if (final_y < 0) final_y = 0;
    if (final_x > LARGURA_TELA - player_largura) {
        final_x = LARGURA_TELA - player_largura;
    }
    if (final_y > ALTURA_TELA - player_altura) {
        final_y = ALTURA_TELA - player_altura;
    }

    *nx = final_x;
    *ny = final_y;
}

//                   FUNÇÕES DO QUIZ 


void configurar_quiz_rectangles() {
    // Quiz 1 (Cenário 1)
    const float LARG_C1 = 1200.0f;
    const float ALT_C1 = 80.0f;
    const float X_C1 = 690.0f;

    quiz_op_C1_A = (Rect){ X_C1, 400, LARG_C1, ALT_C1 };
    quiz_op_C1_B = (Rect){ X_C1, 500, LARG_C1, ALT_C1 };
    quiz_op_C1_C = (Rect){ X_C1, 600, LARG_C1, ALT_C1 }; // Correta
    quiz_op_C1_D = (Rect){ X_C1, 700, LARG_C1, ALT_C1 };
    quiz_op_C1_E = (Rect){ X_C1, 800, LARG_C1, ALT_C1 };

    // Quiz 2 (Cenário 2)
    const float LARG_C2 = 300.0f;
    const float ALT_C2 = 40.0f;
    const float X_C2 = 1150.0f;

    quiz_op_C2_A = (Rect){ X_C2, 565, LARG_C2, ALT_C2 };
    quiz_op_C2_B = (Rect){ X_C2, 645, LARG_C2, ALT_C2 }; // Correta
    quiz_op_C2_C = (Rect){ X_C2, 715, LARG_C2, ALT_C2 };
    quiz_op_C2_D = (Rect){ X_C2, 790, LARG_C2, ALT_C2 };
    quiz_op_C2_E = (Rect){ X_C2, 865, LARG_C2, ALT_C2 };

    // Quiz 3 (Cenário 3)
    const float LARG_C3 = 150.0f;
    const float ALT_C3 = 50.0f;
    const float X_C3 = 1250.0f;

    quiz_op_C3_A = (Rect){ X_C3, 500, LARG_C3, ALT_C3 }; // Correta
    quiz_op_C3_B = (Rect){ X_C3, 580, LARG_C3, ALT_C3 };
    quiz_op_C3_C = (Rect){ X_C3, 660, LARG_C3, ALT_C3 };
    quiz_op_C3_D = (Rect){ X_C3, 740, LARG_C3, ALT_C3 };
    quiz_op_C3_E = (Rect){ X_C3, 820, LARG_C3, ALT_C3 };
}

void configurar_botoes_resposta_errada() {
    botao_reset_rect = (Rect){ 550, 600, 200, 200 };
    botao_exit_rect = (Rect){ 1150, 550, 200, 250 };
}

// FUNÇÃO LÓGICA QUIZ CORRIGIDA
void logica_quiz(ALLEGRO_EVENT event) {
    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
        int mx = event.mouse.x;
        int my = event.mouse.y;
        Rect clique = { (float)mx, (float)my, 1.0f, 1.0f };

        Rect* op_certa = NULL;
        Rect* op_erradas[4] = { NULL, NULL, NULL, NULL };

        // Configurar opções baseadas no cenário atual
        if (cenario_atual == 1) {
            op_certa = &quiz_op_C1_C;
            op_erradas[0] = &quiz_op_C1_A;
            op_erradas[1] = &quiz_op_C1_B;
            op_erradas[2] = &quiz_op_C1_D;
            op_erradas[3] = &quiz_op_C1_E;
        }
        else if (cenario_atual == 2) {
            op_certa = &quiz_op_C2_B;
            op_erradas[0] = &quiz_op_C2_A;
            op_erradas[1] = &quiz_op_C2_C;
            op_erradas[2] = &quiz_op_C2_D;
            op_erradas[3] = &quiz_op_C2_E;
        }
        else if (cenario_atual == 3) {
            op_certa = &quiz_op_C3_A;
            op_erradas[0] = &quiz_op_C3_B;
            op_erradas[1] = &quiz_op_C3_C;
            op_erradas[2] = &quiz_op_C3_D;
            op_erradas[3] = &quiz_op_C3_E;
        }
        else {
            return; // Cenário inválido
        }

        // Verificar resposta correta
        if (op_certa && interseccao(clique, *op_certa)) {
            if (cenario_atual == 1) {
                quiz_resolvido_c1 = true;
                quiz_area_c1.w = 0; // Desativar gatilho
                limpar_teclas();
                configurar_gatilhos();
                estado_atual = JOGANDO;
            }
            else if (cenario_atual == 2) {
                quiz_resolvido_c2 = true;
                quiz_area_c2.w = 0; // Desativar gatilho
                limpar_teclas();
                configurar_gatilhos();
                estado_atual = MENSAGEM;
            }
            else if (cenario_atual == 3) {
                quiz_resolvido_c3 = true;
                quiz_area_c3.w = 0; // Desativar gatilho
                limpar_teclas();
                configurar_gatilhos();
                estado_atual = JOGANDO;  // Voltar ao jogo para permitir saída
            }
            return; // IMPORTANTE: Sair da função após processar resposta correta
        }

        // VERIFICAR RESPOSTAS INCORRETAS - TRANSIÇÃO IMEDIATA
        for (int i = 0; i < 4; i++) {
            if (op_erradas[i] && interseccao(clique, *op_erradas[i])) {
                printf("DEBUG: Resposta incorreta detectada! Mudando para estado RESPOSTA_ERRADA\n");
                limpar_teclas();
                estado_atual = RESPOSTA_ERRADA;
                return; // IMPORTANTE: Sair imediatamente após detectar erro
            }
        }
    }
    // Permitir sair com ESC
    else if (event.type == ALLEGRO_EVENT_KEY_UP) {
        if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            limpar_teclas();
            estado_atual = JOGANDO;
        }
    }
}

//                   FUNÇÕES AUXILIARES

void error_msg(const char* texto) {
    al_show_native_message_box(display, "ERRO", "Ocorreu um erro:",
        texto, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}

void debug_draw_rect(Rect r, ALLEGRO_COLOR cor, float espessura) {
    if (r.w > 0 && r.h > 0) {
        al_draw_rectangle(r.x, r.y, r.x + r.w, r.y + r.h, cor, espessura);
    }
}