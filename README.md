# SpectREMCPP (Spectrum Retro EMulator using C++ and Objective-C++) 

This is a ZX Spectrum emulator written for MacOS 10.14+. The main emulator core has been written in C++ and is available from the ZXEmuCore repository. This is the repository that is used for the core of this emulatore.

## Features

- Emulates the 48k and 128k ZX Spectrum
- Extremely accurate Z80 core (originally developed by Adrian Brown and updated by myself)
  - Passes all emulator based tests for Z80 core accuracy including FLAGS, MEMPTR and SCF/CCF Q register
- Cycle accurate emulation of the ULA allowing all advanced colour demos to work correctly (the ones tested ;o) )
- Beeper emulation
- AY emulation
- TAP file loading and saving
- TAP Insta loading
- SNA 48k snapshot loading/saving
- Z80 48k/128k snapshot loading/saving
- Virtual tape browser
- Debugger (Under active development)
  - Memory Viewer
  - CPU view (registers and flags)
  - Pause, Resume
  - Breakpoints
  - Step In
- Automatically restores your last session
- Allows selection of the default 48k/128k ROM

## Peripheral Emulation

- SpecDrum

## SmartLINK

- SmartLINK being developed by Paul Tankard. This uses an Arduino connected to a Retroleum Smartcard to allow input from the emulator to a real Spectrum. It supports the ability to send what is running on the emulator directly to a real Spectrum.
- Currently only works on 48k Spectrum hardware
- Development goal is to use this as a development/debugger tool for the Spectrum

## Todo list

- SZX
- Full debugger/disassembler
  - Step Over
  - Break on Read/Write/Execute of a memory location
  - Screen debugger that shows what has been drawn to screen even when single stepping instructions
  - Screen debugger that can be used to show a specific memory page for 128k screen debugging e.g. look at the page updating that is going to be flipped too
  - Show on screen where the screen refresh location is for debugging colour effects

