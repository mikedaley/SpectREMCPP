#include "Z80Core.h"
#include <cstdint>

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IX_BC(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIX, m_CPURegisters.reg_pairs.regBC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IX_DE(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIX, m_CPURegisters.reg_pairs.regDE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IX_nn(uint8_t)
{
    m_CPURegisters.regs.regIXl = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regIXh = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_IX(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regIXl);
    Z80CoreMemWrite(m_MEMPTR, m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IX(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regIX++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IXh(uint8_t)
{
    Inc(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IXh(uint8_t)
{
    Dec(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_n(uint8_t)
{
    m_CPURegisters.regs.regIXh = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IX_IX(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIX, m_CPURegisters.reg_pairs.regIX);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IX_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regIXl = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regs.regIXh = Z80CoreMemRead(m_MEMPTR);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IX(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regIX--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IXl(uint8_t)
{
    Inc(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IXl(uint8_t)
{
    Dec(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_n(uint8_t)
{
    m_CPURegisters.regs.regIXl = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regIX + offset, 1);
    Inc(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regIX + offset, 1);
    Dec(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_n(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    uint8_t val = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, val);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IX_SP(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIX, m_CPURegisters.regSP);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_IXh(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_IXl(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_IXh(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_IXl(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_IXh(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_IXl(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_IXh(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_IXl(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_B(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_C(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_D(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_E(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_IXh(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_IXl(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXh_A(uint8_t)
{
    m_CPURegisters.regs.regIXh = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_B(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_C(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_D(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_E(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_IXh(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_IXl(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IXl_A(uint8_t)
{
    m_CPURegisters.regs.regIXl = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_B(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_C(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_D(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_E(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_H(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_L(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IX_d_A(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIX + offset, m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_IXh(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regIXh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_IXl(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regIXl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_IXh(uint8_t)
{
    Add8(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_IXl(uint8_t)
{
    Add8(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Add8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_IXh(uint8_t)
{
    Adc8(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_IXl(uint8_t)
{
    Adc8(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Adc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_IXh(uint8_t)
{
    Sub8(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_IXl(uint8_t)
{
    Sub8(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Sub8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_IXh(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_IXl(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Sbc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_IXh(uint8_t)
{
    And(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_IXl(uint8_t)
{
    And(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    And(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_IXh(uint8_t)
{
    Xor(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_IXl(uint8_t)
{
    Xor(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Xor(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_IXh(uint8_t)
{
    Or(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_IXl(uint8_t)
{
    Or(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Or(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_IXh(uint8_t)
{
    Cp(m_CPURegisters.regs.regIXh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_IXl(uint8_t)
{
    Cp(m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_off_IX_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIX + offset);
    Cp(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_IX(uint8_t)
{
    m_CPURegisters.regs.regIXl = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regIXh = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EX_off_SP_IX(uint8_t)
{
    uint8_t tl = Z80CoreMemRead(m_CPURegisters.regSP + 0);
    uint8_t th = Z80CoreMemRead(m_CPURegisters.regSP + 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP + 1, 1);
    Z80CoreMemWrite(m_CPURegisters.regSP + 1, m_CPURegisters.regs.regIXh);
    Z80CoreMemWrite(m_CPURegisters.regSP + 0, m_CPURegisters.regs.regIXl);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    m_CPURegisters.regs.regIXh = th;
    m_CPURegisters.regs.regIXl = tl;

    m_MEMPTR = m_CPURegisters.reg_pairs.regIX;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_IX(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regIXh);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regIXl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_off_IX(uint8_t)
{
    m_CPURegisters.regPC = m_CPURegisters.reg_pairs.regIX;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_SP_IX(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    m_CPURegisters.regSP = m_CPURegisters.reg_pairs.regIX;
}

//-----------------------------------------------------------------------------------------

