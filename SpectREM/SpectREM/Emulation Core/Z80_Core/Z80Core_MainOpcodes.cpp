#include "Z80Core.h"
#include <cstdint>

//-----------------------------------------------------------------------------------------

void CZ80Core::NOP(uint8_t)
{
    // Nothing to do...
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_BC_nn(uint8_t)
{
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_BC_A(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regA);

    m_MEMPTR = (m_CPURegisters.reg_pairs.regBC + 1) & 0x00ff;
    m_MEMPTR |= m_CPURegisters.regs.regA << 8;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_BC(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regBC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_B(uint8_t)
{
    Inc(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_B(uint8_t)
{
    Dec(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_n(uint8_t)
{
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLCA(uint8_t)
{
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA << 1) | (m_CPURegisters.regs.regA >> 7);
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & (FLAG_P | FLAG_Z | FLAG_S));
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & 0x01) ? FLAG_C : 0;
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EX_AF_AF_(uint8_t)
{
    uint16_t t = m_CPURegisters.reg_pairs.regAF;
    m_CPURegisters.reg_pairs.regAF = m_CPURegisters.reg_pairs.regAF_;
    m_CPURegisters.reg_pairs.regAF_ = t;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_HL_BC(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regBC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_BC(uint8_t)
{
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.reg_pairs.regBC);
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_BC(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regBC--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_C(uint8_t)
{
    Inc(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_C(uint8_t)
{
    Dec(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_n(uint8_t)
{
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRCA(uint8_t)
{
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA >> 1) | (m_CPURegisters.regs.regA << 7);
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & (FLAG_P | FLAG_Z | FLAG_S));
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & 0x80) ? FLAG_C : 0;
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DJNZ_off_PC_e(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    // See if we should branch
    m_CPURegisters.regs.regB--;

    if (m_CPURegisters.regs.regB != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        m_CPURegisters.regPC += offset;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }

    // Do this here because of the contention
    m_CPURegisters.regPC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_DE_nn(uint8_t)
{
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_DE_A(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regDE, m_CPURegisters.regs.regA);

    m_MEMPTR = (m_CPURegisters.reg_pairs.regDE + 1) & 0x00ff;
    m_MEMPTR |= m_CPURegisters.regs.regA << 8;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_DE(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regDE++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_D(uint8_t)
{
    Inc(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_D(uint8_t)
{
    Dec(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_n(uint8_t)
{
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLA(uint8_t)
{
    uint8_t old_a = m_CPURegisters.regs.regA;
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA << 1) | ((m_CPURegisters.regs.regF & FLAG_C) ? 0x01 : 0x00) ;
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & (FLAG_P | FLAG_Z | FLAG_S));
    m_CPURegisters.regs.regF |= ((old_a & 0x80) == 0x80) ? FLAG_C : 0;
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JR_off_PC_e(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
    Z80CoreMemoryContention(m_CPURegisters.regPC, 1);

    m_CPURegisters.regPC += offset;
    m_CPURegisters.regPC++;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_HL_DE(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regDE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_DE(uint8_t)
{
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.reg_pairs.regDE);
    m_MEMPTR = m_CPURegisters.reg_pairs.regDE + 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_DE(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regDE--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_E(uint8_t)
{
    Inc(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_E(uint8_t)
{
    Dec(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_n(uint8_t)
{
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRA(uint8_t)
{
    uint8_t old_a = m_CPURegisters.regs.regA;
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA >> 1) | ((m_CPURegisters.regs.regF & FLAG_C) ? 0x80 : 0x00);
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & (FLAG_P | FLAG_Z | FLAG_S));
    m_CPURegisters.regs.regF |= ((old_a & 0x01) == 0x01) ? FLAG_C : 0;
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JR_NZ_off_PC_e(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    if ((m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);

        m_CPURegisters.regPC += offset;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }

    m_CPURegisters.regPC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_HL_nn(uint8_t)
{

    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_HL(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regL);
    Z80CoreMemWrite(m_MEMPTR, m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_HL(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regHL++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_H(uint8_t)
{
    Inc(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_H(uint8_t)
{
    Dec(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_n(uint8_t)
{
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DAA(uint8_t)
{
    uint8_t daa_value = 0;
    uint8_t flags = (m_CPURegisters.regs.regF & FLAG_C);

    if ((m_CPURegisters.regs.regA & 0x0f) > 0x09 || (m_CPURegisters.regs.regF & FLAG_H) == FLAG_H)
    {
        daa_value |= 0x06;
    }

    if (m_CPURegisters.regs.regA > 0x99)
    {
        flags = FLAG_C;
        daa_value |= 0x60;
    }
    else if ((m_CPURegisters.regs.regF & FLAG_C) == FLAG_C)
    {
        daa_value |= 0x60;
    }

    if ((m_CPURegisters.regs.regF & FLAG_N) == FLAG_N)
    {
        Sub8(daa_value);
    }
    else
    {
        Add8(daa_value);
    }

    m_CPURegisters.regs.regF &= ~(FLAG_C | FLAG_P);
    m_CPURegisters.regs.regF |= flags;
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regA];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JR_Z_off_PC_e(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    if ((m_CPURegisters.regs.regF & FLAG_Z) == FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);

        m_CPURegisters.regPC += offset;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }

    m_CPURegisters.regPC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_HL_HL(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_HL_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regL = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_MEMPTR);

}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_HL(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.reg_pairs.regHL--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_L(uint8_t)
{
    Inc(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_L(uint8_t)
{
    Dec(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_n(uint8_t)
{
    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CPL(uint8_t)
{
    m_CPURegisters.regs.regA ^= 0xff;
    m_CPURegisters.regs.regF &= (FLAG_C | FLAG_P | FLAG_Z | FLAG_S);
    m_CPURegisters.regs.regF |= (FLAG_N | FLAG_H);
    m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JR_NC_off_PC_e(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    if ((m_CPURegisters.regs.regF & FLAG_C) != FLAG_C)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);

        m_CPURegisters.regPC += offset;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }

    m_CPURegisters.regPC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_SP_nn(uint8_t)
{
    uint8_t t1 = Z80CoreMemRead(m_CPURegisters.regPC++);
    uint8_t t2 = Z80CoreMemRead(m_CPURegisters.regPC++);

    m_CPURegisters.regSP = (((uint16_t)t2) << 8) | t1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_A(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regA);

    m_MEMPTR &= 0x00ff;
    m_MEMPTR |= m_CPURegisters.regs.regA << 8;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_SP(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.regSP++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_off_HL(uint8_t)
{
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Inc(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_off_HL(uint8_t)
{
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Dec(temp);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_n(uint8_t)
{
    uint8_t temp = Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, temp);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SCF(uint8_t)
{
    if ( m_CPUType == eCPUTYPE_Zilog )
    {
        if ( m_PrevOpcodeFlags & OPCODEFLAG_AltersFlags )
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z);
        }
        else
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z | FLAG_3 | FLAG_5);
        }

        m_CPURegisters.regs.regF |= FLAG_C;
        m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
    }
    else
    {
        // THIS IS UNKNOWN SO LEAVING AS ZILOG FOR NOW
        if ( m_PrevOpcodeFlags & OPCODEFLAG_AltersFlags )
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z);
        }
        else
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z | FLAG_3 | FLAG_5);
        }

        m_CPURegisters.regs.regF |= FLAG_C;
        m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JR_C_off_PC_e(uint8_t)
{
    int8_t offset = Z80CoreMemRead(m_CPURegisters.regPC);

    if ((m_CPURegisters.regs.regF & FLAG_C) == FLAG_C)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);
        Z80CoreMemoryContention(m_CPURegisters.regPC, 1);

        m_CPURegisters.regPC += offset;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }

    m_CPURegisters.regPC++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_HL_SP(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Add16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regSP);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regA = Z80CoreMemRead(m_MEMPTR++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_SP(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.regSP--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INC_A(uint8_t)
{
    Inc(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DEC_A(uint8_t)
{
    Dec(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_n(uint8_t)
{
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.regPC++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CCF(uint8_t)
{
    uint8_t tf = m_CPURegisters.regs.regF;

    if ( m_CPUType == eCPUTYPE_Zilog )
    {
        if ( m_PrevOpcodeFlags & OPCODEFLAG_AltersFlags )
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z);
        }
        else
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z | FLAG_3 | FLAG_5);
        }

        m_CPURegisters.regs.regF |= (tf & FLAG_C) ? FLAG_H : FLAG_C;
        m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
    }
    else
    {
        // THIS IS UNKNOWN SO LEAVING AS ZILOG FOR NOW
        if ( m_PrevOpcodeFlags & OPCODEFLAG_AltersFlags )
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z);
        }
        else
        {
            m_CPURegisters.regs.regF &= (FLAG_P | FLAG_S | FLAG_Z | FLAG_3 | FLAG_5);
        }

        m_CPURegisters.regs.regF |= (tf & FLAG_C) ? FLAG_H : FLAG_C;
        m_CPURegisters.regs.regF |= (m_CPURegisters.regs.regA & (FLAG_3 | FLAG_5));
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_B(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_C(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_D(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_E(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_H(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_L(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_off_HL(uint8_t)
{
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_B_A(uint8_t)
{
    m_CPURegisters.regs.regB = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_B(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_C(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_D(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_E(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_H(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_L(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_off_HL(uint8_t)
{
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_C_A(uint8_t)
{
    m_CPURegisters.regs.regC = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_B(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_C(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_D(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_E(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_H(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_L(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_off_HL(uint8_t)
{
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_D_A(uint8_t)
{
    m_CPURegisters.regs.regD = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_B(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_C(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_D(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_E(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_H(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_L(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_off_HL(uint8_t)
{
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_E_A(uint8_t)
{
    m_CPURegisters.regs.regE = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_B(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_C(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_D(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_E(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_H(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_L(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_off_HL(uint8_t)
{
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_H_A(uint8_t)
{
    m_CPURegisters.regs.regH = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_B(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_C(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_D(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_E(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_H(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_L(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_off_HL(uint8_t)
{
    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_L_A(uint8_t)
{
    m_CPURegisters.regs.regL = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_B(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_C(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_D(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_E(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_H(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_L(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::HALT(uint8_t)
{
    m_CPURegisters.Halted = 1;
    m_CPURegisters.regPC--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_HL_A(uint8_t)
{
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_B(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regB;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_C(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_D(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regD;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_E(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regE;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_H(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regH;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_L(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_off_HL(uint8_t)
{
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_A(uint8_t)
{
    m_CPURegisters.regs.regA = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_B(uint8_t)
{
    Add8(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_C(uint8_t)
{
    Add8(m_CPURegisters.regs.regC);

}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_D(uint8_t)
{
    Add8(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_E(uint8_t)
{
    Add8(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_H(uint8_t)
{
    Add8(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_L(uint8_t)
{
    Add8(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Add8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_A(uint8_t)
{
    Add8(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_B(uint8_t)
{
    Adc8(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_C(uint8_t)
{
    Adc8(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_D(uint8_t)
{
    Adc8(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_E(uint8_t)
{
    Adc8(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_H(uint8_t)
{
    Adc8(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_L(uint8_t)
{
    Adc8(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Adc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_A(uint8_t)
{
    Adc8(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_B(uint8_t)
{
    Sub8(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_C(uint8_t)
{
    Sub8(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_D(uint8_t)
{
    Sub8(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_E(uint8_t)
{
    Sub8(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_H(uint8_t)
{
    Sub8(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_L(uint8_t)
{
    Sub8(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Sub8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_A(uint8_t)
{
    Sub8(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_B(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_C(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_D(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_E(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_H(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_L(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Sbc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_A(uint8_t)
{
    Sbc8(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_B(uint8_t)
{
    And(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_C(uint8_t)
{
    And(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_D(uint8_t)
{
    And(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_E(uint8_t)
{
    And(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_H(uint8_t)
{
    And(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_L(uint8_t)
{
    And(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    And(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_A(uint8_t)
{
    And(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_B(uint8_t)
{
    Xor(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_C(uint8_t)
{
    Xor(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_D(uint8_t)
{
    Xor(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_E(uint8_t)
{
    Xor(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_H(uint8_t)
{
    Xor(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_L(uint8_t)
{
    Xor(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Xor(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_A(uint8_t)
{
    Xor(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_B(uint8_t)
{
    Or(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_C(uint8_t)
{
    Or(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_D(uint8_t)
{
    Or(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_E(uint8_t)
{
Or(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_H(uint8_t)
{
    Or(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_L(uint8_t)
{
    Or(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Or(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_A(uint8_t)
{
    Or(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_B(uint8_t)
{
    Cp(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_C(uint8_t)
{
    Cp(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_D(uint8_t)
{
    Cp(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_E(uint8_t)
{
    Cp(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_H(uint8_t)
{
    Cp(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_L(uint8_t)
{
    Cp(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Cp(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_A(uint8_t)
{
    Cp(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_NZ(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_BC(uint8_t)
{
    m_CPURegisters.regs.regC = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_NZ_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regPC = m_MEMPTR;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_NZ_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_BC(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regB);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADD_A_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Add8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_0H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0000;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_Z(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_Z) == FLAG_Z)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

    m_CPURegisters.regPC = m_MEMPTR;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_Z_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_Z) == FLAG_Z)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_Z_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_Z) == FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

    m_CPURegisters.regPC = m_MEMPTR;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_A_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Adc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_8H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0008;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_NC(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_C) != FLAG_C)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_DE(uint8_t)
{
    m_CPURegisters.regs.regE = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_NC_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_C) != FLAG_C)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_n_A(uint8_t)
{
    uint16_t address = (((uint16_t)m_CPURegisters.regs.regA) << 8) | Z80CoreMemRead(m_CPURegisters.regPC++);
    Z80CoreIOWrite(address, m_CPURegisters.regs.regA);

    m_MEMPTR = (m_CPURegisters.regs.regA << 8) + ((address + 1) & 0xff);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_NC_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_C) != FLAG_C)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_DE(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regD);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SUB_A_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Sub8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_10H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0010;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_C(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_C) == FLAG_C)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EXX(uint8_t)
{
    uint16_t t = m_CPURegisters.reg_pairs.regBC;
    m_CPURegisters.reg_pairs.regBC = m_CPURegisters.reg_pairs.regBC_;
    m_CPURegisters.reg_pairs.regBC_ = t;

    t = m_CPURegisters.reg_pairs.regDE;
    m_CPURegisters.reg_pairs.regDE = m_CPURegisters.reg_pairs.regDE_;
    m_CPURegisters.reg_pairs.regDE_ = t;

    t = m_CPURegisters.reg_pairs.regHL;
    m_CPURegisters.reg_pairs.regHL = m_CPURegisters.reg_pairs.regHL_;
    m_CPURegisters.reg_pairs.regHL_ = t;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_C_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_C) == FLAG_C)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_A_off_n(uint8_t)
{
    m_MEMPTR = (((uint16_t)m_CPURegisters.regs.regA) << 8) | Z80CoreMemRead(m_CPURegisters.regPC++);
    m_CPURegisters.regs.regA = Z80CoreIORead(m_MEMPTR++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_C_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_C) == FLAG_C)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_A_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Sbc8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_18H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0018;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_PO(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_P) != FLAG_P)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_HL(uint8_t)
{
    m_CPURegisters.regs.regL = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regH = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_PO_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_P) != FLAG_P)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EX_off_SP_HL(uint8_t)
{
    uint8_t tl = Z80CoreMemRead(m_CPURegisters.regSP + 0);
    uint8_t th = Z80CoreMemRead(m_CPURegisters.regSP + 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP + 1, 1);
    Z80CoreMemWrite(m_CPURegisters.regSP + 1, m_CPURegisters.regs.regH);
    Z80CoreMemWrite(m_CPURegisters.regSP + 0, m_CPURegisters.regs.regL);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    Z80CoreMemoryContention(m_CPURegisters.regSP, 1);
    m_CPURegisters.regs.regH = th;
    m_CPURegisters.regs.regL = tl;

    m_MEMPTR = m_CPURegisters.reg_pairs.regHL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_PO_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_P) != FLAG_P)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_HL(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regH);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::AND_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    And(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_20H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0020;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_PE(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_P) == FLAG_P)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_off_HL(uint8_t)
{
    m_CPURegisters.regPC = m_CPURegisters.reg_pairs.regHL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_PE_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_P) == FLAG_P)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EX_DE_HL(uint8_t)
{
    uint16_t t = m_CPURegisters.reg_pairs.regHL;
    m_CPURegisters.reg_pairs.regHL = m_CPURegisters.reg_pairs.regDE;
    m_CPURegisters.reg_pairs.regDE = t;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_PE_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_P) == FLAG_P)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::XOR_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Xor(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_28H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0028;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_P(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_S) != FLAG_S)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::POP_AF(uint8_t)
{
    m_CPURegisters.regs.regF = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_CPURegisters.regs.regA = Z80CoreMemRead(m_CPURegisters.regSP++);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_P_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_S) != FLAG_S)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::DI(uint8_t)
{
    m_CPURegisters.IFF1 = 0;
    m_CPURegisters.IFF2 = 0;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_P_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_S) != FLAG_S)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::PUSH_AF(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regA);
    Z80CoreMemWrite(--m_CPURegisters.regSP, m_CPURegisters.regs.regF);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OR_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Or(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_30H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0030;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RET_M(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    if ((m_CPURegisters.regs.regF & FLAG_S) == FLAG_S)
    {
        m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
        m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_SP_HL(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    m_CPURegisters.regSP = m_CPURegisters.reg_pairs.regHL;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::JP_M_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_S) == FLAG_S)
    {
        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::EI(uint8_t)
{
    m_CPURegisters.IFF1 = 1;
    m_CPURegisters.IFF2 = 1;
    m_CPURegisters.EIHandled = true;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CALL_M_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    if ((m_CPURegisters.regs.regF & FLAG_S) == FLAG_S)
    {
        Z80CoreMemoryContention(m_CPURegisters.regPC - 1, 1);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
        Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);

        m_CPURegisters.regPC = m_MEMPTR;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CP_n(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.regPC++);
    Cp(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RST_38H(uint8_t)
{
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 8) & 0xff);
    Z80CoreMemWrite(--m_CPURegisters.regSP, (m_CPURegisters.regPC >> 0) & 0xff);
    m_CPURegisters.regPC = 0x0038;

    m_MEMPTR = m_CPURegisters.regPC;
}

//-----------------------------------------------------------------------------------------

