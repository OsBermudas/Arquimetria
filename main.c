#include "essencials.h"

// A função inicializador() permanece a mesma.

void errorMSG(char* text) {
    al_show_native_message_box(NULL, "ERRO", "Ocorreu o seguinte erro e o programa sera fechado:", text, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}

int inicializador() {
    // ... seu código de inicialização aqui, sem alterações ...
    if (!al_init()) {
        errorMSG("Falha ao inicializar a Allegro.");
        return -1;
    }

    if (!al_init_image_addon()) {
        errorMSG("Falha ao inicializar o add-on de imagens.");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        errorMSG("Falha ao criar o Timer");
        return -1;
    }

    display = al_create_display(larguraTela, alturaTela);
    if (!display) {
        errorMSG("Falha ao carregar a Janela");
        al_destroy_timer(timer);
        return -1;
    }

    event_queue = al_create_event_queue();
    if (!event_queue) {
        errorMSG("Falha ao criar a fila de eventos");
        al_destroy_timer(timer);
        al_destroy_display(display);
        return -1;
    }

    sprite = al_load_bitmap("imagens/Cute_Fantasy_Free/Player/Player_Actions.png");
    if (!sprite) {
        errorMSG("Falha ao carregar os sprites do jogo!");
        al_destroy_timer(timer);
        al_destroy_display(display);
        al_destroy_event_queue(event_queue);
        return -1;
    }
    al_convert_mask_to_alpha(sprite, al_map_rgb(255, 0, 255));

    fundo = al_load_bitmap("imagens/bush-border/3858.jpg");
    if (!fundo) {
        errorMSG("Erro ao Carregar o fundo do jogo");
        al_destroy_timer(timer);
        al_destroy_display(display);
        al_destroy_event_queue(event_queue);
        al_destroy_bitmap(sprite);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    return 1;
}


int main(void) {

    int sair = 0;

    int altura_sprite = 140, largura_sprite = 108;
    int colunas_folha = 4, coluna_atual = 0;
    int linha_atual = 0;
    int regiao_x_folha = 0, regiao_y_folha = 0;
    int frames_sprite = 6, cont_frames = 0;
    int pos_x_sprite = 50, pos_y_sprite = 150;
    int vel_x_sprite = 4, vel_y_sprite = 0;

    if (inicializador() != 1) {
        return -1;
    }

    while (!sair) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            cont_frames++;
            if (cont_frames >= frames_sprite) {
                cont_frames = 0;
                coluna_atual++;
                if (coluna_atual >= colunas_folha) {
                    coluna_atual = 0;
                }
                regiao_x_folha = coluna_atual * largura_sprite;
            }

            if (pos_x_sprite + largura_sprite >= larguraTela || pos_x_sprite <= 0) {
                vel_x_sprite = -vel_x_sprite;
            }

            pos_x_sprite += vel_x_sprite;
            pos_y_sprite += vel_y_sprite;

            al_draw_bitmap(fundo, 0, 0, 0);

            if (vel_x_sprite > 0) {
                al_draw_bitmap_region(sprite,
                    regiao_x_folha, regiao_y_folha,
                    largura_sprite, altura_sprite,
                    pos_x_sprite, pos_y_sprite, 0);
            }
            else {
                al_draw_bitmap_region(sprite,
                    regiao_x_folha, regiao_y_folha,
                    largura_sprite, altura_sprite,
                    pos_x_sprite, pos_y_sprite, ALLEGRO_FLIP_HORIZONTAL);
            }

            al_flip_display();
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = 1;
        }
    }
    al_destroy_bitmap(sprite);
    al_destroy_bitmap(fundo);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}