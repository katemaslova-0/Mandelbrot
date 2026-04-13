#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <xmmintrin.h>

const float ROI_X = -1.325,
            ROI_Y = 0;

struct Picture
{
    SDL_Window * window     = NULL;
    SDL_Renderer * renderer = NULL;
    SDL_Texture * texture   = NULL;
    Uint32 * pixels         = NULL;
};

Picture PicInit            (void);
uint64_t CalculateOpt      (void);
uint64_t CalculateNoOpt    (void);
uint64_t CalculateArrayOpt (void);
void PicDestroy     (Picture picture);
void PicUpdate      (Picture picture);
void DrawPicture    (Picture picture);
void PicStorePixels (Picture picture, __m256i N, int iy, int ix);

int main (void)
{
    Picture picture = PicInit();
    
    //uint64_t result_noopt = CalculateNoOpt();
    //printf("%ld ticks (no opt)\n", result_noopt);

    uint64_t result_arrayopt = CalculateArrayOpt();
    printf("%ld ticks (array opt)\n", result_arrayopt);

    //uint64_t result_opt = CalculateOpt();
    //printf("%ld ticks (opt)\n", result_opt);
    
    DrawPicture(picture);
    PicDestroy(picture);
    
    return 0;
}

void DrawPicture(Picture picture)
{
    const int nMax = 256;
    const float dx = 1/800.f, dy = 1/800.f;
    const float xC = 0.f, yC = 0.f, scale = 1.f;

    __m256 r2Max = {100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f}; // change to set!
    __m256 _3210 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

    for (int iy = 0; iy < 600; iy++)
    {
        float x0 = ((          - 400.f) * dx + ROI_X + xC) * scale,
            y0 = (((float)iy - 300.f) * dy + ROI_Y + yC) * scale;

        for (int ix = 0; ix < 800; ix += 8, x0 += dx * 8)
        {
            __m256 DX = {}; DX = _mm256_set1_ps(dx); DX = _mm256_mul_ps(DX, _3210);

            __m256 X0 = {}; X0 = _mm256_set1_ps(x0); X0 = _mm256_add_ps(X0, DX);
            __m256 Y0 = {}; Y0 = _mm256_set1_ps(y0); 

            __m256 X = {}; X = _mm256_add_ps(X, X0);
            __m256 Y = {}; Y = _mm256_add_ps(Y, Y0);

            __m256i N = {};

            for (int n = 0; n < nMax; n++)
            {
                __m256 x2 = {}; x2 = _mm256_mul_ps(X, X);
                __m256 y2 = {}; y2 = _mm256_mul_ps(Y, Y);
                __m256 xy = {}; xy = _mm256_mul_ps(X, Y);

                __m256 r2 = {}; r2 = _mm256_add_ps(x2, y2);

                __m256 cmp = {}; cmp = _mm256_cmp_ps(r2, r2Max, _CMP_LE_OQ);

                int mask = _mm256_movemask_ps(cmp);
                if (!mask) break;

                __m256i cmp_int = _mm256_castps_si256(cmp);
                N = _mm256_add_epi32(N, cmp_int);

                X = _mm256_sub_ps(x2, y2); X = _mm256_add_ps(X, X0);
                Y = _mm256_add_ps(xy, xy); Y = _mm256_add_ps(Y, Y0);
            }

            PicStorePixels(picture, N, iy, ix);
        }
    }

    PicUpdate(picture);

    int c = 0;
    while ((c = getchar()) != '\n')
        continue;
}

uint64_t CalculateNoOpt (void)
{
    const int nMax = 256;
    const float dx = 1/800.f, dy = 1/800.f;
    const float r2Max = 100.f;
    const float xC = 0.f, yC = 0.f, scale = 1.f;

    int num_of_cycles = 100;
    int res = 0;

    uint64_t start = __rdtsc();
    volatile int result = 0;

    for (; res < num_of_cycles; res++)
    {
        for (int iy = 0; iy < 600; iy++)
        {
            float x0 = ((          - 400.f) * dx + ROI_X + xC) * scale,
                  y0 = (((float)iy - 300.f) * dy + ROI_Y + yC) * scale;

            for (int ix = 0; ix < 800; ix++, x0 += dx)
            {
                float X = x0,
                      Y = y0;

                int N = 0;

                for (; N < nMax; N++)
                {
                    float x2 = X * X,
                          y2 = Y * Y,
                          xy = X * Y;

                    float r2 = x2 + y2;

                    if (r2 >= r2Max)
                        break;
                    X = x2 - y2 + x0;
                    Y = xy + xy + y0;
                }

                result += N;
            }

        }
    }

    uint64_t end = __rdtsc();

    return (end - start) / num_of_cycles;
}


