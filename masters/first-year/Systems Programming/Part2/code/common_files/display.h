#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "structs.h"

// ============================================================================
// Initialize SDL window and renderer
// ----------------------------------------------------------------------------
// Creates an SDL window and renderer, and clears screen with background color.
//
// Parameters:
//   window_size - width and height of the square window
//   window      - pointer to store created SDL_Window
//   renderer    - pointer to store created SDL_Renderer
//   color       - background color
//
// Returns:
//   1 on success, 0 on failure.
// ============================================================================
int initialize_SDL_server(int window_size, SDL_Window **window,
                          SDL_Renderer **renderer, SDL_Color color);
int initialize_SDL_client(int window_size, SDL_Window **window,
                          SDL_Renderer **renderer, SDL_Color color);
// ============================================================================
// Initialize SDL_ttf and load font
// ----------------------------------------------------------------------------
// Loads a TTF font file at given size.
//
// Parameters:
//   font      - pointer to store loaded TTF_Font
//   text_size - font size
//   font_path - path to TTF font file
//
// Returns:
//   0 on success, 1 on failure.
// ============================================================================
int initialize_SDL_font(TTF_Font** font, int text_size, const char *font_path); 

// ============================================================================
// Set renderer draw color
// ----------------------------------------------------------------------------
// Sets the renderer's drawing color using an SDL_Color structure.
//
// Parameters:
//   renderer - SDL_Renderer to set color for
//   color    - SDL_Color to use
// ============================================================================
void SDL_SetRenderDrawColor_Direct_Color(SDL_Renderer *renderer, SDL_Color color); 

// ============================================================================
// Draw planets
// ----------------------------------------------------------------------------
// Draws planets as filled circles and labels with their trash counts.
//
// Parameters:
//   renderer     - SDL_Renderer to draw on
//   planets      - array of planets
//   num_planets  - number of planets
//   cell_size    - radius of planet circle
//   font         - TTF_Font for labels
// ============================================================================
void draw_planet(SDL_Renderer *renderer, planet_structure *planets,
                 int num_planets, int cell_size, TTF_Font *font);

// ============================================================================
// Draw untaken trash
// ----------------------------------------------------------------------------
// Draws all trash items that are not currently taken by ships.
//
// Parameters:
//   renderer   - SDL_Renderer to draw on
//   trash      - array of trash items
//   num_trash  - number of trash items
//   trash_size - radius of each trash circle
// ============================================================================
void draw_trash(SDL_Renderer *renderer, trash_structure *trash,
                int num_trash, int trash_size);

// ============================================================================
// Draw active ships
// ----------------------------------------------------------------------------
// Draws all active ships as filled circles with labels.
//
// Parameters:
//   renderer    - SDL_Renderer to draw on
//   ships       - array of ships
//   num_ships   - number of ships
//   cell_size   - radius of ship circle
//   font        - TTF_Font for labels
//   color       - color to draw ships
// ============================================================================
void draw_ships(SDL_Renderer *renderer, ship *ships, int num_ships,
                int cell_size, TTF_Font *font, SDL_Color *color);

// ============================================================================
// Render all game objects
// ----------------------------------------------------------------------------
// Clears screen and draws planets, trash, and ships.
//
// Parameters:
//   renderer      - SDL_Renderer to draw on
//   planets       - array of planets
//   n_planets     - number of planets
//   planet_radius - radius for planets
//   trash         - array of trash items
//   n_trash       - number of trash items
//   trash_radius  - radius for trash
//   ships         - array of ships
//   n_ships       - number of ships
//   font          - TTF_Font for labels
//   bg            - background color
//   c_ships       - color to draw ships
// ============================================================================
void render_all(SDL_Renderer *renderer,
                planet_structure *planets, int n_planets, int planet_radius,
                trash_structure *trash, int n_trash, int trash_radius,
                ship *ships, int n_ships, TTF_Font *font,
                SDL_Color bg, SDL_Color c_ships);



// ============================================================================
// Draw server close closing screen (client/server)
// ----------------------------------------------------------------------------
// Renders a centered closing message when the server shuts down correctly.
// The message varies depending on whether the caller is a server or client.
// Clears the screen with a background color, displays the message,
// and keeps it visible for a few seconds before returning.
//
// Parameters:
//   renderer    - SDL renderer used for drawing
//   font        - loaded TTF_Font used to render text
//   window_size - size of the window (assumes square window)
//   bg0         - background color used to clear the screen
//   user        - user type (1 = server, otherwise client)
//
// Returns:
//   void
// ============================================================================
void draw_server_close(
    SDL_Renderer *renderer,
    TTF_Font *font,
    int window_size,
    SDL_Color bg0,
    int user
);

// ============================================================================
// Draw world collapse closing screen (client/server)
// ----------------------------------------------------------------------------
// Renders a centered "GAME OVER" message when the world collapses.
// Clears the screen with a background color, displays the message,
// and keeps it visible for a few seconds before returning.
//
// Parameters:
//   renderer    - SDL renderer used for drawing
//   font        - loaded TTF_Font used to render text
//   window_size - size of the window (assumes square window)
//   bg0         - background color used to clear the screen
//   user        - user identifier (currently unused)
//
// Returns:
//   void
// ============================================================================
void draw_world_colapse_close(
    SDL_Renderer *renderer,
    TTF_Font *font,
    int window_size,
    SDL_Color bg0,
    int user
);

// ============================================================================
// Draw server collapsed closing screen (client-side)
// ----------------------------------------------------------------------------
// Renders a centered error message when the server crashes or is unreachable.
// Clears the screen with a background color, displays the message,
// and keeps it visible for a few seconds before returning.
//
// Parameters:
//   renderer    - SDL renderer used for drawing
//   font        - loaded TTF_Font used to render text
//   window_size - size of the window (assumes square window)
//   bg0         - background color used to clear the screen
//
// Returns:
//   void
// ============================================================================
void draw_server_collapsed(
    SDL_Renderer *renderer,
    TTF_Font *font,
    int window_size,
    SDL_Color bg0
);

#endif

