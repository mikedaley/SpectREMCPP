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
const uint8_t               cZ80_V3_MACHINE_TYPE_128_2 = 12;


// - SNA

ZXSpectrum::Snap ZXSpectrum::snapshotCreateSNA()
{
    // We don't want the core running when we take a snapshot
    pause();

    Snap snap;
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

bool ZXSpectrum::snapshotSNALoadWithPath(const char *path)
{
    FILE *fileHandle;

    fileHandle = fopen(path, "rb");

    if (!fileHandle)
    {
        std::cout << "ERROR LOADING SNAPSHOT: " << path << std::endl;
        return false;
    }
    std::cout << "Loading SNA snapshot: " << path << std::endl;

    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    fseek(fileHandle, 0, SEEK_END);
    size_t size = static_cast<size_t>( ftell(fileHandle) );
    fseek(fileHandle, 0, SEEK_SET);

    uint8_t *pFileBytes = new uint8_t[size];

    if (pFileBytes != nullptr)
    {
        fread(pFileBytes, 1, size, fileHandle);

        // Decode the header
        z80Core.SetRegister(CZ80Core::eREG_I, pFileBytes[0]);
        z80Core.SetRegister(CZ80Core::eREG_R, pFileBytes[20]);
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

        delete[] pFileBytes;
    }

    resume();

    return true;

}

// - Z80

ZXSpectrum::Snap ZXSpectrum::snapshotCreateZ80()
{
    pause();

    int snapshotSize = 0;
    if (machineInfo.machineType == eZXSpectrum48)
    {
        snapshotSize = (48 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 3);
    }
    else if (machineInfo.machineType == eZXSpectrum128 || machineInfo.machineType == eZXSpectrum128_2)
    {
        snapshotSize = (128 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 8);
    }
    else
    {
        std::cout << "Unknown machine type" << std::endl;
        exit(1);
    }

    // Structure to be returned containing the length and size of the snapshot
    Snap snapData;
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

    if (machineInfo.machineType == eZXSpectrum48)
    {
        snapData.data[34] = cZ80_V2_MACHINE_TYPE_48;
    }
    else if (machineInfo.machineType == eZXSpectrum128)
    {
        snapData.data[34] = cZ80_V3_MACHINE_TYPE_128;
    }
    else if (machineInfo.machineType == eZXSpectrum128_2)
    {
        snapData.data[34] = cZ80_V3_MACHINE_TYPE_128_2;
    }

    if (machineInfo.machineType == eZXSpectrum128 || machineInfo.machineType == eZXSpectrum128_2)
    {
        snapData.data[35] = ULAPortnnFDValue; // last 128k 0x7ffd port value
    }
    else
    {
        snapData.data[35] = 0;
    }

    snapData.data[36] = 0; // Interface 1 ROM
    snapData.data[37] = 4; // AY Sound
    snapData.data[38] = ULAPortnnFDValue; // Last OUT fffd

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

    uint32_t snapPtr = cZ80_V3_HEADER_SIZE;

    if (machineInfo.machineType == eZXSpectrum48)
    {
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
    }
    else if (machineInfo.machineType == eZXSpectrum128)
    {
        // 128k/Next
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
    }

    resume();

    return snapData;
}

bool ZXSpectrum::snapshotZ80LoadWithPath(const char *path)
{
    FILE *fileHandle;
    bool bSuccess = false;

    fileHandle = fopen(path, "rb");

    if (!fileHandle)
    {
        std::cout << "ERROR LOADING Z80 SNAPSHOT: " << path << std::endl;
        fclose(fileHandle);
        return false;
    }

    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    fseek(fileHandle, 0, SEEK_END);
    size_t size = static_cast<size_t>( ftell(fileHandle) );
    fseek(fileHandle, 0, SEEK_SET);

    uint8_t *pFileBytes = new uint8_t[ size ];

    if (pFileBytes != nullptr)
    {
        fread(pFileBytes, 1, size, fileHandle);

        // Decode the header
        uint16_t headerLength = (reinterpret_cast<uint16_t *>( &pFileBytes[30])[0] );
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
        std::cout << "Loading Z80 snapshot v" << version << ": " << path << std::endl;

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
    //    int fileBytesIndex = 39;
    //    for (int i = 0; i < 16; i++)
    //    {
    //        audioAYSetRegister(i);
    //        audioAYWriteData(pFileBytes[ fileBytesIndex++ ]);
    //    }

    //    audioAYSetRegister(pFileBytes[38]);

    // Based on the version number of the snapshot, decode the memory contents
        switch (version) {
        case 1:
            snapshotExtractMemoryBlock(pFileBytes, 0x4000, 30, compressed, 0xc000);
            bSuccess = true;
            break;

        case 2:
        case 3:
            hardwareType = reinterpret_cast<uint8_t *>(&pFileBytes[34])[0];
            uint16_t additionHeaderBlockLength = 0;
            additionHeaderBlockLength = reinterpret_cast<uint16_t *>(&pFileBytes[30])[0];
            uint16_t offset = 32 + additionHeaderBlockLength;

            if ((version == 2 && (hardwareType == cZ80_V2_MACHINE_TYPE_128 || hardwareType == cZ80_V2_MACHINE_TYPE_128_IF1)) ||
                (version == 3 && (hardwareType == cZ80_V3_MACHINE_TYPE_128 || hardwareType == cZ80_V3_MACHINE_TYPE_128_IF1 || hardwareType == cz80_V3_MACHINE_TYPE_128_MGT ||
                    hardwareType == cZ80_V3_MACHINE_TYPE_128_2)))
            {
                // Decode byte 35 so that port 0x7ffd can be set on the 128k
                uint8_t data = reinterpret_cast<uint8_t *>(&pFileBytes[35])[0];
                emuDisablePaging = ((data & 0x20) == 0x20) ? true : false;
                emuROMPage = ((data & 0x10) == 0x10) ? 1 : 0;
                emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
                emuRAMPage = (data & 0x07);
            }
            else
            {
                emuDisablePaging = true;
                emuROMPage = 0;
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

                uint32_t pageId = pFileBytes[offset + 2];

                if (version == 1 || ((version == 2 || version == 3) && (hardwareType == cZ80_V2_MACHINE_TYPE_48 || hardwareType == cZ80_V3_MACHINE_TYPE_48 ||
                    hardwareType == cZ80_V2_MACHINE_TYPE_48_IF1 || hardwareType == cZ80_V3_MACHINE_TYPE_48_IF1)
                    ))
                {
                    // 48k
                    switch (pageId) {
                    case 4:
                        snapshotExtractMemoryBlock(pFileBytes, 0x8000, offset + 3, isCompressed, 0x4000);
                        break;
                    case 5:
                        snapshotExtractMemoryBlock(pFileBytes, 0xc000, offset + 3, isCompressed, 0x4000);
                        break;
                    case 8:
                        snapshotExtractMemoryBlock(pFileBytes, 0x4000, offset + 3, isCompressed, 0x4000);
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    // 128k
                    snapshotExtractMemoryBlock(pFileBytes, (pageId - 3) * 0x4000, offset + 3, isCompressed, 0x4000);
                }

                offset += compressedLength + 3;
            }

            bSuccess = true;
            break;
        }
    }

    resume();

    return bSuccess;
}

void ZXSpectrum::snapshotExtractMemoryBlock(uint8_t *fileBytes, uint32_t memAddr, uint32_t fileOffset, bool isCompressed, uint32_t unpackedLength)
{
    uint32_t filePtr = fileOffset;
    uint32_t memoryPtr = memAddr;

    if (!isCompressed)
    {
        while (memoryPtr < unpackedLength + memAddr)
        {
            memoryRam[memoryPtr++] = static_cast<int8_t>(fileBytes[filePtr++]);
        }
    }
    else
    {
        while (memoryPtr < unpackedLength + memAddr)
        {
            uint8_t byte1 = fileBytes[filePtr];
            uint8_t byte2 = fileBytes[filePtr + 1];

            if ((unpackedLength + memAddr) - memoryPtr >= 2 && byte1 == 0xed && byte2 == 0xed)
            {
                uint8_t count = fileBytes[filePtr + 2];
                uint8_t value = fileBytes[filePtr + 3];
                for (int i = 0; i < count; i++)
                {
                    memoryRam[memoryPtr++] = static_cast<int8_t>(value);
                }
                filePtr += 4;
            }
            else
            {
                memoryRam[memoryPtr++] = static_cast<int8_t>(fileBytes[filePtr++]);
            }
        }
    }
}


string ZXSpectrum::snapshotHardwareTypeForVersion(uint32_t version, uint32_t hardwareType)
{
    string hardware = "Unknown";
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

            default:
                break;
        }
    }
    return hardware;
}

