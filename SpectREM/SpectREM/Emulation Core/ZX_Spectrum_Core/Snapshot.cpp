//
//  Snapshot.cpp
//  SpectREM
//
//  Created by Mike Daley on 26/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include <stdio.h>
#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"

// - Constants

const uint8_t               cSNA_HEADER_SIZE = 27;
const uint16_t              cZ80_V3_HEADER_SIZE = 86;
const uint16_t              cZ80_V3_ADD_HEADER_SIZE = 54;
const uint8_t               cZ80_V3_PAGE_HEADER_SIZE = 3;

const uint8_t               cZ80_V2_MACHINE_TYPE_48 = 0;
const uint8_t               cZ80_V3_MACHINE_TYPE_48 = 0;
const uint8_t               cZ80_V2_MACHINE_TYPE_48_IF1 = 1;
const uint8_t               cZ80_V3_MACHINE_TYPE_48_IF1 = 1;
const uint8_t               cZ80_V2_MACHINE_TYPE_128 = 3;
const uint8_t               cZ80_V3_MACHINE_TYPE_48_MGT = 3;
const uint8_t               cZ80_V2_MACHINE_TYPE_128_IF1 = 4;
const uint8_t               cZ80_V3_MACHINE_TYPE_128 = 4;
const uint8_t               cZ80_V3_MACHINE_TYPE_128_IF1 = 5;
const uint8_t               cz80_V3_MACHINE_TYPE_128_MGT = 6;
const uint8_t               cZ80_V3_MACHINE_TYPE_128_3 = 7;
const uint8_t               cZ80_V3_MACHINE_TYPE_128_2 = 12;
const uint8_t               cZ80_V3_MACHINE_TYPE_128_2A = 13;

// ------------------------------------------------------------------------------------------------------------
// - SNA functions

ZXSpectrum::SnapshotData ZXSpectrum::snapshotCreateSNA()
{
    // We don't want the core running when we take a snapshot
    pause();

    SnapshotData snap;
    snap.length = (48 * 1024) + cSNA_HEADER_SIZE;
    snap.data = new uint8_t[snap.length];

    snap.data[0] = z80Core.GetRegister(CZ80Core::eREG_I);
    snap.data[1] = z80Core.GetRegister(CZ80Core::eREG_ALT_HL) & 0xff;
    snap.data[2] = z80Core.GetRegister(CZ80Core::eREG_ALT_HL) >> 8;

    snap.data[3] = z80Core.GetRegister(CZ80Core::eREG_ALT_DE) & 0xff;
    snap.data[4] = z80Core.GetRegister(CZ80Core::eREG_ALT_DE) >> 8;

    snap.data[5] = z80Core.GetRegister(CZ80Core::eREG_ALT_BC) & 0xff;
    snap.data[6] = z80Core.GetRegister(CZ80Core::eREG_ALT_BC) >> 8;

    snap.data[7] = z80Core.GetRegister(CZ80Core::eREG_ALT_AF) & 0xff;
    snap.data[8] = z80Core.GetRegister(CZ80Core::eREG_ALT_AF) >> 8;

    snap.data[9] = z80Core.GetRegister(CZ80Core::eREG_HL) & 0xff;
    snap.data[10] = z80Core.GetRegister(CZ80Core::eREG_HL) >> 8;

    snap.data[11] = z80Core.GetRegister(CZ80Core::eREG_DE) & 0xff;
    snap.data[12] = z80Core.GetRegister(CZ80Core::eREG_DE) >> 8;

    snap.data[13] = z80Core.GetRegister(CZ80Core::eREG_BC) & 0xff;
    snap.data[14] = z80Core.GetRegister(CZ80Core::eREG_BC) >> 8;

    snap.data[15] = z80Core.GetRegister(CZ80Core::eREG_IY) & 0xff;
    snap.data[16] = z80Core.GetRegister(CZ80Core::eREG_IY) >> 8;

    snap.data[17] = z80Core.GetRegister(CZ80Core::eREG_IX) & 0xff;
    snap.data[18] = z80Core.GetRegister(CZ80Core::eREG_IX) >> 8;

    snap.data[19] = static_cast<uint8_t>( (z80Core.GetIFF1() & 1) << 2 );
    snap.data[20] = z80Core.GetRegister(CZ80Core::eREG_R);

    snap.data[21] = z80Core.GetRegister(CZ80Core::eREG_AF) & 0xff;
    snap.data[22] = z80Core.GetRegister(CZ80Core::eREG_AF) >> 8;

    uint16_t pc = z80Core.GetRegister(CZ80Core::eREG_PC);
    uint16_t sp = z80Core.GetRegister(CZ80Core::eREG_SP) - 2;

    snap.data[23] = sp & 0xff;
    snap.data[24] = sp >> 8;

    snap.data[25] = z80Core.GetIMMode();
    snap.data[26] = displayBorderColor & 0x07;

    uint32_t dataIndex = cSNA_HEADER_SIZE;
    for (uint32_t addr = 16384; addr < 16384 + (48 * 1024); addr++)
    {
        snap.data[dataIndex++] = z80Core.Z80CoreDebugMemRead(static_cast<uint16_t>(addr), nullptr);
    }

    // Update the SP location in the snapshot buffer with the new SP, as PC has been added to the stack
    // as part of creating the snapshot
    snap.data[sp - 16384 + cSNA_HEADER_SIZE] = pc & 0xff;
    snap.data[sp - 16384 + cSNA_HEADER_SIZE + 1] = pc >> 8;

    resume();

    return snap;
}

