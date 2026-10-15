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

uint32_t framebuffer[WIDTH * HEIGHT];

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


int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    int pixel_count = 0;

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
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    uint8_t is_running = 1;
    uint32_t frame = 0;

    const Uint64 FRAME_TIME = 1000000000ULL / 60;
    Uint64 next_frame = SDL_GetTicksNS();


    Circle circle;
    circle.growth_speed = 0.0f;
    circle.current_radius = 0;
    circle.is_growing = true;
   
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
                circle.center_x = (int) event.button.x; // 4;
                circle.center_y = (int) event.button.y; // 4;

                circle.current_radius = 0;
                circle.growth_speed = 1.5f;
                circle.base_growth_speed = circle.growth_speed;
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
            {
                circle.growth_speed *= -1.0f;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
            {
                circle.is_growing = !circle.is_growing;
            }

        }

        // get new radious
        if (circle.is_growing)
        {
            circle.current_radius += circle.growth_speed;
        }

        // check if circle is growing and touched an edge.
        if (circle.growth_speed > 0.0f)
        {
            if (circle.center_x + circle.current_radius >= WIDTH ||
                circle.center_x - circle.current_radius <= 0 ||
                circle.center_y + circle.current_radius >= HEIGHT ||
                circle.center_y - circle.current_radius <= 0)
                {
                    circle.growth_speed *= -1.0f;     
                }
        }

        if (circle.current_radius <= 0.0f && circle.growth_speed < 0.0f)
        {
            circle.current_radius = 0.0f;
            circle.growth_speed *= -1.0f;
        }

        clear(0x2A2A2A);

        // here can manipulate our frambuffer array
        //draw the cirlce
        // if (!circle.is_growing)
        // {
        //     circle.growth_speed = 0;
        // }
        // else
        // {
        //     circle.growth_speed = circle.base_growth_speed;
        // }


        for(int i = 0; i <= 720; i++)
        {
            float angle = i * (2.0f * 3.14159265f / 720.0f);

            x_boarder = circle.center_x + circle.current_radius * cos(angle);
            y_boarder = circle.center_y + circle.current_radius * sin(angle);

            put_pixel((int)x_boarder, (int)y_boarder, 0xFF0000);

        }
        


        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        
        SDL_RenderClear(renderer); // clear the renderer current target
        SDL_RenderTexture(renderer, texture, NULL, NULL); // render the frambuffer
        // draw_pixel_count(renderer, pixel_count); // render pixel count on top
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