int32_t ZXSpectrum::snapshotMachineInSnapshotWithPath(const char *path)
{
    FILE *fileHandle;
    int machineType = -1;

    fileHandle = fopen(path, "rb");

    if (!fileHandle)
    {
        std::cout << "ERROR LOADING SNAPSHOT: " << path << std::endl;
        fclose(fileHandle);
        return -1;
    }

    fseek(fileHandle, 0, SEEK_END);
    long size = ftell(fileHandle);
    fseek(fileHandle, 0, SEEK_SET);

    uint8_t *pFileBytes = new uint8_t[size];

    if (pFileBytes != nullptr)
    {
        fread(pFileBytes, 1, size, fileHandle);

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

        if (pc == 0)
        {
            version = 2;
        }

        switch (version)
        {
        case 1:
            machineType = eZXSpectrum48;
            break;

        case 2:
            hardwareType = ((uint8_t *)&pFileBytes[34])[0];
            if (hardwareType == 0 || hardwareType == 1)
            {
                machineType = eZXSpectrum48;
            }
            else if (hardwareType == 3 || hardwareType == 4)
            {
                machineType = eZXSpectrum128;
            }
            else
            {
                machineType = eZXSpectrum48;
            }
            break;

        case 3:
            hardwareType = ((uint8_t *)&pFileBytes[34])[0];
            if (hardwareType == cZ80_V3_MACHINE_TYPE_48 || hardwareType == cZ80_V3_MACHINE_TYPE_48_IF1 || hardwareType == cZ80_V3_MACHINE_TYPE_48_MGT)
            {
                machineType = eZXSpectrum48;
            }
            else if (hardwareType == cZ80_V3_MACHINE_TYPE_128 || hardwareType == cZ80_V3_MACHINE_TYPE_128_IF1 || hardwareType == cz80_V3_MACHINE_TYPE_128_MGT)
            {
                machineType = eZXSpectrum128;
            }
            else if (hardwareType == cZ80_V3_MACHINE_TYPE_128_2)
            {
                machineType = eZXSpectrum128_2;
            }
            break;
        }

        delete[]pFileBytes;
    }

    return machineType;
}

