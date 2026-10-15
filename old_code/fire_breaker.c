#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WIDTH 320
#define HEIGHT 200

#define MAX_PIXELS 1500

uint32_t framebuffer[WIDTH * HEIGHT];

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
    float x;
    float y;

    float init_x;
    float init_y;

    float dx;
    float dy;

    uint32_t color;

} ExplodingPixel;


int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    
    ExplodingPixel pixels[MAX_PIXELS];
    int pixel_count = 0;

    srand((unsigned int)time(NULL));

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

    // click pixel inial location
    float pixel_x = -1.0f;
    float pixel_y = -1.0f;

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

                int pixel_amount = (rand() % 46) + 5;

                pixel_x = (int)event.button.x / 4;
                pixel_y = (int)event.button.y / 4;
                
                for(int i = 0; i < pixel_amount; i++)
                {
                    if (pixel_count < MAX_PIXELS)
                    {
                        pixels[pixel_count].x = pixel_x;
                        pixels[pixel_count].y = pixel_y;
                        
                        pixels[pixel_count].init_x = pixel_x;
                        pixels[pixel_count].init_y = pixel_y;

                        pixels[pixel_count].color = random_color();
    
                        int x_direction = (rand() % 2 == 0) ? -1 : 1;
                        int y_direction = (rand() % 2 == 0) ? -1 : 1;
    
    
                        pixels[pixel_count].dx = ((rand() % 19 + 2) * 0.05) * x_direction;
                        pixels[pixel_count].dy = ((rand() % 19 + 2) * 0.05) * y_direction;
    
                        pixel_count++;
                    }
                }
            }

        }


        // here can manipulate our frambuffer array
        
        for (int i = 0; i < pixel_count; i++)
        {
            // if (pixels[i].x > WIDTH)
            // {
            //     pixels[i].x -= WIDTH;
            // }
            // else if (pixels[i].x < 0)
            // {
            //     pixels[i].x += WIDTH;
            // }
            // else
            // {
            //     pixels[i].x += pixels[i].dx;
            // }

            // if (pixels[i].y > HEIGHT)
            // {
            //     pixels[i].y -= HEIGHT;
            // }
            // else if (pixels[i].y < 0)
            // {
            //     pixels[i].y += HEIGHT;
            // }
            // else
            // {
            //     pixels[i].y += pixels[i].dy;
            // }

            /*
            No longer need wrap around.
            Now we want to have the particles expand outward for some amount of time,
                then the particles revers until they reach there original loaction and they are resported,
                then delete the pixels (do I even need to do the deletion?)
            */



        }

        
        /* Fire update */
        // for (int32_t x = 0; x < FIRE_W; x++) {
        //     for (int32_t y = 1; y < FIRE_H; y++) {
        //         int32_t src = FIRE_W * y + x;
        //         int32_t rand_idx = rand() % 4;
        //         int32_t dst = (int32_t)src - FIRE_W;
        //         int32_t dst_x = (int32_t)(src % FIRE_W) - (int32_t)rand_idx + 1;
        //         if (dst < 0 || dst_x < 0 || dst_x >= FIRE_W) {
        //         continue;
        //         } else {
        //         dst = dst - (int32_t)(src % FIRE_W) + dst_x;
        //         int32_t src_val = fire_pixels[src];
        //         int32_t decay = rand_idx & 1;
        //         fire_pixels[dst] = (src_val > decay) ? (src_val - decay) : 0;
        //         }
        //     }
        // }

        // /* Fire render */
        // for (int32_t y = 0; y < FIRE_H; y++) {
        //     for (int32_t x = 0; x < FIRE_W; x++) {
        //         int8_t idx = fire_pixels[y * FIRE_W + x];
        //         put_pixel(x, y, fire_palette[idx]);
        //     }
        // }



        //put pixel at mouse click location
        for (int i = 0; i < pixel_count; i++)
        {
            put_pixel(
                pixels[i].x,
                pixels[i].y,
                pixels[i].color
            );
        }

        // copy the contents of the frambuffer to the texture.
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t)); // pitch --> width of the image/texture in bytes.

        // clear the renderer current target
        SDL_RenderClear(renderer);

        // render the frambuffer
        SDL_RenderTexture(renderer, texture, NULL, NULL);

        // render pixel count on top
        draw_pixel_count(renderer, pixel_count);

        //display frame
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