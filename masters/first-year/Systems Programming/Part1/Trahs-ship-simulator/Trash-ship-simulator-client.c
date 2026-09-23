#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include <SDL2/SDL_ttf.h>
#include "Comunnication.h"
#include "display_client.h"


int main(int argc, char** argv)
{
    // ---------------------------------------
    // CONNECT TO SERVER
    // ---------------------------------------
    void *fd = create_client_channel("127.0.0.1");


    send_connection_message(fd);

    ship ship;
    if (!receive_response_connection(fd, &ship)) {
        printf("ERROR CONNECTION!\n");
        return 1;
    }else{
        printf("Your name is %s\n", ship.name);
    }
    // ---------------------------------------
    // SDL + TTF INIT (NOW SEPARATED)
    // ---------------------------------------
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;

    if (!init_sdl(&font, &window, &renderer)) {
        return 1;
    }

   int running = 1;
   SDL_Event event;
   char direction_str[10] = "";
    // ---------------------------------------
    // MAIN LOOP
    // ---------------------------------------
    while (running)
    {
        while (SDL_PollEvent(&event)) {


            if (event.type == SDL_QUIT) {
                running = 0;
            }

            if (event.type == SDL_KEYDOWN) {

                direction_t direction;
                int send = 1;

                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        direction = LEFT;
                      //  printf("LEFT\n");
                    strcpy(direction_str, "←");

                        break;

                    case SDLK_RIGHT:
                        direction = RIGHT;
                     strcpy(direction_str, "→");

                    //printf("RIGHT\n");
                        break;

                    case SDLK_UP:
                        direction = UP;
                        strcpy(direction_str, "↑");

                        //printf("UP\n");
                        break;

                    case SDLK_DOWN:
                        direction = DOWN;
                        strcpy(direction_str, "↓");

                       // printf("DOWN\n");
                        break;

                    case SDLK_ESCAPE:
                        running = 0;
                        send = 0;
                        break;

                    default:
                        send = 0;
                        break;
                }

                if (send) {
                    send_movement_message(fd, &ship, direction);

                    int ok = receive_response(fd);

                    if (!ok) {
                        printf("Error on move message\n");
                        return 0;
                    }
                }
            }
        }



        draw_text(renderer, font, direction_str);

        SDL_RenderPresent(renderer);
        SDL_Delay(5); 
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

