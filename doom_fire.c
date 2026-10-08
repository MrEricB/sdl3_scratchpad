#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// #define WIDTH 320
// #define HEIGHT 200
#define WIDTH 1280
#define HEIGHT 800


uint32_t framebuffer[WIDTH * HEIGHT];

/* Doom fire variables and color palette */
#define FIRE_W WIDTH
#define FIRE_H HEIGHT

#define  FIRE_PALETTE_SIZE 37

static uint8_t fire_pixels[FIRE_W * FIRE_H];

static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
  0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
  0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
  0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
  0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
  0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
  0xEFEFC7, 0xFFFFFF
};


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
    
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL init faild: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    // window = SDL_CreateWindow("Pixel Manipulation Framebuffer", WIDTH * 4, HEIGHT * 4, 0);
    window = SDL_CreateWindow("Pixel Manipulation Framebuffer", WIDTH, HEIGHT, 0);
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

    // 60 FPS
    const Uint64 FRAME_TIME = 1000000000ULL / 60;
    Uint64 next_frame = SDL_GetTicksNS();


    /* Fire initialization */
    for (int i = 0; i < FIRE_W * FIRE_H; i++) {
        fire_pixels[i] = 0;
    }

    /* Seed the fire bottom row with maximum intensity (this is the fire source) */
    for (int i = 0; i < FIRE_W; i++) {
        fire_pixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
    }

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
        /* Fire update */
        for (uint32_t x = 0; x < FIRE_W; x++) {
            for (uint32_t y = 1; y < FIRE_H; y++) {
                uint32_t src = FIRE_W * y + x;
                uint32_t rand_idx = rand() % 4;
                uint32_t dst = (uint32_t)src - FIRE_W;
                uint32_t dst_x = (uint32_t)(src % FIRE_W) - (uint32_t)rand_idx + 1;
                if (dst < 0 || dst_x < 0 || dst_x >= FIRE_W) {
                continue;
                } else {
                dst = dst - (uint32_t)(src % FIRE_W) + dst_x;
                uint32_t src_val = fire_pixels[src];
                uint32_t decay = rand_idx & 1;
                fire_pixels[dst] = (src_val > decay) ? (src_val - decay) : 0;
                }
            }
        }

        /* Fire render */
        for (uint32_t y = 0; y < FIRE_H; y++) {
            for (uint32_t x = 0; x < FIRE_W; x++) {
                uint8_t idx = fire_pixels[y * FIRE_W + x];
                put_pixel(x, y, fire_palette[idx]);
            }
        }


        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        // display the window and the renderer
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);


        // 60 FPS
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