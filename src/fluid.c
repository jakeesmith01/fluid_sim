#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x34c3eb 
#define COLOR_GRAY 0x1f1f1f1f
#define CELL_SIZE 20 // size of each cell in pixels
#define LINE_WIDTH 2 // width of the lines that separate the cells
#define NUM_COLUMNS WIDTH / CELL_SIZE 
#define NUM_ROWS HEIGHT / CELL_SIZE
#define WATER_TYPE 0
#define SOLID_TYPE 1


struct Cell{
    int type; // 1 = solid, 0 = fluid
    double fill_level; // how much liquid is inside the cell (between 0 and 1)
    int x;
    int y;
};

/// @brief Colors a cell on the screen
/// @param surface The surface to draw to 
/// @param x The x coordinate of the cell
/// @param y The y coordinate of the cell
void draw_cell(SDL_Surface* surface, struct Cell cell){
    int pixel_x = cell.x * CELL_SIZE;
    int pixel_y = cell.y * CELL_SIZE;

    SDL_Rect cell_rect = (SDL_Rect){ pixel_x, pixel_y, CELL_SIZE, CELL_SIZE };

    // Background color
    SDL_FillRect(surface, &cell_rect, COLOR_BLACK);

    // Water fill level
    if(cell.type == WATER_TYPE){
        int water_level = cell.fill_level > 1 ? CELL_SIZE : cell.fill_level * CELL_SIZE; //height of the water pixels
        int empty_level = CELL_SIZE - water_level; // height of the empty pixels 
        SDL_Rect water_rect = (SDL_Rect){ pixel_x, pixel_y + empty_level, CELL_SIZE, water_level };
        SDL_FillRect(surface, &water_rect, COLOR_BLUE);
    }

    // Solid cell
    if(cell.type == SOLID_TYPE){
        SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
    }
}

/// @brief Draws the environment on the surface
/// @param surface The surface to draw to
/// @param environment The environment containing the cell information 
void draw_environment(SDL_Surface* surface, struct Cell environment[NUM_ROWS * NUM_COLUMNS]){
    for(int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++){
        draw_cell(surface, environment[i]);
    }
}

/// @brief Draws the grid of cells on the screen
/// @param surface The surface to draw to
void draw_grid(SDL_Surface* surface){
    for(int i = 0; i < NUM_COLUMNS; i++){
        SDL_Rect column = (SDL_Rect) { i * CELL_SIZE, 0, LINE_WIDTH, HEIGHT };
        SDL_FillRect(surface, &column, COLOR_GRAY);
    }
    for(int j = 0; j < NUM_ROWS; j++){
        SDL_Rect row = (SDL_Rect) { 0, j * CELL_SIZE, WIDTH, LINE_WIDTH };
        SDL_FillRect(surface, &row, COLOR_GRAY);
    }
}

/// @brief Initializes the environment with water cells at 0 fill level
/// @param environment The environment to initialize
void init_environment(struct Cell environment[NUM_ROWS * NUM_COLUMNS]){
    for(int i = 0; i < NUM_ROWS; i++){
        for(int j = 0; j < NUM_COLUMNS; j++){
            environment[j + NUM_COLUMNS * i] = (struct Cell){ WATER_TYPE, 0, j, i };
        }
    }
}

void simulation_rule_1(struct Cell environment[NUM_ROWS*NUM_COLUMNS]){

    struct Cell env_next[NUM_ROWS * NUM_COLUMNS];

    for(int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++){
        env_next[i] = environment[i];
    }

    // Water should drop to the cell below it unless a solid is under it 
    for(int i = 0; i < NUM_ROWS; i++){
        for(int j = 0; j < NUM_COLUMNS; j++){
            // Rule 1: Water flows down to neighboring cells 
            struct Cell src_cell = environment[j + NUM_COLUMNS * i];

            if(src_cell.type == WATER_TYPE && i < NUM_ROWS - 1){
                struct Cell dest_cell = environment[j + NUM_COLUMNS * (i + 1)];

                // How much liquid can flow into dest cell 
                // Only flows if dest cell has less liquid than source cell
                if(dest_cell.fill_level < src_cell.fill_level){
                    // How much liquid can fit in to dest cell
                    double free_space_dest = 1 - dest_cell.fill_level;
                    
                    if(free_space_dest >= src_cell.fill_level){
                        env_next[j + NUM_COLUMNS * i].fill_level = 0;
                        env_next[j + NUM_COLUMNS * (i + 1)].fill_level += src_cell.fill_level;
                    }
                    else{
                        env_next[j + NUM_COLUMNS * i].fill_level -= free_space_dest;
                        env_next[j + NUM_COLUMNS * (i + 1)].fill_level = 1;
                    }
                    
                }
            }
        }
    }

    for(int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++){
        environment[i] = env_next[i];     
    }
}

