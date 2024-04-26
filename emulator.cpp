#include "Emulator.h"

uint8_t keymap[16] = {0};

// map from keysym to keymap index
std::map<SDL_Keycode, int> keymapIndex = {
    {SDLK_1, 0x1},
    {SDLK_2, 0x2},
    {SDLK_3, 0x3},
    {SDLK_4, 0xC},
    {SDLK_q, 0x4},
    {SDLK_w, 0x5},
    {SDLK_e, 0x6},
    {SDLK_r, 0xD},
    {SDLK_a, 0x7},
    {SDLK_s, 0x8},
    {SDLK_d, 0x9},
    {SDLK_f, 0xE},
    {SDLK_z, 0xA},
    {SDLK_x, 0x0},
    {SDLK_c, 0xB},
    {SDLK_v, 0xF}};

Emulator::Emulator(std::vector<uint8_t> rom) : rom(rom)
{
    setOpcodeHandlers();
    // Initialize the memory
    for (int i = 0; i < RAM_SIZE; i++)
    {
        memory[i] = 0;
    }

    // Initialize the program counter and index register
    pc = 0x200;
    I = 0;

    // Initialize the stack pointer
    sp = 0;

    // Initialize the stack
    for (int i = 0; i < 16; i++)
    {
        stack[i] = 0;
    }

    // Initialize the registers
    for (int i = 0; i < 16; i++)
    {
        V[i] = 0;
    }

    // Initialize the delay timer and sound timer
    delay_timer = 0;
    sound_timer = 0;

    // put rom after 0x200
    for (size_t i = 0; i < rom.size(); i++)
    {
        memory[0x200 + i] = rom[i];
    }

    // load characters into memory
    for (int i = 0; i < 80; i++)
    {
        memory[i] = characters[i];
    }
}

void Emulator::handle0Family()
{
    if (nnn == 0x0E0)
    {
        display.clear();
        display.update();
    }
    else if (nnn == 0x0EE)
    {
        ret();
    }
    // Handle other 0x0 family cases...
}

// returns from a subroutine by setting the program counter to the address at the top of the stack
void Emulator::ret()
{
    pc = stack[sp];
    sp--;
}

// jumps to the address nnn
void Emulator::jpAddr()
{
    pc = nnn;
}

// calls the subroutine at nnn by pushing the program counter to the stack and setting the program counter to nnn
void Emulator::callAddr()
{
    sp++;
    stack[sp] = pc;
    pc = nnn;
}

// skips the next instruction if vx to kk
void Emulator::skipIfVxEqualsNN()
{
    if (V[x] == nn)
    {
        pc += 2;
    }
}

// skips the next instruction if vx is not equal to kk
void Emulator::skipIfVxNotEqualsNN()
{
    if (V[x] != nn)
    {
        pc += 2;
    }
}

// skips the next instruction if vx is equal to vy
void Emulator::skipIfVxEqualsVy()
{
    if (V[x] == V[y])
    {
        pc += 2;
    }
}

// sets vx to nn
void Emulator::setVxToNN()
{
    V[x] = nn;
}

// adds nn to vx
void Emulator::addNNToVx()
{
    V[x] += nn;
}

// Handle 8 family opcodes
void Emulator::handle8Family()
{
    if (n == 0x0) // stores the value of the register vy in register vx

    {
        V[x] = V[y];
    }
    else if (n == 0x1) // sets vx to vx or vy
    {
        V[x] |= V[y];
    }
    else if (n == 0x2) // sets vx to vx and vy
    {
        V[x] &= V[y];
    }
    else if (n == 0x3) // sets vx to vx xor vy
    {
        V[x] ^= V[y];
    }
    else if (n == 0x4) // adds vy to vx. vf is set to 1 when there is a carry, and to 0 when there is not
    {
        V[0xF] = (V[x] + V[y] > 255) ? 1 : 0;
        V[x] += V[y];
    }
    else if (n == 0x5) // subtracts vy from vx. vf is set to 0 when there is a borrow, and 1 when there is not
    {
        V[0xF] = (V[x] > V[y]) ? 1 : 0;
        V[x] -= V[y];
    }
    else if (n == 0x6) // shifts vx right by one. vf is set to the least significant bit of vx before the shift
    {
        V[0xF] = V[x] & 0x1;
        V[x] >>= 1;
    }
    else if (n == 0x7) // sets vx to vy minus vx. vf is set to 0 when there is a borrow, and 1 when there is not
    {
        V[0xF] = (V[y] > V[x]) ? 1 : 0;
        V[x] = V[y] - V[x];
    }
    else if (n == 0xE) // shifts vx left by one. vf is set to the most significant bit of vx before the shift
    {
        V[0xF] = V[x] >> 7;
        V[x] <<= 1;
    }
}

void Emulator::skipIfVxNotEqualsVy()
{
    if (V[x] != V[y])
    {
        pc += 2;
    }
}

void Emulator::setI()
{
    I = nnn;
}

void Emulator::jumpToNNNPlusV0()
{
    pc = nnn + V[0];
}

void Emulator::setVxToRandAndNN()
{
    V[x] = rand() % 256 & nn;
}

// draw sprite at vx, vy with width of 8 and height of n
// vf is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
void Emulator::drawSprite()
{
    V[0xF] = 0;
    for (int i = 0; i < n; i++)
    {
        uint8_t sprite = memory[I + i];
        for (int j = 0; j < 8; j++)
        {
            if ((sprite & (0x80 >> j)) != 0)
            {
                if (display.getPixel((V[x] + j), (V[y] + i)))
                {
                    V[0xF] = 1;
                }
                display.setPixel((V[x] + j), (V[y] + i) , display.getPixel((V[x] + j) , (V[y] + i) ) ^ 1);
            }
        }
    }
    display.update();
}

