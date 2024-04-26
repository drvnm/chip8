#ifndef H_Emulator
#define H_Emulator
#include <cstdint>
#include <vector>
#include <iostream>
#include <map>
// #include <chrono>
// #include <thread>
#include <ctime>
#include <SDL2/SDL.h>

#include "display.h"

#define RAM_SIZE 4096

const uint8_t characters[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

extern uint8_t keymap[16];

// A class to represent the Emulator of the Chip-8
class Emulator
{
private:
    std::vector<uint8_t> rom; // The rom being executed
    // The memory of the Emulator
    uint8_t memory[RAM_SIZE];
    // program counter and index register
    uint16_t pc, I;
    // stack pointer
    uint8_t sp;
    // stack
    uint16_t stack[16];
    // registers
    uint8_t V[16];
    // delay timer and sound timer
    uint8_t delay_timer, sound_timer;
    // screen is external class
    Display display = Display();

    // opcode
    uint16_t opcode;

    // x,y,n,nn,nnn
    uint8_t x;    // The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
    uint8_t y;    // The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
    uint8_t n;    // The fourth nibble. A 4-bit number.
    uint8_t nn;   // The second byte (third and fourth nibbles). An 8-bit immediate number.
    uint16_t nnn; // The second, third and fourth nibbles. A 12-bit immediate memory address.

    std::map<uint8_t, void (Emulator::*)()> opcodeHandlers;

    // Opcode handlers
    void handle0Family();
    void cls();
    void ret();
    void jpAddr();
    void callAddr();
    void skipIfVxEqualsNN();
    void skipIfVxNotEqualsNN();
    void skipIfVxEqualsVy();
    void setVxToNN();
    void addNNToVx();
    void handle8Family();
    void skipIfVxNotEqualsVy();
    void setI();
    void jumpToNNNPlusV0();
    void setVxToRandAndNN();
    void drawSprite();
    void handleEFamily();
    void handleFFamily();

    void fetch();
    // decodes and executes the opcode
    void decode();

    void setOpcodeHandlers();

public:
    Emulator(std::vector<uint8_t> rom);
    void cycle();
};

#endif