void simulation_rule_2(struct Cell environment[NUM_ROWS*NUM_COLUMNS]){

    struct Cell env_next[NUM_ROWS * NUM_COLUMNS];

    for(int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++){
        env_next[i] = environment[i];
    }

    for(int i = 0; i < NUM_ROWS; i++){
        for(int j = 0; j < NUM_COLUMNS; j++){
            // check if cell below is either full, solid, or bottom border
            struct Cell src_cell = environment[j + NUM_COLUMNS * i];
            if(i + 1 == NUM_ROWS || environment[j + NUM_COLUMNS * (i + 1)].fill_level > environment[j + NUM_COLUMNS * i].fill_level || environment[j + NUM_COLUMNS * (i + 1)].type == SOLID_TYPE){
                if(src_cell.type == WATER_TYPE && j > 0){
                    struct Cell dest_cell = environment[(j - 1) + NUM_COLUMNS * i];
                    if(dest_cell.fill_level < src_cell.fill_level && dest_cell.type == WATER_TYPE){
                        double d_fill = src_cell.fill_level - dest_cell.fill_level;
    
                        env_next[j + NUM_COLUMNS * i].fill_level -= d_fill / 3;
                        env_next[(j - 1) + NUM_COLUMNS * i].fill_level += d_fill / 3;
                    }
                }
                if(src_cell.type == WATER_TYPE && j < NUM_COLUMNS - 1){
                    struct Cell dest_cell = environment[(j + 1) + NUM_COLUMNS * i];
                    if(dest_cell.fill_level < src_cell.fill_level && dest_cell.type == WATER_TYPE){
                        double d_fill = src_cell.fill_level - dest_cell.fill_level;
    
                        env_next[j + NUM_COLUMNS * i].fill_level -= d_fill / 3;
                        env_next[(j + 1) + NUM_COLUMNS * i].fill_level += d_fill / 3;
                    }
                }
            }
        }
    }

    for(int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++){
        environment[i] = env_next[i];     
    }
}

/// @brief Takes a step in the simulation
/// @param environment The environment to simulate
void simulation_step(struct Cell environment[NUM_ROWS * NUM_COLUMNS]){

    // Rule 1: Water should fall down to the cell below it unless a solid is under it or the boundary is reached
    simulation_rule_1(environment);

    // Rule 2: Water should flow to the left or right if the cell below is full or solid and the cells to the left or right are not solid
    simulation_rule_2(environment);

    

    

    
}

/// @brief Simple fluid motion simulation with the ability to draw walls to contain the liquids
/// @brief Simulation based on: https://www.jgallant.com/2d-liquid-simulator-with-cellular-automaton-in-unity/ 
/// @return 
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Fluid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    draw_grid(surface);

    // Create the environment model
    struct Cell environment[NUM_ROWS * NUM_COLUMNS];
    init_environment(environment);

    int type = SOLID_TYPE;
    int delete = 0;
    int running = 1;
    SDL_Event event;
    while(running){
        while(SDL_PollEvent(&event)){ // Event loop to handle user input and exits
            if(event.type == SDL_QUIT){
                running = 0;
            }
            if(event.motion.type == SDL_MOUSEMOTION){ // If the left mouse button is pressed and moving
                if(event.motion.state & SDL_BUTTON_LMASK){
                    // Get the cell that the mouse is currently over
                    int cell_x = event.motion.x / CELL_SIZE;
                    int cell_y = event.motion.y / CELL_SIZE;
                    int fill_level = type ? 0 : 1;

                    if(delete){
                        type = WATER_TYPE;
                        fill_level = 0;
                    }

                    struct Cell cell = (struct Cell) { type, fill_level, cell_x, cell_y };

                    environment[cell_x + NUM_COLUMNS * cell_y] = cell;
                }
            }
            if(event.type == SDL_KEYDOWN){ // If the space key is pressed
               if(event.key.keysym.sym == SDLK_SPACE){
                    type = !type;
               } 

               if(event.key.keysym.sym == SDLK_BACKSPACE){
                    delete = !delete;
               }
            }
        }

        // handle the fluid simulation here 
        simulation_step(environment);

        draw_environment(surface, environment);
        draw_grid(surface);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(30);
    }
}