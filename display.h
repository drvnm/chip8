#ifndef H_DISPLAY
#define H_DISPLAY

#include <iostream>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

// acts as a wrapper for SDL_Window, and the display of the chip8 emulator
class Display
{
    private: 
        SDL_Window *window;
        SDL_Renderer *renderer;
        bool display[SCREEN_WIDTH][SCREEN_HEIGHT] = {false};

    public:
        Display();
        ~Display();
        void update();
        void clear();
        void setPixel(int x, int y, bool on);
        // true if there is a pixel at x,y, false otherwise
        bool getPixel(int x, int y);

};

#endif