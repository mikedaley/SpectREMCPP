#include "Z80Core.h"
#include <cstdint>

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IY_BC(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIY, m_CPURegisters.reg_pairs.regBC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IY_DE(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIY, m_CPURegisters.reg_pairs.regDE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IY_nn(uint8_t)
{
    m_CPURegisters.regs.regIYl = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regIYh = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_IY(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regIYl);
    Z80CoreMemWrite(m_MEMPTR, m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IY(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regIY++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IYh(uint8_t)
{
    Inc(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IYh(uint8_t)
{
    Dec(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_n(uint8_t)
{
    m_CPURegisters.regs.regIYh = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IY_IY(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIY, m_CPURegisters.reg_pairs.regIY);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IY_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regIYl = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regs.regIYh = Z80CoreMemRead(m_MEMPTR);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IY(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regIY--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_IYl(uint8_t)
{
    Inc(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_IYl(uint8_t)
{
    Dec(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_n(uint8_t)
{
    m_CPURegisters.regs.regIYl = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regIY + offset, 1);
    Inc(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regIY + offset, 1);
    Dec(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_n(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    uint8_t val = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, val);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_IY_SP(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regIY, m_CPURegisters.regSP);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_IYh(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_IYl(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_IYh(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_IYl(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_IYh(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_IYl(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_IYh(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_IYl(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_B(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_C(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_D(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_E(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_IYh(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_IYl(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYh_A(uint8_t)
{
    m_CPURegisters.regs.regIYh = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_B(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_C(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_D(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_E(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_IYh(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_IYl(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_IYl_A(uint8_t)
{
    m_CPURegisters.regs.regIYl = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_B(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_C(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_D(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_E(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_H(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_L(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_IY_d_A(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regIY + offset, m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_IYh(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regIYh;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_IYl(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regIYl;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_IYh(uint8_t)
{
    Add8(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_IYl(uint8_t)
{
    Add8(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Add8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_IYh(uint8_t)
{
    Adc8(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_IYl(uint8_t)
{
    Adc8(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Adc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_IYh(uint8_t)
{
    Sub8(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_IYl(uint8_t)
{
    Sub8(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Sub8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_IYh(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_IYl(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Sbc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_IYh(uint8_t)
{
    And(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_IYl(uint8_t)
{
    And(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    And(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_IYh(uint8_t)
{
    Xor(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_IYl(uint8_t)
{
    Xor(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Xor(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_IYh(uint8_t)
{
    Or(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_IYl(uint8_t)
{
    Or(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Or(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_IYh(uint8_t)
{
    Cp(m_CPURegisters.regs.regIYh);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_IYl(uint8_t)
{
    Cp(m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_off_IY_d(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regIY + offset);
    Cp(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_IY(uint8_t)
{
    m_CPURegisters.regs.regIYl = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regIYh = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EX_off_SP_IY(uint8_t)
{
    uint8_t tl = Z80CoreMemRead(m_CPURegisters.regSP + 0);
    uint8_t th = Z80CoreMemRead(m_CPURegisters.regSP + 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP + 1, 1);
    Z80CoreMemWrite(m_CPURegisters.regSP + 1, m_CPURegisters.regs.regIYh);
    Z80CoreMemWrite(m_CPURegisters.regSP + 0, m_CPURegisters.regs.regIYl);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    m_CPURegisters.regs.regIYh = th;
    m_CPURegisters.regs.regIYl = tl;

    m_MEMPTR = m_CPURegisters.reg_pairs.regIY;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_IY(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regIYh);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regIYl);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_off_IY(uint8_t)
{
    m_CPURegisters.regPC = m_CPURegisters.reg_pairs.regIY;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_SP_IY(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.regSP = m_CPURegisters.reg_pairs.regIY;
}

//-----------------------------------------------------------------------------------------