// ------------------------------------------------------------------------------------------------------------

Tape::FileResponse ZXSpectrum::snapshotSNALoadWithPath(const std::string path)
{
    
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        char* errorstring = strerror(errno);
        return Tape::FileResponse{false, errorstring};
    }
    
    std::vector<char> pFileBytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(pFileBytes.data(), pFileBytes.size());

    std::cout << "Loading SNA snapshot: " << "\n";
    
    return snapshotSNALoadWithBuffer(pFileBytes.data(), pFileBytes.size());
}

// ------------------------------------------------------------------------------------------------------------

Tape::FileResponse ZXSpectrum::snapshotSNALoadWithBuffer(const char *buffer, size_t size)
{
    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    std::vector<char> pFileBytes(buffer, buffer + size);

    if (size > 0)
    {
        // Decode the header
        z80Core.SetRegister(CZ80Core::eREG_I, buffer[0]);
        z80Core.SetRegister(CZ80Core::eREG_R, buffer[20]);
        z80Core.SetRegister(CZ80Core::eREG_ALT_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[0]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[1]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[2]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_AF, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[3]));
        z80Core.SetRegister(CZ80Core::eREG_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[4]));
        z80Core.SetRegister(CZ80Core::eREG_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[5]));
        z80Core.SetRegister(CZ80Core::eREG_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[6]));
        z80Core.SetRegister(CZ80Core::eREG_IY, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[7]));
        z80Core.SetRegister(CZ80Core::eREG_IX, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[8]));
        z80Core.SetRegister(CZ80Core::eREG_AF, (reinterpret_cast<uint16_t *>(&pFileBytes[21])[0]));
        z80Core.SetRegister(CZ80Core::eREG_SP, (reinterpret_cast<uint16_t *>(&pFileBytes[21])[1]));

        // Border colour
        displayBorderColor = pFileBytes[26] & 0x07;

        // Set the IM
        z80Core.SetIMMode(pFileBytes[25]);

        // Do both on bit 2 as a RETN copies IFF2 to IFF1
        z80Core.SetIFF1((pFileBytes[19] >> 2) & 1);
        z80Core.SetIFF2((pFileBytes[19] >> 2) & 1);

        if (size == (48 * 1024) + cSNA_HEADER_SIZE)
        {
            uint32_t snaAddr = cSNA_HEADER_SIZE;
            for (uint32_t i = 16384; i < (64 * 1024); i++)
            {
                memoryRam[i] = static_cast<char>( pFileBytes[snaAddr++] );
            }

            // Set the PC
            uint8_t pc_lsb = static_cast<uint8_t>(memoryRam[z80Core.GetRegister(CZ80Core::eREG_SP)]);
            uint8_t pc_msb = static_cast<uint8_t>(memoryRam[(z80Core.GetRegister(CZ80Core::eREG_SP) + 1)]);
            z80Core.SetRegister(CZ80Core::eREG_PC, static_cast<uint16_t>((pc_msb << 8) | pc_lsb));
            z80Core.SetRegister(CZ80Core::eREG_SP, z80Core.GetRegister(CZ80Core::eREG_SP) + 2);
        }
    }

    resume();

    return Tape::FileResponse{true, "Loaded successfully"};
}

