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

const uint8_t               kSnaHeaderSize = 27;
const uint16_t              kZ80V3HeaderSize = 86;
const uint16_t              kZ80V3AddHeaderSize = 54;
const uint8_t               kZ80V3PageHeaderSize = 3;

const uint8_t               kZ80V2MachineType_48 = 0;
const uint8_t               kZ80V3MachineType_48 = 0;
const uint8_t               kZ80V2MachineType_48_If1 = 1;
const uint8_t               kZ80V3MachineType_48_If1 = 1;
const uint8_t               kZ80V2MachineType_128 = 3;
const uint8_t               kZ80V3MachineType_48_Mgt = 3;
const uint8_t               kZ80V2MachineType_128_If1 = 4;
const uint8_t               kZ80V3MachineType_128 = 4;
const uint8_t               kZ80V3MachineType_128_If1 = 5;
const uint8_t               kZ80V3MachineType_128_Mgt = 6;
const uint8_t               cZ80V3MachineType_128_2 = 12;

// ------------------------------------------------------------------------------------------------------------
// - SNA functions

ZXSpectrum::snapshot_data ZXSpectrum::snapshotCreateSNA()
{
    // We don't want the core running when we take a snapshot
    pause();

    snapshot_data snap;
    snap.length = (48 * 1024) + kSnaHeaderSize;
    snap.data = new uint8_t[snap.length];

    snap.data[0] = z80_core.GetRegister(CZ80Core::eREG_I);
    snap.data[1] = z80_core.GetRegister(CZ80Core::eREG_ALT_HL) & 0xff;
    snap.data[2] = z80_core.GetRegister(CZ80Core::eREG_ALT_HL) >> 8;

    snap.data[3] = z80_core.GetRegister(CZ80Core::eREG_ALT_DE) & 0xff;
    snap.data[4] = z80_core.GetRegister(CZ80Core::eREG_ALT_DE) >> 8;

    snap.data[5] = z80_core.GetRegister(CZ80Core::eREG_ALT_BC) & 0xff;
    snap.data[6] = z80_core.GetRegister(CZ80Core::eREG_ALT_BC) >> 8;

    snap.data[7] = z80_core.GetRegister(CZ80Core::eREG_ALT_AF) & 0xff;
    snap.data[8] = z80_core.GetRegister(CZ80Core::eREG_ALT_AF) >> 8;

    snap.data[9] = z80_core.GetRegister(CZ80Core::eREG_HL) & 0xff;
    snap.data[10] = z80_core.GetRegister(CZ80Core::eREG_HL) >> 8;

    snap.data[11] = z80_core.GetRegister(CZ80Core::eREG_DE) & 0xff;
    snap.data[12] = z80_core.GetRegister(CZ80Core::eREG_DE) >> 8;

    snap.data[13] = z80_core.GetRegister(CZ80Core::eREG_BC) & 0xff;
    snap.data[14] = z80_core.GetRegister(CZ80Core::eREG_BC) >> 8;

    snap.data[15] = z80_core.GetRegister(CZ80Core::eREG_IY) & 0xff;
    snap.data[16] = z80_core.GetRegister(CZ80Core::eREG_IY) >> 8;

    snap.data[17] = z80_core.GetRegister(CZ80Core::eREG_IX) & 0xff;
    snap.data[18] = z80_core.GetRegister(CZ80Core::eREG_IX) >> 8;

    snap.data[19] = static_cast<uint8_t>( (z80_core.GetIFF1() & 1) << 2 );
    snap.data[20] = z80_core.GetRegister(CZ80Core::eREG_R);

    snap.data[21] = z80_core.GetRegister(CZ80Core::eREG_AF) & 0xff;
    snap.data[22] = z80_core.GetRegister(CZ80Core::eREG_AF) >> 8;

    uint16_t pc = z80_core.GetRegister(CZ80Core::eREG_PC);
    uint16_t sp = z80_core.GetRegister(CZ80Core::eREG_SP) - 2;

    snap.data[23] = sp & 0xff;
    snap.data[24] = sp >> 8;

    snap.data[25] = z80_core.GetIMMode();
    snap.data[26] = display_border_color & 0x07;

    uint32_t display_index = kSnaHeaderSize;
    for (uint32_t addr = 16384; addr < 16384 + (48 * 1024); addr++)
    {
        snap.data[display_index++] = z80_core.Z80CoreDebugMemRead(static_cast<uint16_t>(addr), nullptr);
    }

    // Update the SP location in the snapshot buffer with the new SP, as PC has been added to the stack
    // as part of creating the snapshot
    snap.data[sp - 16384 + kSnaHeaderSize] = pc & 0xff;
    snap.data[sp - 16384 + kSnaHeaderSize + 1] = pc >> 8;

    resume();

    return snap;
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum::FileResponse ZXSpectrum::snapshotSNALoadWithPath(const std::string path)
{
    
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        char* error_string = strerror(errno);
        FileResponse response;
        return FileResponse{false, error_string};
    }
    
    std::vector<char> pFileBytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(pFileBytes.data(), pFileBytes.size());

    std::cout << "Loading SNA snapshot: " << "\n";
    
    return snapshotSNALoadWithBuffer(pFileBytes.data(), pFileBytes.size());
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum::FileResponse ZXSpectrum::snapshotSNALoadWithBuffer(const char *buffer, size_t size)
{
    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    std::vector<char> pFileBytes(buffer, buffer + size);

    if (size > 0)
    {
        // Decode the header
        z80_core.SetRegister(CZ80Core::eREG_I, buffer[0]);
        z80_core.SetRegister(CZ80Core::eREG_R, buffer[20]);
        z80_core.SetRegister(CZ80Core::eREG_ALT_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[0]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[1]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[2]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_AF, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[3]));
        z80_core.SetRegister(CZ80Core::eREG_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[4]));
        z80_core.SetRegister(CZ80Core::eREG_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[5]));
        z80_core.SetRegister(CZ80Core::eREG_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[6]));
        z80_core.SetRegister(CZ80Core::eREG_IY, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[7]));
        z80_core.SetRegister(CZ80Core::eREG_IX, (reinterpret_cast<uint16_t *>(&pFileBytes[1])[8]));
        z80_core.SetRegister(CZ80Core::eREG_AF, (reinterpret_cast<uint16_t *>(&pFileBytes[21])[0]));
        z80_core.SetRegister(CZ80Core::eREG_SP, (reinterpret_cast<uint16_t *>(&pFileBytes[21])[1]));

        // Border colour
        display_border_color = pFileBytes[26] & 0x07;

        // Set the IM
        z80_core.SetIMMode(pFileBytes[25]);

        // Do both on bit 2 as a RETN copies IFF2 to IFF1
        z80_core.SetIFF1((pFileBytes[19] >> 2) & 1);
        z80_core.SetIFF2((pFileBytes[19] >> 2) & 1);

        if (size == (48 * 1024) + kSnaHeaderSize)
        {
            uint32_t sna_address = kSnaHeaderSize;
            for (uint32_t i = 16384; i < (64 * 1024); i++)
            {
                memory_ram[i] = static_cast<char>( pFileBytes[sna_address++] );
            }

            // Set the PC
            uint8_t pc_lsb = static_cast<uint8_t>(memory_ram[z80_core.GetRegister(CZ80Core::eREG_SP)]);
            uint8_t pc_msb = static_cast<uint8_t>(memory_ram[(z80_core.GetRegister(CZ80Core::eREG_SP) + 1)]);
            z80_core.SetRegister(CZ80Core::eREG_PC, static_cast<uint16_t>((pc_msb << 8) | pc_lsb));
            z80_core.SetRegister(CZ80Core::eREG_SP, z80_core.GetRegister(CZ80Core::eREG_SP) + 2);
        }
    }

    resume();

    return FileResponse{true, "Loaded successfully"};
}