void Emulator::handleEFamily()
{
    // skips the next instruction if the key stored in vx is pressed
    if (nn == 0x9E)
    {
        // std::cout << "Checking key " << V[x] << std::endl;
        if (keymap[V[x]])
        {
            pc += 2;
        }
    }

    // skips the next instruction if the key stored in vx is not pressed
    if (nn == 0xA1)
    {
        // std::cout << "Checking key " << std::hex << unsigned(V[x]) << std::endl;
        if (!keymap[V[x]])
        {
            pc += 2;
        }
    }
}

void Emulator::handleFFamily()
{
    if (nn == 0x07) // sets vx to the value of the delay timer
    {
        V[x] = delay_timer;
    }
    else if (nn == 0x0A) // waits for a key press and stores the result in vx
    {
        bool key_pressed = false;
        for (int i = 0; i < 16; i++)
        {
            if (keymap[i])
            {
                V[x] = i;
                key_pressed = true;
            }
        }
        if (!key_pressed)
        {
            pc -= 2;
        }
    }
    else if (nn == 0x15) // sets the delay timer to vx
    {
        delay_timer = V[x];
    }
    else if (nn == 0x18) // sets the sound timer to vx
    {
        sound_timer = V[x];
    }
    else if (nn == 0x1E) // adds vx to I
    {
        I += V[x];
    }
    else if (nn == 0x29) // sets I to the location of the sprite for the character in vx
    {
        I = V[x] * 5;
    }
    else if (nn == 0x33) // stores the binary-coded decimal representation of vx at I, I+1, and I+2
    {
        memory[I] = V[x] / 100;
        memory[I + 1] = (V[x] / 10) % 10;
        memory[I + 2] = V[x] % 10;
    }
    else if (nn == 0x55) // stores V0 to vx in memory starting at I
    {
        for (int i = 0; i <= x; i++)
        {
            memory[I + i] = V[i];
        }
    }
    else if (nn == 0x65) // fills V0 to vx with values from memory starting at I
    {
        for (int i = 0; i <= x; i++)
        {
            V[i] = memory[I + i];
        }
    }
}
void Emulator::setOpcodeHandlers()
{
    opcodeHandlers[0x0] = &Emulator::handle0Family;
    opcodeHandlers[0x1] = &Emulator::jpAddr;
    opcodeHandlers[0x2] = &Emulator::callAddr;
    opcodeHandlers[0x3] = &Emulator::skipIfVxEqualsNN;
    opcodeHandlers[0x4] = &Emulator::skipIfVxNotEqualsNN;
    opcodeHandlers[0x5] = &Emulator::skipIfVxEqualsVy;
    opcodeHandlers[0x6] = &Emulator::setVxToNN;
    opcodeHandlers[0x7] = &Emulator::addNNToVx;
    opcodeHandlers[0x8] = &Emulator::handle8Family;
    opcodeHandlers[0x9] = &Emulator::skipIfVxNotEqualsVy;
    opcodeHandlers[0xA] = &Emulator::setI;
    opcodeHandlers[0xB] = &Emulator::jumpToNNNPlusV0;
    opcodeHandlers[0xC] = &Emulator::setVxToRandAndNN;
    opcodeHandlers[0xD] = &Emulator::drawSprite;
    opcodeHandlers[0xE] = &Emulator::handleEFamily;
    opcodeHandlers[0xF] = &Emulator::handleFFamily;
}

void Emulator::fetch()
{
    opcode = memory[pc] << 8 | memory[pc + 1];
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    n = opcode & 0x000F;
    nn = opcode & 0x00FF;
    nnn = opcode & 0x0FFF;
    pc += 2;
}

void Emulator::decode()
{
    uint8_t first_nibble = (opcode & 0xF000) >> 12;
    // if the opcode is in the map keys, call the function
    if (opcodeHandlers.find(first_nibble) != opcodeHandlers.end())
    {
        (this->*opcodeHandlers[first_nibble])();
    }
    else
    {
        std::cerr << "Error: Opcode " << std::hex << opcode << " not implemented" << std::endl;
    }
    if (delay_timer > 0)
    {
        delay_timer--;
    }
    if (sound_timer > 0)
    {
        printf("\a\n"); // gulp
        sound_timer--;
    }
}

void Emulator::cycle()
{
    // Constants for timing
    const int instructionsPerSecond = 900;                     // Target instructions per second
    const auto instructionTime = 1000 / instructionsPerSecond; // Time per instruction

    auto nextInstructionTime = clock();
    // for each cycle, fetch, decode, and execute
    std::cout
        << "size of rom: " << rom.size() << std::endl;
    for (; pc < 0x200 + rom.size();)
    {
        // check for key up and down events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (keymapIndex.find(event.key.keysym.sym) != keymapIndex.end())
                keymap[keymapIndex[event.key.keysym.sym]] = 1;
            }
            else if (event.type == SDL_KEYUP)
            {
                if (keymapIndex.find(event.key.keysym.sym) != keymapIndex.end()) 
                keymap[keymapIndex[event.key.keysym.sym]] = 0;
            }
        }

        fetch();
        decode();
        // reset values for next cycle
        opcode = 0;
        x = 0;
        y = 0;
        n = 0;
        nn = 0;
        nnn = 0;

        nextInstructionTime += instructionTime;
        // Sleep until the next instruction time
        while (clock() < nextInstructionTime)
        {
        }
    }
}