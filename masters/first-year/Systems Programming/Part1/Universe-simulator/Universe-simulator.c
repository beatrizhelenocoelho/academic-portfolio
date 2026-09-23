#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "display.h"
#include "Universe-data.h"
#include "structs.h"
#include "read_conf.h"

int main(){
    srand(time(NULL));   
    
    // -------------------------
    // 1) Ler ficheiro do config
    // -------------------------
    config_t cfg;
    if(open_conf_file(&cfg, "config.conf")){
        // se der erro a abrir o config, não consigo continuar
        return 1;
    }

    // cores definidas no ficheiro de configuração
    SDL_Color red   = getColor(&cfg, "red");
    SDL_Color blue  = getColor(&cfg, "blue");
    SDL_Color green = getColor(&cfg, "green");
    SDL_Color bg0   = getColor(&cfg, "white");
    
    // restantes parâmetros do universo/jogo lidos do config
    int window_size, n_planets, max_trash, initial_trash, ship_capacity;
    int planet_radius, trash_radius, player_radius;

    if(!read_configurations(&cfg, &window_size, &n_planets, &max_trash,
                            &initial_trash, &ship_capacity,
                            &planet_radius, &trash_radius, &player_radius)){
        printf("ERRROR reading data for configuration\n");
        return 1;
    }

    config_destroy(&cfg); 
   
    // -------------------------
    // 2) Iniciar SDL 
    // -------------------------
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!initialize_SDL_simple(window_size, &window, &renderer, bg0)) {
        return 1;
    }

    set_universe_size(window_size);                   

    // -------------------------
    // 3) labels nos planetas
    // -------------------------
    TTF_Font *font = NULL;
    int font_size = 20;
    if (initialize_SDL_font(&font, font_size, "./HackNerdFontMono-Regular.ttf") != 0) {
        return 1;
    }

    // -------------------------
    // 4) Criar planetas e lixo inicial
    // -------------------------

    // criar o array de planetas com base nos dados do config
    // a função trata de distribuir os planetas pelo universo
    planet_structure *planets = create_planet(n_planets, window_size, 10, &red, &blue);  
    if (planets == NULL) {
        printf("ERROR creating planets\n");
        return 1;
    }

    // criar o lixo inicial de forma aleatória (1 lixo como está no config)
    // current_trash vai começar em initial_trash
    trash_structure *trash = create_trash_init(&green, 1, window_size,
                                               initial_trash, max_trash);

    if (trash == NULL) {
        printf("ERROR creating trash\n");
        free(planets);
        return 1;
    }

    // este array serve para saber, para cada bola de lixo,
    // se no frame anterior ela já estava em colisão com algum planeta.
    // assim, só crio lixo novo quando a colisão acontece outra vez
    // e não enquanto a bola está colada ao planeta.
    int *was_colliding = calloc(max_trash, sizeof(int));
    if (was_colliding == NULL) {
        printf("ERROR allocating was_colliding\n");
        free(planets);
        free(trash);
        return 1;
    }

    // -------------------------
    // 5) Variáveis principais da simulação
    // -------------------------

    int current_trash = initial_trash;  
    int quit = 0;                       // controlo do loop principal
    int universe_collapsed = 0;         // fica a 1 quando o universo enche de lixo e para saber que game over!!!
    SDL_Event e;

    const Uint32 STEP_MS = 10;          // (10 ms) atualiza...
    Uint32 last_update = SDL_GetTicks();

    // -------------------------
    // 6) Loop principal 
    // -------------------------
    while (!quit) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN &&
                       e.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_update >= STEP_MS) {
            last_update = now;

            new_trash_acceleration(planets, n_planets, trash, current_trash);
            new_trash_velocity(trash, current_trash);
            new_trash_position(trash, current_trash);

            int i = 0;
            while (i < current_trash) {
                int is_colliding = 0;

                // para cada bola de lixo, vejo se está perto o suficiente
                // de algum planeta para considerar colisão, ou seja o tamanho do lixo colide com o pixel do centro 
                for (int p = 0; p < n_planets; p++) {
                    float dx = (float)planets[p].x - (float)trash[i].x;
                    float dy = (float)planets[p].y - (float)trash[i].y;
                    
                    float dist2 = dx * dx + dy * dy;

                    // aqui estou a usar o raio do lixo como referência.
                    // como dist2 é a distância ao quadrado, esta condição
                    // faz com que só conte colisão quando o lixo já está
                    // bastante próximo do centro do planeta.
                    if (dist2 <= (float)trash_radius) {
                        is_colliding = 1;
                        break;
                    }
                }

                // “nova colisão” = estava sem colisão no frame anterior
                // e agora passou a is_colliding == 1
                if (is_colliding && !was_colliding[i] && current_trash < max_trash) {
                    // quando isto acontece, crio uum novo lixo
                    // numa posição aleatória do universo
                    trash[current_trash] = create_trash(&green, 1, window_size);
                    // o novo lixoo entra no sistema como "não em colisão"
                    was_colliding[current_trash] = 0;
                    current_trash++;
                }

                // atualizo o estado desta bola para poder comparar no próximo frame
                was_colliding[i] = is_colliding;

                i++;
            }

            //  Verificar se o universo colapsou 
            if (current_trash >= max_trash) {
                printf("O universo colapsou! Lixo = %d\n", current_trash);
                universe_collapsed = 1;
                quit = 1;
            }

            SDL_SetRenderDrawColor_Direct_Color(renderer, bg0);
            SDL_RenderClear(renderer);

            draw_planet(renderer, planets, n_planets, planet_radius, font);
            draw_trash(renderer, trash, current_trash, trash_radius);

            SDL_RenderPresent(renderer);
        } else {
            SDL_Delay(1); 
        }
    }

    // -------------------------
    // 7) Ecrã final quando o universo fica cheio de lixo ----- GAME OVER!!!!!!!!!!!!!!!!!!!-------
    // -------------------------
    if (universe_collapsed) {
        SDL_Color red = {255, 0, 0, 255};
        const char *msg = "The universe is full of trash! GAME OVER!";

        SDL_Surface *surface = TTF_RenderText_Blended(font, msg, red);
        if (surface != NULL) {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            int tw = 0, th = 0;
            SDL_QueryTexture(texture, NULL, NULL, &tw, &th);

            SDL_Rect dst = {
                (window_size - tw) / 2,
                (window_size - th) / 2,
                tw, th
            };

            // limpo o ecrã e mostro a mensagem centrada
            SDL_SetRenderDrawColor(renderer, bg0.r, bg0.g, bg0.b, bg0.a);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, texture, NULL, &dst);
            SDL_RenderPresent(renderer);

            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);

            SDL_Delay(3000); // deixo a mensagem 3 segundos antes de fechar
        }
    }

    // -------------------------
    // 8) Libertar recursos e sair
    // -------------------------
    if (font) {
        TTF_CloseFont(font);
        TTF_Quit();
    }

    free(planets);
    free(trash);
    free(was_colliding);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
