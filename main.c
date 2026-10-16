#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define WIDTH 640
#define HEIGHT 400

#define MAX_PIXELS 1500
#define MAX_CIRCLES 100

uint32_t framebuffer[WIDTH * HEIGHT];


void draw_instructions(SDL_Renderer *renderer)
{
    // Draw a dark background behind the instructions
    SDL_FRect header = {
        0.0f, 0.0f, (float)WIDTH, 60.0f
    };

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &header);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    SDL_RenderDebugText(renderer, 10, 8, "Right click: Place growing circle | Left click: Reverse circles");

    SDL_RenderDebugText(renderer, 10, 24, "Space: Toggle growing/paused | D: Delete random circle");

    SDL_RenderDebugText(renderer, 10, 40, "C: Clear all circles");
}


void put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
    {
        return;
    }

    framebuffer[WIDTH * y + x] = color;
}

void clear(uint32_t color)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = color;
    }
}

void draw_pixel_count(SDL_Renderer *renderer, int pixel_count)
{
    char text[64];

    snprintf(text, sizeof(text), "Pixels: %d", pixel_count);

    // Debug font characters are 8 pixels wide
    float text_width = strlen(text) * 8.0f;

    int screen_width;
    int screen_height;

    if (!SDL_GetCurrentRenderOutputSize(renderer, &screen_width, &screen_height))
        return;

    // Position text 10 pixels from the right edge
    float x = screen_width - text_width - 10;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDebugText(renderer, x, 10, text);
}

uint32_t random_color(void)
{
    uint8_t r = rand() % 156;  // 0–155
    uint8_t g = rand() % 156;
    uint8_t b = rand() % 156;

    return (r << 16) | (g << 8) | b;
}

typedef struct
{
    int center_x;
    int center_y;
    float growth_speed;
    float base_growth_speed;
    float current_radius;
    bool is_growing;

} Circle;

int circle_count = 0;

Circle circles[MAX_CIRCLES] = {0};

bool add_circle(int x, int y)
{
    if (circle_count >= MAX_CIRCLES)
    {
        return false;
    }
    // insert circle at end of array
    circles[circle_count].center_x = x;
    circles[circle_count].center_y = y;
    circles[circle_count].growth_speed = ((rand() % 13) * .25) + 0.5; // random number between 0.5 and 3.5
    circles[circle_count].base_growth_speed = circles[circle_count].growth_speed;
    circles[circle_count].current_radius = 0;
    circles[circle_count].is_growing = true;

    circle_count++;

    return true;
}

bool remove_circle_at_index(int index)
{
    if (index < 0 || index >= circle_count)
    {
        return false;
    }

    // Shift remaining circles left
    for (int i = index; i < circle_count - 1; i++)
    {
        circles[i] = circles[i + 1];
    }

    circle_count--;

    circles[circle_count] = (Circle){0};

    return true;
}

bool clear_all_circles(void)
{
    if (circle_count == 0)
    {
        return false;
    }

    circle_count = 0;

    return true;
}

int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    srand((unsigned int)time(NULL));

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL init faild: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    window = SDL_CreateWindow("Pixel Manipulation Framebuffer", WIDTH, HEIGHT, 0);
    // window = SDL_CreateWindow("Pixel Manipulation Framebuffer", WIDTH * 4, HEIGHT *v4, 0);
    if(window == NULL){
        fprintf(stderr, "SDL_CreateWindow faild: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == NULL){
        fprintf(stderr, "SDL_CreateRenderer faild: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if(texture == NULL){
        fprintf(stderr, "SDL_CreateTexture faild: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    

    uint8_t is_running = 1;
    uint32_t frame = 0;

    const Uint64 FRAME_TIME = 1000000000ULL / 60;
    Uint64 next_frame = SDL_GetTicksNS();

    float x_boarder;
    float y_boarder;


    while (is_running)
    {
        next_frame += FRAME_TIME;
        
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                is_running = 0;
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                add_circle((int) event.button.x, (int) event.button.y);
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
            {
                // reverse all circle (for now)
                for (int i = 0; i <= circle_count - 1; i++)
                {
                    circles[i].growth_speed *= -1.0f;
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE && !event.key.repeat)
            {
                //puase all circle (for now)
                for (int i = 0; i <= circle_count - 1; i++)
                {
                    circles[i].is_growing = !circles[i].is_growing;   
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_D && !event.key.repeat)
            {
                // generate random index and remove that circle
                int remove_index = (rand() % circle_count);
                remove_circle_at_index(remove_index);
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_C && !event.key.repeat)
            {
                clear_all_circles();
            }
        }


        for (int i = 0; i <= circle_count - 1; i++)
        {
            // get new radious
            if (circles[i].is_growing)
            {
                circles[i].current_radius += circles[i].growth_speed;
            }

            // check if circle is growing and touched an edge.
            if (circles[i].growth_speed > 0.0f)
            {
                if (circles[i].center_x + circles[i].current_radius >= WIDTH ||
                    circles[i].center_x - circles[i].current_radius <= 0 ||
                    circles[i].center_y + circles[i].current_radius >= HEIGHT ||
                    circles[i].center_y - circles[i].current_radius <= 0)
                    {
                        circles[i].growth_speed *= -1.0f;     
                    }
            }
    
            if (circles[i].current_radius <= 0.0f && circles[i].growth_speed < 0.0f)
            {
                circles[i].current_radius = 0.0f;
                circles[i].growth_speed *= -1.0f;
            }
        }


        clear(0x2A2A2A);

        // here can manipulate our frambuffer array
        //draw the cirlces
        for (int i = 0; i <= circle_count - 1; i++)
        {
            for(int j = 0; j < 720; j++)
            {
                float angle = j * (2.0f * 3.14159265f / 720.0f);
    
                x_boarder = circles[i].center_x + circles[i].current_radius * cos(angle);
                y_boarder = circles[i].center_y + circles[i].current_radius * sin(angle);
    
                put_pixel((int)x_boarder, (int)y_boarder, 0xFF0000);
    
            }
        }
        


        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        
        SDL_RenderClear(renderer); // clear the renderer current target
        SDL_RenderTexture(renderer, texture, NULL, NULL); // render the frambuffer (i.e. draw the circles)
        // draw_instructions(renderer);
        SDL_RenderPresent(renderer); //display frame


        frame++;
        Uint64 now = SDL_GetTicksNS();

        if (now < next_frame)
        {
            SDL_DelayPrecise(next_frame - now);
        }
        else if (now > next_frame + FRAME_TIME)
        {
            next_frame = now;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}