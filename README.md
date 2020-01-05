# SpectREMCPP (Spectrum Retro EMulator using C++ and Objective-C++) 

This is a ZX Spectrum emulator written for MacOS 10.14+. The main emulator core has been written in C++ and is shared across a Mac, Windows and iOS versions. This is very much a hobby/play project, but it's prettry accurate for the 48k/128k Spectrum :)

## Thanks

- **Adrian Brown** for the initial Z80 core written in C because my core written in Swift was...well...a bad idea :) If I couldn't have switched to Adrians core this project would have died right there :)
- **Paul Tankard** for testing, his C++ experience, highlighting my poor coding practices :) and creating SmartLINK hardware and software
- **John Young** for taking on the task of creating the Windows platform specific code on top of the emulator core
- **Mark Woodmass** for his vast knowledge of the ZX Spectrum, emulator programming and his never ending stream emulator accuracy tests
- **Richard Chandler** for his banta on the ZXASM - Z80 Programming Slack channel and keeping things real ;)

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