// ------------------------------------------------------------------------------------------------------------
// - Z80 Snapshot functions

/*
 Returning an empty Snap struct means snapshot creation failed
 */
ZXSpectrum::SnapshotData ZXSpectrum::snapshotCreateZ80()
{
    int snapshotSize = 0;
    switch (machineInfo.machineType) {
        case eZXSpectrum48:
            snapshotSize = (48 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 3);
            break;
            
        case eZXSpectrum128:
        case eZXSpectrum128_2:
        case eZXSpectrum128_2A:
            snapshotSize = (128 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 8);
            break;
            
        default:
            std::cout << "Unknown machine type" << "\n";
            SnapshotData emptySnap;
            emptySnap.length = 0;
            emptySnap.data = nullptr;
            return emptySnap;
            break;
    }

    // We don't want the emulator running while we create a snapshot
    pause();

    // Structure to be returned containing the length and size of the snapshot
    SnapshotData snapData;
    snapData.length = snapshotSize;
    snapData.data = new uint8_t[ snapshotSize ];

    // Header
    snapData.data[0] = z80Core.GetRegister(CZ80Core::eREG_A);
    snapData.data[1] = z80Core.GetRegister(CZ80Core::eREG_F);
    snapData.data[2] = z80Core.GetRegister(CZ80Core::eREG_BC) & 0xff;
    snapData.data[3] = z80Core.GetRegister(CZ80Core::eREG_BC) >> 8;
    snapData.data[4] = z80Core.GetRegister(CZ80Core::eREG_HL) & 0xff;
    snapData.data[5] = z80Core.GetRegister(CZ80Core::eREG_HL) >> 8;
    snapData.data[6] = 0x0; // PC
    snapData.data[7] = 0x0;
    snapData.data[8] = z80Core.GetRegister(CZ80Core::eREG_SP) & 0xff;
    snapData.data[9] = z80Core.GetRegister(CZ80Core::eREG_SP) >> 8;
    snapData.data[10] = z80Core.GetRegister(CZ80Core::eREG_I);
    snapData.data[11] = z80Core.GetRegister(CZ80Core::eREG_R) & 0x7f;

    uint8_t byte12 = z80Core.GetRegister(CZ80Core::eREG_R) >> 7;
    byte12 |= (displayBorderColor & 0x07) << 1;
    byte12 &= ~(1 << 5);
    snapData.data[12] = byte12;

    snapData.data[13] = z80Core.GetRegister(CZ80Core::eREG_E);            // E
    snapData.data[14] = z80Core.GetRegister(CZ80Core::eREG_D);            // D
    snapData.data[15] = z80Core.GetRegister(CZ80Core::eREG_ALT_C);        // C'
    snapData.data[16] = z80Core.GetRegister(CZ80Core::eREG_ALT_B);        // B'
    snapData.data[17] = z80Core.GetRegister(CZ80Core::eREG_ALT_E);        // E'
    snapData.data[18] = z80Core.GetRegister(CZ80Core::eREG_ALT_D);        // D'
    snapData.data[19] = z80Core.GetRegister(CZ80Core::eREG_ALT_L);        // L'
    snapData.data[20] = z80Core.GetRegister(CZ80Core::eREG_ALT_H);        // H'
    snapData.data[21] = z80Core.GetRegister(CZ80Core::eREG_ALT_A);        // A'
    snapData.data[22] = z80Core.GetRegister(CZ80Core::eREG_ALT_F);        // F'
    snapData.data[23] = z80Core.GetRegister(CZ80Core::eREG_IY) & 0xff;    // IY
    snapData.data[24] = z80Core.GetRegister(CZ80Core::eREG_IY) >> 8;      //
    snapData.data[25] = z80Core.GetRegister(CZ80Core::eREG_IX) & 0xff;    // IX
    snapData.data[26] = z80Core.GetRegister(CZ80Core::eREG_IX) >> 8;      //
    snapData.data[27] = (z80Core.GetIFF1()) ? 0xff : 0x0;
    snapData.data[28] = (z80Core.GetIFF2()) ? 0xff : 0x0;
    snapData.data[29] = z80Core.GetIMMode() & 0x03;                       // IM Mode

    // Version 3 Additional Header
    snapData.data[30] = (cZ80_V3_ADD_HEADER_SIZE) & 0xff;                 // Additional Header Length
    snapData.data[31] = (cZ80_V3_ADD_HEADER_SIZE) >> 8;

    snapData.data[32] = z80Core.GetRegister(CZ80Core::eREG_PC) & 0xff;    // PC
    snapData.data[33] = z80Core.GetRegister(CZ80Core::eREG_PC) >> 8;

    switch (machineInfo.machineType) {
        case eZXSpectrum48:
            snapData.data[34] = cZ80_V3_MACHINE_TYPE_48;
            break;

        case eZXSpectrum128:
            snapData.data[34] = cZ80_V3_MACHINE_TYPE_128;
            break;

        case eZXSpectrum128_2:
            snapData.data[34] = cZ80_V3_MACHINE_TYPE_128_2;
            break;

        case eZXSpectrum128_2A:
            snapData.data[34] = cZ80_V3_MACHINE_TYPE_128_2A;
            break;

        default:
            break;
    }

    if (machineInfo.machineType == eZXSpectrum128 ||
        machineInfo.machineType == eZXSpectrum128_2 ||
        machineInfo.machineType == eZXSpectrum128_2A ||
        machineInfo.machineType == eZXSpectrum128_3)
    {
        snapData.data[35] = ULAPort7FFDValue; // last 128k 0x7ffd port value
    }
    else
    {
        snapData.data[35] = 0;
    }

    snapData.data[36] = 0; // Interface 1 ROM
    snapData.data[37] = 4; // AY Sound
    snapData.data[38] = ULAPort7FFDValue; // Last OUT fffd

    // Save the AY register values
    uint32_t dataIndex = 39;
    for (uint8_t i = 0; i < 16; i++)
    {
        audioAYSetRegister(i);
        snapData.data[dataIndex++] = audioAYReadData();
    }

    uint32_t quarterStates = machineInfo.tsPerFrame / 4;
    uint32_t lowTStates = quarterStates - (z80Core.GetTStates() % quarterStates) - 1;
    snapData.data[55] = lowTStates & 0xff;
    snapData.data[56] = static_cast<uint8_t>(lowTStates >> 8);

    snapData.data[57] = ((z80Core.GetTStates() / quarterStates) + 3) % 4;
    snapData.data[58] = 0; // QL Emu
    snapData.data[59] = 0; // MGT Paged ROM
    snapData.data[60] = 0; // Multiface ROM paged
    snapData.data[61] = 0; // 0 - 8192 ROM
    snapData.data[63] = 0; // 8192 - 16384 ROM
    snapData.data[83] = 0; // MGT Type
    snapData.data[84] = 0; // Disciple inhibit button
    snapData.data[85] = 0; // Disciple inhibit flag
    snapData.data[86] = ULAPort1FFDValue; // Last out to 0x1ffd

    uint32_t snapPtr = cZ80_V3_HEADER_SIZE;

    switch (machineInfo.machineType) {
        case eZXSpectrum48:
            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 4;

            for (uint32_t memAddr = 0x8000; memAddr <= 0xbfff; memAddr++)
            {
                snapData.data[snapPtr++] = z80Core.Z80CoreDebugMemRead(static_cast<uint16_t>(memAddr), nullptr);
            }

            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 5;

            for (uint32_t memAddr = 0xc000; memAddr <= 0xffff; memAddr++)
            {
                snapData.data[snapPtr++] = z80Core.Z80CoreDebugMemRead(static_cast<uint16_t>(memAddr), nullptr);
            }

            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 0xff;
            snapData.data[snapPtr++] = 8;

            for (uint32_t memAddr = 0x4000; memAddr <= 0x7fff; memAddr++)
            {
                snapData.data[snapPtr++] = z80Core.Z80CoreDebugMemRead(static_cast<uint16_t>(memAddr), nullptr);
            }
            break;

        case eZXSpectrum128:
        case eZXSpectrum128_2:
        case eZXSpectrum128_2A:
            for (uint8_t page = 0; page < 8; page++)
            {
                snapData.data[snapPtr++] = 0xff;
                snapData.data[snapPtr++] = 0xff;
                snapData.data[snapPtr++] = page + 3;

                for (uint32_t memAddr = page * 0x4000ul; memAddr < (page * 0x4000ul) + 0x4000ul; memAddr++)
                {
                    snapData.data[snapPtr++] = static_cast<uint8_t>(memoryRam[static_cast<size_t>(memAddr)]);
                }
            }
            break;

        default:
            break;
    }

    resume();

    return snapData;
}

