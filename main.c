#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define HIST_W 400
#define HIST_H 300
#define BTN_W 120
#define BTN_H 40

typedef struct {
    SDL_FRect rect;
    bool hovered;
    bool clicked;
    const char *label;
} Button;

void convertToGrayscale(SDL_Surface *surface) {
    if (!surface) return;
    SDL_LockSurface(surface);

    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);
    Uint8 r, g, b;
    Uint32 *pixels = (Uint32 *)surface->pixels;

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = pixels[y * surface->w + x];
            SDL_GetRGB(pixel, fmt, NULL, &r, &g, &b);
            Uint8 gray = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);
            pixels[y * surface->w + x] = SDL_MapRGB(fmt, NULL, gray, gray, gray);
        }
    }

    SDL_UnlockSurface(surface);
}

void computeHistogram(SDL_Surface *surface, int hist[256], double *media, double *desvio) {
    for (int i = 0; i < 256; i++) hist[i] = 0;

    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int total = surface->w * surface->h;
    Uint8 r, g, b;

    for (int i = 0; i < total; i++) {
        SDL_GetRGB(pixels[i], fmt, NULL, &r, &g, &b);
        hist[r]++;
    }

    double soma = 0;
    for (int i = 0; i < 256; i++) soma += i * hist[i];
    *media = soma / total;

    double soma2 = 0;
    for (int i = 0; i < 256; i++) soma2 += (i - *media) * (i - *media) * hist[i];
    *desvio = sqrt(soma2 / total);
}

SDL_Surface* equalizeHistogram(SDL_Surface *src) {
    int hist[256] = {0}, cdf[256];
    const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(src->format);

    Uint32 *pixels = (Uint32 *)src->pixels;
    int total = src->w * src->h;
    Uint8 r, g, b;

    for (int i = 0; i < total; i++) {
        SDL_GetRGB(pixels[i], fmt, NULL, &r, &g, &b);
        hist[r]++;
    }

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) cdf[i] = cdf[i - 1] + hist[i];

    SDL_Surface *dst = SDL_ConvertSurface(src, src->format);
    Uint32 *pixOut = (Uint32 *)dst->pixels;
    int cdf_min = 0;
    for (int i = 0; i < 256; i++) if (cdf[i] > 0) { cdf_min = cdf[i]; break; }

    for (int i = 0; i < total; i++) {
        SDL_GetRGB(pixels[i], fmt, NULL, &r, &g, &b);
        int newVal = round(((double)(cdf[r] - cdf_min) / (total - cdf_min)) * 255.0);
        pixOut[i] = SDL_MapRGB(fmt, NULL, newVal, newVal, newVal);
    }

    return dst;
}

void drawHistogram(SDL_Renderer *ren, int hist[256]) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    int maxVal = 1;
    for (int i = 0; i < 256; i++) if (hist[i] > maxVal) maxVal = hist[i];

    for (int i = 0; i < 256; i++) {
        int h = (hist[i] * (HIST_H - 100)) / maxVal;
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderLine(ren, i * HIST_W / 256, HIST_H, i * HIST_W / 256, HIST_H - h);
    }
}

void renderButton(SDL_Renderer *ren, Button *btn, TTF_Font *font) {
    if (btn->clicked)         SDL_SetRenderDrawColor(ren, 0, 0, 180, 255);
    else if (btn->hovered)    SDL_SetRenderDrawColor(ren, 100, 149, 237, 255);
    else                      SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);

    SDL_RenderFillRect(ren, &btn->rect);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *textSurf = TTF_RenderText_Blended(font, btn->label, strlen(btn->label), white);
    SDL_Texture *textTex = SDL_CreateTextureFromSurface(ren, textSurf);
    SDL_FRect dst = {btn->rect.x + (btn->rect.w - textSurf->w)/2.0f,
                     btn->rect.y + (btn->rect.h - textSurf->h)/2.0f,
                     (float)textSurf->w, (float)textSurf->h};
    SDL_RenderTexture(ren, textTex, NULL, &dst);
    SDL_DestroyTexture(textTex);
    SDL_DestroySurface(textSurf);
}

