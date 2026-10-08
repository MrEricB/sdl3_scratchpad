#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 200



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

int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event; // just a struct not a pointer like the ones above
    
    // const double target_frame = 1.0 / 60.0;

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

        }

        clear(0xFFFFFF);
        // here we can manipulate our frambuffer arary as we wish
        int x = frame % WIDTH;
        int y = HEIGHT/2;
        put_pixel(x, y, 0x000055);


        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        // display the window and the renderer
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // uint64_t end = SDL_GetPerformanceCounter();
        // double elapsed = (double)(end - start) / (double) SDL_GetPerformanceFrequency();

        //cap fps
        // if (elapsed < target_frame)
        // {
        //     SDL_Delay((target_frame - elapsed) * 1000); // sleep until reach target for 60 fps
        // }

        // frame++;

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