// ------------------------------------------------------------------------------------------------------------
// - Z80 Snapshot functions

/*
 Returning an empty Snap struct means snapshot creation failed
 */
ZXSpectrum::snapshot_data ZXSpectrum::snapshotCreateZ80()
{
    int snapshot_size = 0;
    switch (machine_info.machine_type) {
        case eZXSpectrum48:
            snapshot_size = (48 * 1024) + kZ80V3HeaderSize + (kZ80V3PageHeaderSize * 3);
            break;
            
        case eZXSpectrum128:
        case eZXSpectrum128_2:
            snapshot_size = (128 * 1024) + kZ80V3HeaderSize + (kZ80V3PageHeaderSize * 8);
            break;
            
        default:
            std::cout << "Unknown machine type" << "\n";
            snapshot_data emptySnap;
            emptySnap.length = 0;
            emptySnap.data = nullptr;
            return emptySnap;
            break;
    }

    // We don't want the emulator running while we create a snapshot
    pause();

    // Structure to be returned containing the length and size of the snapshot
    snapshot_data snapshot_data;
    snapshot_data.length = snapshot_size;
    snapshot_data.data = new uint8_t[ snapshot_size ];

    // Header
    snapshot_data.data[0] = z80_core.GetRegister(CZ80Core::eREG_A);
    snapshot_data.data[1] = z80_core.GetRegister(CZ80Core::eREG_F);
    snapshot_data.data[2] = z80_core.GetRegister(CZ80Core::eREG_BC) & 0xff;
    snapshot_data.data[3] = z80_core.GetRegister(CZ80Core::eREG_BC) >> 8;
    snapshot_data.data[4] = z80_core.GetRegister(CZ80Core::eREG_HL) & 0xff;
    snapshot_data.data[5] = z80_core.GetRegister(CZ80Core::eREG_HL) >> 8;
    snapshot_data.data[6] = 0x0; // PC
    snapshot_data.data[7] = 0x0;
    snapshot_data.data[8] = z80_core.GetRegister(CZ80Core::eREG_SP) & 0xff;
    snapshot_data.data[9] = z80_core.GetRegister(CZ80Core::eREG_SP) >> 8;
    snapshot_data.data[10] = z80_core.GetRegister(CZ80Core::eREG_I);
    snapshot_data.data[11] = z80_core.GetRegister(CZ80Core::eREG_R) & 0x7f;

    uint8_t byte12 = z80_core.GetRegister(CZ80Core::eREG_R) >> 7;
    byte12 |= (display_border_color & 0x07) << 1;
    byte12 &= ~(1 << 5);
    snapshot_data.data[12] = byte12;

    snapshot_data.data[13] = z80_core.GetRegister(CZ80Core::eREG_E);            // E
    snapshot_data.data[14] = z80_core.GetRegister(CZ80Core::eREG_D);            // D
    snapshot_data.data[15] = z80_core.GetRegister(CZ80Core::eREG_ALT_C);        // C'
    snapshot_data.data[16] = z80_core.GetRegister(CZ80Core::eREG_ALT_B);        // B'
    snapshot_data.data[17] = z80_core.GetRegister(CZ80Core::eREG_ALT_E);        // E'
    snapshot_data.data[18] = z80_core.GetRegister(CZ80Core::eREG_ALT_D);        // D'
    snapshot_data.data[19] = z80_core.GetRegister(CZ80Core::eREG_ALT_L);        // L'
    snapshot_data.data[20] = z80_core.GetRegister(CZ80Core::eREG_ALT_H);        // H'
    snapshot_data.data[21] = z80_core.GetRegister(CZ80Core::eREG_ALT_A);        // A'
    snapshot_data.data[22] = z80_core.GetRegister(CZ80Core::eREG_ALT_F);        // F'
    snapshot_data.data[23] = z80_core.GetRegister(CZ80Core::eREG_IY) & 0xff;    // IY
    snapshot_data.data[24] = z80_core.GetRegister(CZ80Core::eREG_IY) >> 8;      //
    snapshot_data.data[25] = z80_core.GetRegister(CZ80Core::eREG_IX) & 0xff;    // IX
    snapshot_data.data[26] = z80_core.GetRegister(CZ80Core::eREG_IX) >> 8;      //
    snapshot_data.data[27] = (z80_core.GetIFF1()) ? 0xff : 0x0;
    snapshot_data.data[28] = (z80_core.GetIFF2()) ? 0xff : 0x0;
    snapshot_data.data[29] = z80_core.GetIMMode() & 0x03;                       // IM Mode

    // Version 3 Additional Header
    snapshot_data.data[30] = (kZ80V3AddHeaderSize) & 0xff;                 // Additional Header Length
    snapshot_data.data[31] = (kZ80V3AddHeaderSize) >> 8;

    snapshot_data.data[32] = z80_core.GetRegister(CZ80Core::eREG_PC) & 0xff;    // PC
    snapshot_data.data[33] = z80_core.GetRegister(CZ80Core::eREG_PC) >> 8;

    if (machine_info.machine_type == eZXSpectrum48)
    {
        snapshot_data.data[34] = kZ80V2MachineType_48;
    }
    else if (machine_info.machine_type == eZXSpectrum128)
    {
        snapshot_data.data[34] = kZ80V3MachineType_128;
    }
    else if (machine_info.machine_type == eZXSpectrum128_2)
    {
        snapshot_data.data[34] = cZ80V3MachineType_128_2;
    }

    if (machine_info.machine_type == eZXSpectrum128 || machine_info.machine_type == eZXSpectrum128_2)
    {
        snapshot_data.data[35] = ula_port_nnfd_value; // last 128k 0x7ffd port value
    }
    else
    {
        snapshot_data.data[35] = 0;
    }

    snapshot_data.data[36] = 0; // Interface 1 ROM
    snapshot_data.data[37] = 4; // AY Sound
    snapshot_data.data[38] = ula_port_nnfd_value; // Last OUT fffd

    // Save the AY register values
    int data_index = 39;
    for (uint8_t i = 0; i < 16; i++)
    {
        audioAYSetRegister(i);
        snapshot_data.data[data_index++] = audioAYReadData();
    }

    uint32_t quarter_ts = machine_info.ts_per_frame / 4;
    uint32_t low_ts = quarter_ts - (z80_core.GetTStates() % quarter_ts) - 1;
    snapshot_data.data[55] = low_ts & 0xff;
    snapshot_data.data[56] = static_cast<uint8_t>(low_ts >> 8);

    snapshot_data.data[57] = ((z80_core.GetTStates() / quarter_ts) + 3) % 4;
    snapshot_data.data[58] = 0; // QL Emu
    snapshot_data.data[59] = 0; // MGT Paged ROM
    snapshot_data.data[60] = 0; // Multiface ROM paged
    snapshot_data.data[61] = 0; // 0 - 8192 ROM
    snapshot_data.data[63] = 0; // 8192 - 16384 ROM
    snapshot_data.data[83] = 0; // MGT Type
    snapshot_data.data[84] = 0; // Disciple inhibit button
    snapshot_data.data[85] = 0; // Disciple inhibit flag

    uint32_t snapshot_pointer = kZ80V3HeaderSize;

    if (machine_info.machine_type == eZXSpectrum48)
    {
        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 4;

        for (uint32_t memory_address = 0x8000; memory_address <= 0xbfff; memory_address++)
        {
            snapshot_data.data[snapshot_pointer++] = z80_core.Z80CoreDebugMemRead(static_cast<uint16_t>(memory_address), nullptr);
        }

        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 5;

        for (uint32_t memory_address = 0xc000; memory_address <= 0xffff; memory_address++)
        {
            snapshot_data.data[snapshot_pointer++] = z80_core.Z80CoreDebugMemRead(static_cast<uint16_t>(memory_address), nullptr);
        }

        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 0xff;
        snapshot_data.data[snapshot_pointer++] = 8;

        for (uint32_t memory_address = 0x4000; memory_address <= 0x7fff; memory_address++)
        {
            snapshot_data.data[snapshot_pointer++] = z80_core.Z80CoreDebugMemRead(static_cast<uint16_t>(memory_address), nullptr);
        }
    }
    else if (machine_info.machine_type == eZXSpectrum128)
    {
        // 128k/Next
        for (uint8_t page = 0; page < 8; page++)
        {
            snapshot_data.data[snapshot_pointer++] = 0xff;
            snapshot_data.data[snapshot_pointer++] = 0xff;
            snapshot_data.data[snapshot_pointer++] = page + 3;

            for (uint32_t memory_address = page * 0x4000ul; memory_address < (page * 0x4000ul) + 0x4000ul; memory_address++)
            {
                snapshot_data.data[snapshot_pointer++] = static_cast<uint8_t>(memory_ram[static_cast<size_t>(memory_address)]);
            }
        }
    }

    resume();

    return snapshot_data;
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum::FileResponse ZXSpectrum::snapshotZ80LoadWithPath(const std::string path)
{
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        char* error_string = strerror(errno);
        FileResponse response;
        return FileResponse{false, error_string};
    }
    
    std::vector<char> pFileBytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(pFileBytes.data(), pFileBytes.size());
    
    return snapshotZ80LoadWithBuffer(pFileBytes.data(), pFileBytes.size());
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum::FileResponse ZXSpectrum::snapshotZ80LoadWithBuffer(const char *buffer, size_t size)
{
    pause();
    displayFrameReset();
    displayClear();
    audioReset();

    std::vector<char> pFileBytes(buffer, buffer + size);

    if (size > 0)
    {
        // Decode the header
        uint16_t header_length = (reinterpret_cast<uint16_t *>(&pFileBytes[30])[0] );
        uint16_t version;
        uint16_t hardware_type;
        uint16_t pc;

        switch (header_length) {
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

        z80_core.SetRegister(CZ80Core::eREG_A, pFileBytes[0]);
        z80_core.SetRegister(CZ80Core::eREG_F, pFileBytes[1]);
        z80_core.SetRegister(CZ80Core::eREG_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[2])[0]));
        z80_core.SetRegister(CZ80Core::eREG_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[2])[1]));
        z80_core.SetRegister(CZ80Core::eREG_PC, pc);
        z80_core.SetRegister(CZ80Core::eREG_SP, (reinterpret_cast<uint16_t *>(&pFileBytes[8])[0]));
        z80_core.SetRegister(CZ80Core::eREG_I, static_cast<uint8_t>(pFileBytes[10]));
        z80_core.SetRegister(CZ80Core::eREG_R, static_cast<uint8_t>((pFileBytes[11] & 127) | ((pFileBytes[12] & 1) << 7)));

        // Decode byte 12
        //    Bit 0  : Bit 7 of the R-register
        //    Bit 1-3: Border colour
        //    Bit 4  : 1=Basic SamRom switched in
        //    Bit 5  : 1=Block of data is compressed
        //    Bit 6-7: No meaning
        uint8_t byte12 = pFileBytes[12];

        // For campatibility reasons if byte 12 = 255 then it should be assumed to = 1
        byte12 = (byte12 == 255) ? 1 : byte12;

        display_border_color = (byte12 & 14) >> 1;
        bool compressed = (byte12 & 32) != 0 ? true : false;

        z80_core.SetRegister(CZ80Core::eREG_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[0]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_BC, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[1]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_DE, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[2]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_HL, (reinterpret_cast<uint16_t *>(&pFileBytes[13])[3]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_A, static_cast<uint8_t>(pFileBytes[21]));
        z80_core.SetRegister(CZ80Core::eREG_ALT_F, static_cast<uint8_t>(pFileBytes[22]));
        z80_core.SetRegister(CZ80Core::eREG_IY, (reinterpret_cast<uint16_t *>(&pFileBytes[23])[0]));
        z80_core.SetRegister(CZ80Core::eREG_IX, (reinterpret_cast<uint16_t *>(&pFileBytes[23])[1]));
        z80_core.SetIFF1(static_cast<uint8_t>(pFileBytes[27] & 1));
        z80_core.SetIFF2(static_cast<uint8_t>(pFileBytes[28] & 1));
        z80_core.SetIMMode(static_cast<uint8_t>(pFileBytes[29] & 3));

        // Load AY register values
        uint32_t files_bytes_index = 39;
        for (uint32_t i = 0; i < 16; i++)
        {
            audioAYSetRegister(i);
            audioAYWriteData(pFileBytes[ files_bytes_index++ ]);
        }

        audioAYSetRegister(pFileBytes[38]);

        // Based on the version number of the snapshot, decode the memory contents
        switch (version) {
        case 1:
            snapshotExtractMemoryBlock(buffer, size, 0x4000, 30, compressed, 0xc000);
            break;

        case 2:
        case 3:
            hardware_type = reinterpret_cast<uint8_t *>(&pFileBytes[34])[0];
            uint16_t additional_header_block_length = 0;
            additional_header_block_length = reinterpret_cast<uint16_t *>(&pFileBytes[30])[0];
            uint32_t offset = 32 + additional_header_block_length;

            if ((version == 2 && (hardware_type == kZ80V2MachineType_128 || hardware_type == kZ80V2MachineType_128_If1)) ||
                (version == 3 && (hardware_type == kZ80V3MachineType_128 || hardware_type == kZ80V3MachineType_128_If1 ||
                                  hardware_type == kZ80V3MachineType_128_Mgt ||hardware_type == cZ80V3MachineType_128_2)))
            {
                // Decode byte 35 so that port 0x7ffd can be set on the 128k
                uint8_t data = reinterpret_cast<uint8_t *>(&pFileBytes[35])[0];
                emu_disable_paging = ((data & 0x20) == 0x20) ? true : false;
                emu_rom_page = ((data & 0x10) == 0x10) ? 1 : 0;
                emu_display_page = ((data & 0x08) == 0x08) ? 7 : 5;
                emu_ram_page = (data & 0x07);
            }
            else
            {
                emu_disable_paging = true;
                emu_rom_page = 0;
                emu_ram_page = 0;
                emu_display_page = 1;
            }

            while (offset < size)
            {
                uint32_t compressed_length = reinterpret_cast<uint16_t*>(&pFileBytes[offset])[0];
                bool isCompressed = true;
                if (compressed_length == 0xffff)
                {
                    compressed_length = 0x4000;
                    isCompressed = false;
                }

                uint32_t page_id = buffer[offset + 2];
                
                std::cout << "Snap Page: " << page_id << "\t\t";
//                std::cout << ((pageId > 9) ? "\t" : "\t\t");
                std::cout << "Mem Page: " << page_id - 3 << "\tCompressed Length: " << compressed_length << "\tIsCompressed: " << isCompressed;
                std::cout << "\tHardware Type: " << snapshotHardwareTypeForVersion(version, hardware_type) << "\n";

                if (version == 1 || ((version == 2 || version == 3) && (hardware_type == kZ80V2MachineType_48 ||
                                                                        hardware_type == kZ80V3MachineType_48 ||
                                                                        hardware_type == kZ80V2MachineType_48_If1 ||
                                                                        hardware_type == kZ80V3MachineType_48_If1)))
                {
                    // 48k
                    switch (page_id) {
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
                else
                {
                    // 128k
                    snapshotExtractMemoryBlock(buffer, size, (page_id - 3) * 0x4000, offset + 3, isCompressed, 0x4000);
                }

                offset += compressed_length + 3;
            }

            break;
        }
    }

    resume();
    
    return FileResponse{true, "Loaded successfully"};
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::snapshotExtractMemoryBlock(const char *buffer, size_t buffer_size, uint32_t memory_address, uint32_t file_offset, bool is_compressed, uint32_t unpacked_length)
{
    uint32_t file_pointer = file_offset;
    uint32_t memory_pointer = memory_address;
    std::vector<char> fileBytes(buffer, buffer + buffer_size);

    if (!is_compressed)
    {
        while (memory_pointer < unpacked_length + memory_address)
        {
            memory_ram[memory_pointer++] = static_cast<int8_t>(fileBytes[file_pointer++]);
        }
    }
    else
    {
        while (memory_pointer < unpacked_length + memory_address)
        {
            uint8_t byte1 = fileBytes[file_pointer];
            uint8_t byte2 = fileBytes[file_pointer + 1];

            if ((unpacked_length + memory_address) - memory_pointer >= 2 && byte1 == 0xed && byte2 == 0xed)
            {
                uint8_t count = fileBytes[file_pointer + 2];
                uint8_t value = fileBytes[file_pointer + 3];
                for (uint32_t i = 0; i < count; i++)
                {
                    memory_ram[memory_pointer++] = static_cast<int8_t>(value);
                }
                file_pointer += 4;
            }
            else
            {
                memory_ram[memory_pointer++] = static_cast<int8_t>(fileBytes[file_pointer++]);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

std::string ZXSpectrum::snapshotHardwareTypeForVersion(uint32_t version, uint32_t hardware_type)
{
    std::string hardware = "Unknown";
    if (version == 2)
    {
        switch (hardware_type) {
            case kZ80V2MachineType_48:
                hardware = "48k";
                break;
            case kZ80V2MachineType_48_If1:
                hardware = "48k + Interface 1";
                break;
            case kZ80V2MachineType_128:
                hardware = "128k";
                break;
            case kZ80V2MachineType_128_If1:
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
        switch (hardware_type) {
            case kZ80V3MachineType_48:
                hardware = "48k";
                break;
            case kZ80V3MachineType_48_If1:
                hardware = "48k + Interface 1";
                break;
            case kZ80V3MachineType_48_Mgt:
                hardware = "48k + M.G.T";
                break;
            case kZ80V3MachineType_128:
                hardware = "128k";
                break;
            case kZ80V3MachineType_128_If1:
                hardware = "128k + Interface 1";
                break;
            case kZ80V3MachineType_128_Mgt:
                hardware = "128k + M.G.T";
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
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.ios_base::good()) {
        return -1;
    }
    
    std::vector<char> p_file_bytes(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(p_file_bytes.data(), p_file_bytes.size());

    int32_t machine_type = -1;

    if (p_file_bytes.size() > 0)
    {
        // Decode the header
        uint16_t headerLength = ((uint16_t *)&p_file_bytes[30])[0];
        uint32_t version;
        uint8_t hardwareType;
        uint16_t pc;

        switch (headerLength) {
        case 23:
            version = 2;
            pc = ((uint16_t *)&p_file_bytes[32])[0];
            break;
        case 54:
        case 55:
            version = 3;
            pc = ((uint16_t *)&p_file_bytes[32])[0];
            break;
        default:
            version = 1;
            pc = ((uint16_t *)&p_file_bytes[6])[0];
            break;
        }

        if (pc == 0) {
            version = 2;
        }

        hardwareType = ((uint8_t *)&p_file_bytes[34])[0];
        switch (version) {
        case 1:
            machine_type = eZXSpectrum48;
            break;

        case 2:
                switch (hardwareType) {
                    case 0:
                    case 1:
                        machine_type = eZXSpectrum48;
                        break;
                    
                    case 3:
                    case 4:
                        machine_type = eZXSpectrum128;
                        break;
                        
                    default:
                        machine_type = eZXSpectrum48;
                        break;
                }
            break;

        case 3:
                switch (hardwareType) {
                    case kZ80V3MachineType_48:
                    case kZ80V3MachineType_48_If1:
                    case kZ80V3MachineType_48_Mgt:
                        machine_type = eZXSpectrum48;
                        break;
                        
                    case kZ80V3MachineType_128:
                    case kZ80V3MachineType_128_If1:
                    case kZ80V3MachineType_128_Mgt:
                    case cZ80V3MachineType_128_2:
                        machine_type = eZXSpectrum128;
                    default:
                        break;
                }
            break;
        }
    }

    return machine_type;
}