uint64_t CalculateArrayOpt (void)
{
    const int nMax = 256;
    const float dx = 1/800.f, dy = 1/800.f;
    const float r2Max = 100.f;
    const float xC = 0.f, yC = 0.f, scale = 1.f;

    int num_of_cycles = 100;
    int res = 0;

    uint64_t start = __rdtsc();

    volatile int result[4] = {};

    for (; res < num_of_cycles; res++)
    {
        float xC = 0.f, yC = 0.f, scale = 1.f;

        for (int iy = 0; iy < 600; iy++)
        {
            float x0 = ((          - 400.f) * dx + ROI_X + xC) * scale,
                  y0 = (((float)iy - 300.f) * dy + ROI_Y + yC) * scale;

            for (int ix = 0; ix < 800; ix += 4, x0 += dx * 4)
            {
                float X0[4] = {x0, x0 + dx, x0 + dx*2, x0 + dx*3};
                float Y0[4] = {y0, y0,      y0,        y0};

                float X[4] = {}; for (int i = 0; i < 4; i++) X[i] = X0[i];
                float Y[4] = {}; for (int i = 0; i < 4; i++) Y[i] = Y0[i];

                int N[4] = {0, 0, 0, 0};

                int n = 0;
                for (; n < nMax; n++)
                {
                    float x2[4] = {}; for (int i = 0; i < 4; i++) x2[i] = X[i] * X[i];
                    float y2[4] = {}; for (int i = 0; i < 4; i++) y2[i] = Y[i] * Y[i];
                    float xy[4] = {}; for (int i = 0; i < 4; i++) xy[i] = X[i] * Y[i];

                    float r2[4] = {}; for (int i = 0; i < 4; i++) r2[i] = x2[i] + y2[i];

                    int cmp[4] = {};
                    for (int i = 0; i < 4; i++) if (r2[i] <= r2Max) cmp[i] = 1;

                    int mask = 0;
                    for (int i = 0; i < 4; i++) mask |= (cmp[i] << i);
                    if (!mask) break;

                    for (int i = 0; i < 4; i++) N[i] = N[i] + cmp[i];

                    for (int i = 0; i < 4; i++) X[i] = x2[i] - y2[i] + X0[i];
                    for (int i = 0; i < 4; i++) Y[i] = xy[i] + xy[i] + Y0[i];
                }

                for (int i = 0; i < 4; i++) result[i] += N[i];
            }
        }
    }

    uint64_t end = __rdtsc();

    return (end - start) / num_of_cycles;
}

uint64_t CalculateOpt (void)
{
    const int nMax = 256;
    const float dx = 1/800.f, dy = 1/800.f;
    const float xC = 0.f, yC = 0.f, scale = 1.f;

    __m256 r2Max = {100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 100.f}; // change to set!
    __m256 _3210 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

    int num_of_cycles = 100;
    int res = 0;

    uint64_t start = __rdtsc();

    volatile int result = 0;

    for (; res < num_of_cycles; res++)
    {
        for (int iy = 0; iy < 600; iy++)
        {
            float x0 = ((          - 400.f) * dx + ROI_X + xC) * scale,
                  y0 = (((float)iy - 300.f) * dy + ROI_Y + yC) * scale;

            for (int ix = 0; ix < 800; ix += 8, x0 += dx * 8)
            {
                __m256 DX = {}; DX = _mm256_set1_ps(dx); DX = _mm256_mul_ps(DX, _3210);

                __m256 X0 = {}; X0 = _mm256_set1_ps(x0); X0 = _mm256_add_ps(X0, DX);
                __m256 Y0 = {}; Y0 = _mm256_set1_ps(y0); 

                __m256 X = {}; X = _mm256_add_ps(X, X0);
                __m256 Y = {}; Y = _mm256_add_ps(Y, Y0);

                __m256i N = {};

                for (int n = 0; n < nMax; n++)
                {
                    __m256 x2 = {}; x2 = _mm256_mul_ps(X, X);
                    __m256 y2 = {}; y2 = _mm256_mul_ps(Y, Y);
                    __m256 xy = {}; xy = _mm256_mul_ps(X, Y);

                    __m256 r2 = {}; r2 = _mm256_add_ps(x2, y2);

                    __m256 cmp = {}; cmp = _mm256_cmp_ps(r2, r2Max, _CMP_LE_OQ);

                    int mask = _mm256_movemask_ps(cmp);
                    if (!mask) break;

                    __m256i cmp_int = _mm256_castps_si256(cmp);
                    N = _mm256_add_epi32(N, cmp_int);

                    X = _mm256_sub_ps(x2, y2); X = _mm256_add_ps(X, X0);
                    Y = _mm256_add_ps(xy, xy); Y = _mm256_add_ps(Y, Y0);
                }

                result += _mm256_extract_epi32(N, 0);
            }
        }
    }

    uint64_t end = __rdtsc();

    return (end - start) / num_of_cycles;
}

void PicStorePixels (Picture picture, __m256i N, int iy, int ix)
{
    alignas(32) int N_vals[8] = {};
    _mm256_store_si256((__m256i*)N_vals, N);

    for (int i = 0; i < 8; i++)
    {
        Uint32 color_value = N_vals[i] % 2 * 255;
        Uint32 pixel_color = (color_value << 24) |
                             (color_value << 16) |
                             (color_value << 8)  | 
                              color_value;
        picture.pixels[iy * 800 + ix + i] = pixel_color;
    }
}

void PicDestroy (Picture picture)
{
    free(picture.pixels);
    SDL_DestroyTexture(picture.texture);
    SDL_DestroyRenderer(picture.renderer);
    SDL_DestroyWindow(picture.window);
    SDL_Quit();
}

void PicUpdate (Picture picture)
{
    SDL_UpdateTexture(picture.texture, NULL, picture.pixels, 800 * sizeof(Uint32));
    SDL_RenderClear(picture.renderer);
    SDL_RenderCopy(picture.renderer, picture.texture, NULL, NULL);
    SDL_RenderPresent(picture.renderer);
}

Picture PicInit (void)
{
    Picture picture = {};

    int result = SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN, &picture.window, &picture.renderer);
    if (result < 0)
        printf("Error creating window and render\n");

    picture.texture = SDL_CreateTexture(picture.renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        800, 600);

    picture.pixels = (Uint32 * ) calloc(800 * 600, sizeof(Uint32));
    if (!picture.pixels)
        printf("Error allocating pixel buffer\n");

    return picture;
}