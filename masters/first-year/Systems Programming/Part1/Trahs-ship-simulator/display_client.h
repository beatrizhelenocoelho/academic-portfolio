#ifndef DISPLAY_CLIENT_H
#define DISPLAY_CLIENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// ------------------------------------------------------------
// Draw UTF-8 text at a fixed position.
// ------------------------------------------------------------
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* message);

// ------------------------------------------------------------
// Initialize SDL2 + SDL_ttf, create window, renderer, and font.
// Returns 1 on success, 0 on failure.
//
// Parameters (output):
//   font     -> loaded TTF font
//   window   -> created SDL_Window
//   renderer -> created SDL_Renderer
// ------------------------------------------------------------
int init_sdl(TTF_Font **font, SDL_Window **window, SDL_Renderer **renderer);

#endif // DISPLAY_CLIENT_H

