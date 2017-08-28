//
//  Snapshot.cpp
//  SpectREM
//
//  Created by Mike Daley on 26/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include <stdio.h>
#include "ZXSpectrum.hpp"

void ZXSpectrum::loadZ80SnapshotWithPath(const char *path)
{
    FILE *fileHandle;

    fileHandle = fopen(path, "rb");
    
    if (!fileHandle)
    {
        cout << "ERROR LOADING SNAPSHOT: " << path << endl;
        fclose(fileHandle);
        return;
    }
    
    z80Core.Reset();
    
    fseek(fileHandle, 0, SEEK_END);
    long size = ftell(fileHandle);
    fseek(fileHandle, 0, SEEK_SET);

    unsigned char fileBytes[size];
    
    fread(&fileBytes, 1, size, fileHandle);
    
    // Decode the header
    unsigned short headerLength = ((unsigned short *)&fileBytes[30])[0];
    int version;
    unsigned char hardwareType;
    unsigned short pc;
    
    switch (headerLength) {
        case 23:
            version = 2;
            pc = ((unsigned short *)&fileBytes[32])[0];
            break;
        case 54:
        case 55:
            version = 3;
            pc = ((unsigned short *)&fileBytes[32])[0];
            break;
        default:
            version = 1;
            pc = ((unsigned short *)&fileBytes[6])[0];
            break;
    }
    
    if (pc == 0)
    {
        version = 2;
        pc = ((unsigned short *)&fileBytes[32])[0];
    }
    
    cout << "---------------------------" << endl;
    cout << "Z80 Snapshot Version " << version << endl;
    
    z80Core.SetRegister(CZ80Core::eREG_A, (unsigned char)fileBytes[0]);
    z80Core.SetRegister(CZ80Core::eREG_F, (unsigned char)fileBytes[1]);
    z80Core.SetRegister(CZ80Core::eREG_BC, ((unsigned short *)&fileBytes[2])[0]);
    z80Core.SetRegister(CZ80Core::eREG_HL, ((unsigned short *)&fileBytes[2])[1]);
    z80Core.SetRegister(CZ80Core::eREG_PC, pc);
    z80Core.SetRegister(CZ80Core::eREG_SP, ((unsigned short *)&fileBytes[8])[0]);
    z80Core.SetRegister(CZ80Core::eREG_I, (unsigned char)fileBytes[10]);
    z80Core.SetRegister(CZ80Core::eREG_R, (fileBytes[11] & 127) | ((fileBytes[12] & 1) << 7));
    
    // Decode byte 12
    //    Bit 0  : Bit 7 of the R-register
    //    Bit 1-3: Border colour
    //    Bit 4  : 1=Basic SamRom switched in
    //    Bit 5  : 1=Block of data is compressed
    //    Bit 6-7: No meaning
    unsigned char byte12 = fileBytes[12];
    
    // For campatibility reasons if byte 12 = 255 then it should be assumed to = 1
    byte12 = (byte12 == 255) ? 1 : byte12;
    
    borderColor = (fileBytes[12] & 14) >> 1;
    bool compressed = fileBytes[12] & 32;
    
    z80Core.SetRegister(CZ80Core::eREG_DE, ((unsigned short *)&fileBytes[13])[0]);
    z80Core.SetRegister(CZ80Core::eREG_ALT_BC, ((unsigned short *)&fileBytes[13])[1]);
    z80Core.SetRegister(CZ80Core::eREG_ALT_DE, ((unsigned short *)&fileBytes[13])[2]);
    z80Core.SetRegister(CZ80Core::eREG_ALT_HL, ((unsigned short *)&fileBytes[13])[3]);
    z80Core.SetRegister(CZ80Core::eREG_ALT_A, (unsigned char)fileBytes[21]);
    z80Core.SetRegister(CZ80Core::eREG_ALT_F, (unsigned char)fileBytes[22]);
    z80Core.SetRegister(CZ80Core::eREG_IY, ((unsigned short *)&fileBytes[23])[0]);
    z80Core.SetRegister(CZ80Core::eREG_IX, ((unsigned short *)&fileBytes[23])[1]);
    z80Core.SetIFF1((unsigned char)fileBytes[27] & 1);
    z80Core.SetIFF2((unsigned char)fileBytes[28] & 1);
    z80Core.SetIMMode((unsigned char)fileBytes[29] & 3);
    
//    NSLog(@"RB7: %i Border: %i SamRom: %i Compressed: %i", byte12 & 1, (byte12 & 14) >> 1, byte12 & 16, byte12 & 32);
//    NSLog(@"IFF1: %i IM Mode: %i", (unsigned char)fileBytes[27] & 1, (unsigned char)fileBytes[29] & 3);
    
    // Based on the version number of the snapshot, decode the memory contents
    switch (version) {
        case 1:
            cout << "Hardware Type: 48k" << endl;
            extractMemoryBlock(fileBytes, 0x4000, 30, compressed, 0xc000);
            break;
            
        case 2:
        case 3:
            hardwareType = ((unsigned char *)&fileBytes[34])[0];
            cout << "Hardware Type: " << hardwareTypeForVersion(version, hardwareType) << endl;
            
            int16_t additionHeaderBlockLength = 0;
            additionHeaderBlockLength = ((unsigned short *)&fileBytes[30])[0];
            int offset = 32 + additionHeaderBlockLength;
            
//            if ( (version == 2 && (hardwareType == 3 || hardwareType == 4)) || (version == 3 && (hardwareType == 4 || hardwareType == 5 || hardwareType == 6 || hardwareType == 9)) )
//            {
//                // Decode byte 35 so that port 0x7ffd can be set on the 128k
//                unsigned char data = ((unsigned char *)&fileBytes[35])[0];
//                machine->disablePaging = ((data & 0x20) == 0x20) ? YES : NO;
//                machine->currentROMPage = ((data & 0x10) == 0x10) ? 1 : 0;
//                machine->displayPage = ((data & 0x08) == 0x08) ? 7 : 5;
//                machine->currentRAMPage = (data & 0x07);
//            }
//            else
//            {
//                machine->disablePaging = YES;
//                machine->currentROMPage = 0;
//                machine->currentRAMPage = 0;
                displayPage = 1;
//            }
            
            while (offset < size)
            {
                int compressedLength = ((unsigned short *)&fileBytes[offset])[0];
                bool isCompressed = true;
                if (compressedLength == 0xffff)
                {
                    compressedLength = 0x4000;
                    isCompressed = false;
                }
                
                int pageId = fileBytes[offset + 2];
                
                if (version == 1 || ((version == 2 || version == 3) && (hardwareType == 0 || hardwareType == 1)))
                {
                    // 48k
                    switch (pageId) {
                        case 4:
                            extractMemoryBlock(fileBytes, 0x8000, offset + 3, isCompressed, 0x4000);
                            break;
                        case 5:
                            extractMemoryBlock(fileBytes, 0xc000, offset + 3, isCompressed, 0x4000);
                            break;
                        case 8:
                            extractMemoryBlock(fileBytes, 0x4000, offset + 3, isCompressed, 0x4000);
                            break;
                        default:
                            break;
                    }
                }
//                else
//                {
//                    // 128k
//                    [self extractMemoryBlock:fileBytes memAddr:((pageId - 3) * 0x4000) fileOffset:offset + 3 compressed:isCompressed unpackedLength:0x4000 intoMachine:machine];
//                }
                
                offset += compressedLength + 3;
            }
            break;
    }
    
}

