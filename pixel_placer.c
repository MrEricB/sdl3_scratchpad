#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 200

#define MAX_PIXELS 1000

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

typedef struct
{
    int x;
    int y;

    int dx;
    int dy;
} MovingPixel;


int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event; // just a struct not a pointer like the ones above
    
    MovingPixel pixels[MAX_PIXELS];
    int pixel_count = 0;


    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL init faild: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    window = SDL_CreateWindow("Pixel Manipulation Framebuffer", WIDTH * 4, HEIGHT * 4, 0);
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

    int pixel_x = -1;
    int pixel_y = -1;

    const Uint64 FRAME_TIME = 1000000000ULL / 60;
    Uint64 next_frame = SDL_GetTicksNS();

    while (is_running)
    {
        // uint64_t start = SDL_GetPerformanceCounter();
        next_frame += FRAME_TIME;
        // poll events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                is_running = 0;
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                pixel_x = (int)event.button.x / 4;
                pixel_y = (int)event.button.y / 4;

                if (pixel_count < MAX_PIXELS)
                {
                    pixels[pixel_count].x = pixel_x;
                    pixels[pixel_count].y = pixel_y;

                    pixels[pixel_count].dx = 1;
                    pixels[pixel_count].dy = 0;

                    pixel_count++;
                }
            }

        }

        for (int i = 0; i < pixel_count; i++)
        {
            if (pixels[i].x > WIDTH)
            {
                pixels[i].x = -1;
                continue;
            } 
            pixels[i].x += pixels[i].dx;
            pixels[i].y += pixels[i].dy;
        }

        clear(0xFFFFFF);
        // here we can manipulate our frambuffer arary as we wish
        
        //put pixel at mouse click location
        for (int i = 0; i < pixel_count; i++)
        {
            put_pixel(
                pixels[i].x,
                pixels[i].y,
                0x000000
            );
        }

        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        // display the window and the renderer
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);


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