void renderClassification(SDL_Renderer *ren, TTF_Font *font, double media, double desvio) {
    const char *luminosidade;
    if (media < 85) luminosidade = "Imagem escura";
    else if (media < 170) luminosidade = "Imagem media";
    else luminosidade = "Imagem clara";

    const char *contraste;
    if (desvio < 50) contraste = "Contraste baixo";
    else if (desvio < 100) contraste = "Contraste medio";
    else contraste = "Contraste alto";

    SDL_Color yellow = {255, 255, 0, 255};

    SDL_Surface *surf1 = TTF_RenderText_Blended(font, luminosidade, strlen(luminosidade), yellow);
    SDL_Texture *tex1 = SDL_CreateTextureFromSurface(ren, surf1);
    SDL_FRect dst1 = {10, HIST_H - 80, (float)surf1->w, (float)surf1->h};
    SDL_RenderTexture(ren, tex1, NULL, &dst1);

    SDL_Surface *surf2 = TTF_RenderText_Blended(font, contraste, strlen(contraste), yellow);
    SDL_Texture *tex2 = SDL_CreateTextureFromSurface(ren, surf2);
    SDL_FRect dst2 = {10, HIST_H - 50, (float)surf2->w, (float)surf2->h};
    SDL_RenderTexture(ren, tex2, NULL, &dst2);

    SDL_DestroyTexture(tex1);
    SDL_DestroySurface(surf1);
    SDL_DestroyTexture(tex2);
    SDL_DestroySurface(surf2);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s caminho_da_imagem\n", argv[0]);
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Surface *image = IMG_Load(argv[1]);
    if (!image) {
        printf("Erro ao carregar imagem: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface *gray = SDL_ConvertSurface(image, image->format);
    convertToGrayscale(gray);
    SDL_Surface *equalized = equalizeHistogram(gray);

    SDL_Window *win_main = SDL_CreateWindow("Imagem", gray->w, gray->h, 0);
    int main_x, main_y, main_w, main_h;
    SDL_GetWindowPosition(win_main, &main_x, &main_y);
    SDL_GetWindowSize(win_main, &main_w, &main_h);

    SDL_Window *win_hist = SDL_CreateWindow("Histograma", HIST_W, HIST_H, 0);
    SDL_SetWindowPosition(win_hist, main_x + main_w, main_y);

    SDL_Renderer *ren_main = SDL_CreateRenderer(win_main, NULL);
    SDL_Renderer *ren_hist = SDL_CreateRenderer(win_hist, NULL);

    SDL_Texture *tex_main = SDL_CreateTextureFromSurface(ren_main, gray);

    int hist[256]; double media, desvio;
    computeHistogram(gray, hist, &media, &desvio);

    Button btn = {{(HIST_W - BTN_W)/2.0f, 10, BTN_W, BTN_H}, false, false, "Equalizar"};
    TTF_Font *font = TTF_OpenFont("assets/arial.ttf", 16);
    if (!font) {
        printf("Erro ao carregar fonte: %s\n", SDL_GetError());
        return 1;
    }

    bool running = true, isEqualized = false;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;

            if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                Uint32 winID = e.window.windowID;
                if (winID == SDL_GetWindowID(win_main) || winID == SDL_GetWindowID(win_hist)) {
                    running = false;
                }
            }

            if (e.type == SDL_EVENT_MOUSE_MOTION && e.window.windowID == SDL_GetWindowID(win_hist)) {
                float mx = e.motion.x, my = e.motion.y;
                btn.hovered = (mx >= btn.rect.x && mx <= btn.rect.x+btn.rect.w &&
                               my >= btn.rect.y && my <= btn.rect.y+btn.rect.h);
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && btn.hovered) {
                btn.clicked = true;
            }
            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && btn.clicked) {
                btn.clicked = false;
                isEqualized = !isEqualized;
                if (isEqualized) {
                    SDL_DestroyTexture(tex_main);
                    tex_main = SDL_CreateTextureFromSurface(ren_main, equalized);
                    computeHistogram(equalized, hist, &media, &desvio);
                    btn.label = "Original";
                } else {
                    SDL_DestroyTexture(tex_main);
                    tex_main = SDL_CreateTextureFromSurface(ren_main, gray);
                    computeHistogram(gray, hist, &media, &desvio);
                    btn.label = "Equalizar";
                }
            }

            // tecla S para salvar
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_S) {
                SDL_Surface *toSave = isEqualized ? equalized : gray;
                IMG_SavePNG(toSave, "output_image.png");
                printf("Imagem salva como output_image.png\n");
            }
        }

        SDL_RenderClear(ren_main);
        SDL_RenderTexture(ren_main, tex_main, NULL, NULL);
        SDL_RenderPresent(ren_main);

        drawHistogram(ren_hist, hist);
        renderButton(ren_hist, &btn, font);
        renderClassification(ren_hist, font, media, desvio);
        SDL_RenderPresent(ren_hist);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(tex_main);
    SDL_DestroyRenderer(ren_main);
    SDL_DestroyRenderer(ren_hist);
    SDL_DestroyWindow(win_main);
    SDL_DestroyWindow(win_hist);
    SDL_DestroySurface(image);
    SDL_DestroySurface(gray);
    SDL_DestroySurface(equalized);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
