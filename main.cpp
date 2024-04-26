
#include <iostream>
#include <fstream>
#include <vector>
#include <SDL2/SDL.h>

#include "display.h"
#include "emulator.h"

int main(int argc, char *argv[])
{

    // get rom from command line
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <rom file>" << std::endl;
        return 1;
    }

    // read rom file
    std::string rom_file = argv[1];
    std::ifstream file(rom_file, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << rom_file << std::endl;
        return 1;
    }

    // read rom file into memory
    std::vector<uint8_t> rom;   
    while (!file.eof())
    {
        uint8_t byte;
        file.read(reinterpret_cast<char *>(&byte), sizeof(byte));
        rom.push_back(byte);
    }

    if (rom.size() > 4096)
    {
        std::cerr << "Error: ROM file too large" << std::endl;
        return 1;
    }

    // create display

    Emulator emulator(rom);
    emulator.cycle();

    // Display display;
    // display.clear();
    // display.setPixel(10, 10, true);
    // display.update();

    // bool running = true;
    // SDL_Event event;
    // while (running)
    // {
    //     while (SDL_PollEvent(&event))
    //     {
    //         if (event.type == SDL_QUIT)
    //         {
    //             running = false;
    //         }
    //     }
    // }

    return 0;
}