// ------------------------------------------------------------------------------------------------------------

Tape::FileResponse ZXSpectrum::snapshotZ80LoadWithPath(const std::string path)
{
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        char* errorstring = strerror(errno);
        return Tape::FileResponse{ false, errorstring };
    }
    
    std::vector<char> pFileBytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(pFileBytes.data(), pFileBytes.size());
    
    return snapshotZ80LoadWithBuffer(pFileBytes.data(), pFileBytes.size());
}

// ------------------------------------------------------------------------------------------------------------

Tape::FileResponse ZXSpectrum::snapshotZ80LoadWithBuffer(const char *buffer, size_t size)
{
    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    std::vector<char> pFileBytes(buffer, buffer + size);

    if (size > 0)
    {
        // Decode the header
        uint16_t headerLength = (reinterpret_cast<uint16_t *>(&pFileBytes[30])[0] );
        uint16_t version;
        uint16_t hardwareType;
        uint16_t pc;

        switch (headerLength) {
        case 23:
            version = 2;
            pc = (reinterpret_cast<uint16_t *>( &pFileBytes[32])[0]);
            break;
        case 54:
        case 55:
            version = 3;
            pc = (reinterpret_cast<uint16_t *>( &pFileBytes[32])[0]);
            break;
        default:
            version = 1;
            pc = (reinterpret_cast<uint16_t *>( &pFileBytes[6])[0]);
            break;
        }

        if (pc == 0)
        {
            version = 2;
            pc = (reinterpret_cast<uint16_t *>( &pFileBytes[32])[0]);
        }
        std::cout << "Loading Z80 snapshot v" << version << ": " << "\n";

        z80Core.SetRegister(CZ80Core::eREG_A, pFileBytes[0]);
        z80Core.SetRegister(CZ80Core::eREG_F, pFileBytes[1]);
        z80Core.SetRegister(CZ80Core::eREG_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[2])[0]));
        z80Core.SetRegister(CZ80Core::eREG_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[2])[1]));
        z80Core.SetRegister(CZ80Core::eREG_PC, pc);
        z80Core.SetRegister(CZ80Core::eREG_SP, (reinterpret_cast<uint16_t *>(&pFileBytes[8])[0]));
        z80Core.SetRegister(CZ80Core::eREG_I, static_cast<uint8_t>(pFileBytes[10]));
        z80Core.SetRegister(CZ80Core::eREG_R, static_cast<uint8_t>((pFileBytes[11] & 127) | ((pFileBytes[12] & 1) << 7)));

        // Decode byte 12
        //    Bit 0  : Bit 7 of the R-register
        //    Bit 1-3: Border colour
        //    Bit 4  : 1=Basic SamRom switched in
        //    Bit 5  : 1=Block of data is compressed
        //    Bit 6-7: No meaning
        uint8_t byte12 = pFileBytes[12];

        // For campatibility reasons if byte 12 = 255 then it should be assumed to = 1
        byte12 = (byte12 == 255) ? 1 : byte12;

        displayBorderColor = (byte12 & 14) >> 1;
        bool compressed = (byte12 & 32) != 0 ? true : false;

        z80Core.SetRegister(CZ80Core::eREG_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[0]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[1]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[2]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[3]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_A, static_cast<uint8_t>(pFileBytes[21]));
        z80Core.SetRegister(CZ80Core::eREG_ALT_F, static_cast<uint8_t>(pFileBytes[22]));
        z80Core.SetRegister(CZ80Core::eREG_IY, (reinterpret_cast<uint16_t *>(&pFileBytes[23])[0]));
        z80Core.SetRegister(CZ80Core::eREG_IX, (reinterpret_cast<uint16_t *>(&pFileBytes[23])[1]));
        z80Core.SetIFF1(static_cast<uint8_t>(pFileBytes[27] & 1));
        z80Core.SetIFF2(static_cast<uint8_t>(pFileBytes[28] & 1));
        z80Core.SetIMMode(static_cast<uint8_t>(pFileBytes[29] & 3));

        // Load AY register values
        uint32_t fileBytesIndex = 39;
        for (uint32_t i = 0; i < 16; i++)
        {
            audioAYSetRegister(i);
            audioAYWriteData(pFileBytes[ fileBytesIndex++ ]);
        }

        audioAYSetRegister(pFileBytes[38]);

        // Based on the version number of the snapshot, decode the memory contents
        switch (version) {
        case 1:
            snapshotExtractMemoryBlock(buffer, size, 0x4000, 30, compressed, 0xc000);
            break;

        case 2:
        case 3:
            hardwareType = reinterpret_cast<uint8_t *>(&pFileBytes[34])[0];
            uint16_t additionHeaderBlockLength = 0;
            additionHeaderBlockLength = reinterpret_cast<uint16_t *>(&pFileBytes[30])[0];
            uint32_t offset = 32 + additionHeaderBlockLength;

            if (hardwareType == cZ80_V3_MACHINE_TYPE_128 ||
                hardwareType == cZ80_V3_MACHINE_TYPE_128_IF1 ||
                hardwareType == cz80_V3_MACHINE_TYPE_128_MGT ||
                hardwareType == cZ80_V3_MACHINE_TYPE_128_2)
            {
                // Decode byte 35 so that port 0x7ffd can be set on the 128k
                uint8_t data = reinterpret_cast<uint8_t *>(&pFileBytes[35])[0];
                emuDisablePaging = ((data & 0x20) == 0x20) ? true : false;
                emuROMNumber = ((data & 0x10) == 0x10) ? 1 : 0;
                emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
                emuRAMPage = (data & 0x07);
            }
            else if (hardwareType == cZ80_V3_MACHINE_TYPE_128_2A ||
                     hardwareType == cZ80_V3_MACHINE_TYPE_128_3)
            {
                uint8_t data7ffd = reinterpret_cast<uint8_t *>(&pFileBytes[35])[0];
                emuDisablePaging = ((data7ffd & 0x20) == 0x20) ? true : false;
                emuROMLoBit = (data7ffd & 0x10) >> 4;
                emuDisplayPage = ((data7ffd & 0x08) == 0x08) ? 7 : 5;
                emuRAMPage = (data7ffd & 0x07);

                uint8_t data1ffd = reinterpret_cast<uint8_t *>(&pFileBytes[86])[0];
                emuROMHiBit = ((data1ffd & 0x04) >> 1);
                
                emuROMNumber = emuROMHiBit | emuROMLoBit;
            }
            else
            {
                emuDisablePaging = true;
                emuROMNumber = 0;
                emuRAMPage = 0;
                emuDisplayPage = 1;
            }

            while (offset < size)
            {
                uint32_t compressedLength = reinterpret_cast<uint16_t*>(&pFileBytes[offset])[0];
                bool isCompressed = true;
                if (compressedLength == 0xffff)
                {
                    compressedLength = 0x4000;
                    isCompressed = false;
                }

                uint32_t pageId = buffer[offset + 2];
                
                std::cout << "Snap Page: " << pageId << "\t\t";
                std::cout << "Mem Page: " << pageId - 3 << "\tCompressed Length: " << compressedLength << "\tIsCompressed: " << isCompressed;
                std::cout << "\tHardware Type: " << snapshotHardwareTypeForVersion(version, hardwareType) << "\n";

                if (hardwareType == cZ80_V2_MACHINE_TYPE_48 ||
                    hardwareType == cZ80_V3_MACHINE_TYPE_48 ||
                    hardwareType == cZ80_V2_MACHINE_TYPE_48_IF1 ||
                    hardwareType == cZ80_V3_MACHINE_TYPE_48_IF1 ||
                    hardwareType == cZ80_V3_MACHINE_TYPE_48_MGT)
                {
                    switch (pageId) {
                    case 4:
                        snapshotExtractMemoryBlock(buffer, size, 0x8000, offset + 3, isCompressed, 0x4000);
                        break;
                    case 5:
                        snapshotExtractMemoryBlock(buffer, size, 0xc000, offset + 3, isCompressed, 0x4000);
                        break;
                    case 8:
                        snapshotExtractMemoryBlock(buffer, size, 0x4000, offset + 3, isCompressed, 0x4000);
                        break;
                    default:
                        break;
                    }
                }
                else if (hardwareType == cZ80_V2_MACHINE_TYPE_128 ||
                         hardwareType == cZ80_V3_MACHINE_TYPE_128 ||
                         hardwareType == cZ80_V2_MACHINE_TYPE_128_IF1 ||
                         hardwareType == cZ80_V3_MACHINE_TYPE_128_IF1 ||
                         hardwareType == cz80_V3_MACHINE_TYPE_128_MGT ||
                         hardwareType == cZ80_V3_MACHINE_TYPE_128_2 ||
                         hardwareType == cZ80_V3_MACHINE_TYPE_128_2A ||
                         hardwareType == cZ80_V3_MACHINE_TYPE_128_3)
                {
                    snapshotExtractMemoryBlock(buffer, size, (pageId - 3) * 0x4000, offset + 3, isCompressed, 0x4000);
                }
                else
                {
                    std::cout << "Something funny going on! Can't find a match for the snap version and machine type.\n";
                    resume();
                    return Tape::FileResponse{ false, "Could not find a match for the supplied version and machine type!" };
                }

                offset += compressedLength + 3;
            }

            break;
        }
    }

    resume();
    
    return Tape::FileResponse{true, "Loaded successfully"};
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::snapshotExtractMemoryBlock(const char *buffer, size_t bufferSize, uint32_t memAddr, uint32_t fileOffset, bool isCompressed, uint32_t unpackedLength)
{
    uint32_t filePtr = fileOffset;
    uint32_t memoryPtr = memAddr;
    std::vector<char> fileBytes(buffer, buffer + bufferSize);

    if (!isCompressed)
    {
        while (memoryPtr < unpackedLength + memAddr && memoryPtr + 1 < memoryRam.size())
        {
            memoryRam[memoryPtr++] = static_cast<int8_t>(fileBytes[filePtr++]);
        }
    }
    else
    {
        while (memoryPtr < unpackedLength + memAddr && memoryPtr < memoryRam.size())
        {
            std::string n = std::string("memoryPtr: ") + std::to_string(memoryPtr) + std::string(" unpackedLength: ") + std::to_string(unpackedLength) + std::string(" memAddr: ") + std::to_string(memAddr) + std::string(" filePtr+1: ") + std::to_string(filePtr + 1) + " - " + std::to_string(fileBytes.size()) + "\n";
            uint8_t byte1 = fileBytes[filePtr];
            
            if (byte1 == 0xed && filePtr + 1 < fileBytes.size())
            {
                uint8_t byte2 = fileBytes[filePtr + 1];

                if (byte2 == 0xed)
                {
                    if (filePtr + 3 < fileBytes.size())
                    {
                        uint8_t count = fileBytes[filePtr + 2];
                        uint8_t value = fileBytes[filePtr + 3];
                        for (int i = 0; i < count; i++)
                        {
                            memoryRam[memoryPtr++] = static_cast<int8_t>(value);
                        }
                        filePtr += 4;
                        continue;
                    }
                    else
                    {
                        std::cout << "Something isn't right, expected packed bytes but ran out of file data!\n";
                        return;
                    }
                }
            }
            
            // Getting here means no compressed bytes were found, so just load the byte into memory
            memoryRam[memoryPtr++] = static_cast<int8_t>(fileBytes.at(filePtr++));
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

std::string ZXSpectrum::snapshotHardwareTypeForVersion(uint32_t version, uint32_t hardwareType)
{
    std::string hardware = "Unknown";
    if (version == 2)
    {
        switch (hardwareType) {
            case cZ80_V2_MACHINE_TYPE_48:
                hardware = "48k";
                break;
            case cZ80_V2_MACHINE_TYPE_48_IF1:
                hardware = "48k + Interface 1";
                break;
            case cZ80_V2_MACHINE_TYPE_128:
                hardware = "128k";
                break;
            case cZ80_V2_MACHINE_TYPE_128_IF1:
                hardware = "128k + Interface 1";
                break;
            case 5:
            case 6:
                hardware = "UNKNOWN";
                break;

            default:
                break;
        }
    }
    else
    {
        switch (hardwareType) {
            case cZ80_V3_MACHINE_TYPE_48:
                hardware = "48k";
                break;
            case cZ80_V3_MACHINE_TYPE_48_IF1:
                hardware = "48k + Interface 1";
                break;
            case cZ80_V3_MACHINE_TYPE_48_MGT:
                hardware = "48k + M.G.T";
                break;
            case cZ80_V3_MACHINE_TYPE_128:
                hardware = "128k";
                break;
            case cZ80_V3_MACHINE_TYPE_128_IF1:
                hardware = "128k + Interface 1";
                break;
            case cz80_V3_MACHINE_TYPE_128_MGT:
                hardware = "128k + M.G.T";
                break;
            case cZ80_V3_MACHINE_TYPE_128_2:
                hardware = "128k +2";
                break;

            case cZ80_V3_MACHINE_TYPE_128_2A:
                hardware = "128k +2A";
                break;

            case cZ80_V3_MACHINE_TYPE_128_3:
                hardware = "128k +3";
                break;

            default:
                break;
        }
    }
    return hardware;
}

// ------------------------------------------------------------------------------------------------------------

int32_t ZXSpectrum::snapshotMachineInSnapshotWithPath(const char *path)
{
    ayEnabledSnapshot = false;

    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        return -1;
    }
    
    std::vector<char> pFileBytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(pFileBytes.data(), pFileBytes.size());

    int32_t machineType = -1;

    if (pFileBytes.size() > 0)
    {
        // Decode the header
        uint16_t headerLength = ((uint16_t *)&pFileBytes[30])[0];
        uint32_t version;
        uint8_t hardwareType;
        uint16_t pc;

        switch (headerLength) {
        case 23:
            version = 2;
            pc = ((uint16_t *)&pFileBytes[32])[0];
            break;
        case 54:
        case 55:
            version = 3;
            pc = ((uint16_t *)&pFileBytes[32])[0];
            break;
        default:
            version = 1;
            pc = ((uint16_t *)&pFileBytes[6])[0];
            break;
        }

        if (pc == 0) {
            version = 2;
        }

        switch (version) {
        case 1:
            machineType = eZXSpectrum48;
            ayEnabledSnapshot = false;
            break;

        case 2:
                hardwareType = ((uint8_t *)&pFileBytes[34])[0];
                switch (hardwareType) {
                    case 0:
                    case 1:
                        machineType = eZXSpectrum48;
                        break;
                    
                    case 3:
                    case 4:
                        machineType = eZXSpectrum128;
                        break;
                        
                    default:
                        machineType = eZXSpectrum48;
                        break;
                }
                IsAYSnapshot(((uint8_t*)&pFileBytes[37])[0]);

            break;

        case 3:
                hardwareType = ((uint8_t *)&pFileBytes[34])[0];
                switch (hardwareType) {
                    case cZ80_V3_MACHINE_TYPE_48:
                    case cZ80_V3_MACHINE_TYPE_48_IF1:
                    case cZ80_V3_MACHINE_TYPE_48_MGT:
                        machineType = eZXSpectrum48;
                        break;
                        
                    case cZ80_V3_MACHINE_TYPE_128:
                    case cZ80_V3_MACHINE_TYPE_128_IF1:
                    case cz80_V3_MACHINE_TYPE_128_MGT:
                        machineType = eZXSpectrum128;
                        break;

                    case cZ80_V3_MACHINE_TYPE_128_2:
                        machineType = eZXSpectrum128_2;
                        break;

                    case cZ80_V3_MACHINE_TYPE_128_2A:
                        machineType = eZXSpectrum128_2A;
                        break;

                    case cZ80_V3_MACHINE_TYPE_128_3:
                        machineType = eZXSpectrum128_3;
                        break;

                    default:
                        break;
                }
                IsAYSnapshot(((uint8_t*)&pFileBytes[37])[0]);

            break;
        }
    }

    return machineType;
}

bool ZXSpectrum::IsAYSnapshot(uint8_t infoByte)
{
    if (infoByte & 4)
    {
        ayEnabledSnapshot = true;
    }
    else
    {
        ayEnabledSnapshot = false;
    }
    return ayEnabledSnapshot;
}