void ZXSpectrum::extractMemoryBlock(unsigned char *fileBytes, int memAddr, int fileOffset, bool isCompressed, int unpackedLength)
{
    int filePtr = fileOffset;
    int memoryPtr = memAddr - machineInfo.romSize;
    
    if (!isCompressed)
    {
        while (memoryPtr < unpackedLength + (memAddr - machineInfo.romSize))
        {
            memoryRam[memoryPtr++] = fileBytes[filePtr++];
        }
    }
    else
    {
        while (memoryPtr < unpackedLength + (memAddr - machineInfo.romSize))
        {
            unsigned char byte1 = fileBytes[filePtr];
            unsigned char byte2 = fileBytes[filePtr + 1];
            
            if ((unpackedLength + (memAddr - machineInfo.romSize)) - memoryPtr >= 2 && byte1 == 0xed && byte2 == 0xed)
            {
                unsigned char count = fileBytes[filePtr + 2];
                unsigned char value = fileBytes[filePtr + 3];
                for (int i = 0; i < count; i++)
                {
                    memoryRam[memoryPtr++] = value;
                }
                filePtr += 4;
            }
            else
            {
                memoryRam[memoryPtr++] = fileBytes[filePtr++];
            }
        }
    }
}


string ZXSpectrum::hardwareTypeForVersion(int version, int hardwareType)
{
    string hardware = "Unknown";
    if (version == 2)
    {
        switch (hardwareType) {
            case 0:
                hardware = "48k";
                break;
            case 1:
                hardware = "48k + Interface 1";
                break;
            case 2:
                hardware = "SamRam";
                break;
            case 3:
                hardware = "128k";
                break;
            case 4:
                hardware = "128k + Interface 1";
                break;
            case 5:
            case 6:
                break;
                
            default:
                break;
        }
    }
    else
    {
        switch (hardwareType) {
            case 0:
                hardware = "48k";
                break;
            case 1:
                hardware = "48k + Interface 1";
                break;
            case 2:
                hardware = "SamRam";
                break;
            case 3:
                hardware = "48k + M.G.T";
                break;
            case 4:
                hardware = "128k";
                break;
            case 5:
                hardware = "128k + Interface 1";
                break;
            case 6:
                hardware = "128k + M.G.T";
                break;
            case 9:
                hardware = "Next";
                
            default:
                break;
        }
    }
    return hardware;
}


