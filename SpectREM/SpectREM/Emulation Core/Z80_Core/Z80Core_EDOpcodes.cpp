#include "Z80Core.h"
#include <cstdint>

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_B_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regB = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regB];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regB];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_B(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_HL_BC(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);
    Z80CoreMemoryContention(static_cast<uint16_t>((m_CPURegisters.regI << 8) | m_CPURegisters.regR), 1);

    Sbc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regBC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_BC(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regC);
    Z80CoreMemWrite(m_MEMPTR, m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::NEG(uint8_t)
{
    uint8_t t = m_CPURegisters.regs.regA;
    m_CPURegisters.regs.regA = 0;
    Sub8(t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RETN(uint8_t)
{
    m_CPURegisters.IFF1 = m_CPURegisters.IFF2;

    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

    m_CPURegisters.regPC = m_MEMPTR;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IM_0(uint8_t)
{
    m_CPURegisters.IM = 0;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_I_A(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    m_CPURegisters.regI = m_CPURegisters.regs.regA;
    m_LD_I_A = true;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_C_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regC = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regC];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regC];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_HL_BC(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Adc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regBC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_BC_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regC = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regs.regB = Z80CoreMemRead(m_MEMPTR);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RETI(uint8_t)
{
    m_CPURegisters.IFF1 = m_CPURegisters.IFF2;

    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regSP++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regSP++) << 8;

    m_CPURegisters.regPC = m_MEMPTR;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_R_A(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    m_CPURegisters.regR = m_CPURegisters.regs.regA;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_D_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regD = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regD];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regD];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_D(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_HL_DE(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Sbc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regDE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_DE(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regs.regE);
    Z80CoreMemWrite(m_MEMPTR, m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IM_1(uint8_t)
{
    m_CPURegisters.IM = 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_I(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    m_CPURegisters.regs.regA = m_CPURegisters.regI;
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & FLAG_C);
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regA];
    m_CPURegisters.regs.regF |= (m_CPURegisters.IFF2 == 0) ? 0 : FLAG_V;

    m_Iff2_read = true;
    m_LD_I_A = true;

}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_E_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regE = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regE];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regE];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_E(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_HL_DE(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Adc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regDE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_DE_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regs.regE = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regs.regD = Z80CoreMemRead(m_MEMPTR);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IM_2(uint8_t)
{
    m_CPURegisters.IM = 2;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_A_R(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    m_CPURegisters.regs.regA = m_CPURegisters.regR;
    m_CPURegisters.regs.regF = (m_CPURegisters.regs.regF & FLAG_C);
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regA];
    m_CPURegisters.regs.regF |= (m_CPURegisters.IFF2 == 0) ? 0 : FLAG_V;

    m_Iff2_read = true;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_H_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regH = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regH];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regH];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_H(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_HL_HL(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Sbc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRD(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, (m_CPURegisters.regs.regA << 4) | (t >> 4));
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA & 0xf0) | (t & 0x0f);
    m_CPURegisters.regs.regF = m_CPURegisters.regs.regF & FLAG_C;
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regA];
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regA];

    m_MEMPTR = m_CPURegisters.reg_pairs.regHL + 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_L_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regL = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regL];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regL];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_L(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_HL_HL(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Adc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.reg_pairs.regHL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLD(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, (m_CPURegisters.regs.regA & 0x0f) | (t << 4));
    m_CPURegisters.regs.regA = (m_CPURegisters.regs.regA & 0xf0) | (t >> 4);
    m_CPURegisters.regs.regF = m_CPURegisters.regs.regF & FLAG_C;
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regA];
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regA];

    m_MEMPTR = m_CPURegisters.reg_pairs.regHL + 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_F_off_C(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    uint8_t t = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[t];
    m_CPURegisters.regs.regF |= m_ParityTable[t];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_0(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SBC_HL_SP(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Sbc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regSP);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_off_nn_SP(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    Z80CoreMemWrite(m_MEMPTR++, m_CPURegisters.regSP & 0xff);
    Z80CoreMemWrite(m_MEMPTR, (m_CPURegisters.regSP >> 8) & 0xff);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IN_A_off_C(uint8_t)
{
    m_CPURegisters.regs.regA = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= m_SZ35Table[m_CPURegisters.regs.regA];
    m_CPURegisters.regs.regF |= m_ParityTable[m_CPURegisters.regs.regA];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUT_off_C_A(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::ADC_HL_SP(uint8_t)
{
    // Handle contention
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);

    Adc16(m_CPURegisters.reg_pairs.regHL, m_CPURegisters.regSP);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LD_SP_off_nn(uint8_t)
{
    m_MEMPTR = Z80CoreMemRead(m_CPURegisters.regPC++);
    m_MEMPTR |= Z80CoreMemRead(m_CPURegisters.regPC++) << 8;

    m_CPURegisters.regSP = Z80CoreMemRead(m_MEMPTR++);
    m_CPURegisters.regSP |= (Z80CoreMemRead(m_MEMPTR) << 8);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LDI(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regDE, t);

    // Get the temp stuff for flags
    t += m_CPURegisters.regs.regA;

    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE, 1);

    m_CPURegisters.reg_pairs.regDE++;
    m_CPURegisters.reg_pairs.regHL++;
    m_CPURegisters.reg_pairs.regBC--;

    m_CPURegisters.regs.regF &= (FLAG_C | FLAG_S | FLAG_Z);
    m_CPURegisters.regs.regF |= (m_CPURegisters.reg_pairs.regBC != 0) ? FLAG_V : 0;
    m_CPURegisters.regs.regF |= (t & (1 << 1)) ? FLAG_5 : 0;
    m_CPURegisters.regs.regF |= (t & (1 << 3)) ? FLAG_3 : 0;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CPI(uint8_t)
{
    static uint8_t halfcarry_lookup[] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };

    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    uint16_t full_answer = m_CPURegisters.regs.regA - t;

    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);

    m_CPURegisters.reg_pairs.regHL++;
    m_CPURegisters.reg_pairs.regBC--;

    int lookup = ((m_CPURegisters.regs.regA & 0x08) >> 3) | ((t & 0x08) >> 2) | ((full_answer & 0x08) >> 1);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= (full_answer == 0) ? FLAG_Z : 0;
    m_CPURegisters.regs.regF |= ((full_answer & 0x80) == 0x80) ? FLAG_S : 0;
    m_CPURegisters.regs.regF |= (halfcarry_lookup[lookup] | FLAG_N);
    m_CPURegisters.regs.regF |= (m_CPURegisters.reg_pairs.regBC != 0) ? FLAG_V : 0;

    if (m_CPURegisters.regs.regF & FLAG_H)
    {
        full_answer--;
    }

    m_CPURegisters.regs.regF |= (full_answer & (1 << 1)) ? FLAG_5 : 0;
    m_CPURegisters.regs.regF |= (full_answer & (1 << 3)) ? FLAG_3 : 0;

    m_MEMPTR++;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INI(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;

    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    uint8_t t = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
    m_CPURegisters.reg_pairs.regHL++;
    m_CPURegisters.regs.regB--;

    uint16_t temp = ((m_CPURegisters.regs.regC + 1) & 0xff) + t;

    m_CPURegisters.regs.regF = m_SZ35Table[m_CPURegisters.regs.regB];
    m_CPURegisters.regs.regF |= ((t & 0x80) == 0x80) ? FLAG_N : 0;
    m_CPURegisters.regs.regF |= (temp > 255) ? (FLAG_H | FLAG_C) : 0;
    m_CPURegisters.regs.regF |= m_ParityTable[((temp & 7) ^ m_CPURegisters.regs.regB)];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUTI(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    m_CPURegisters.regs.regB--;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, t);
    m_CPURegisters.reg_pairs.regHL++;

    uint16_t temp = m_CPURegisters.regs.regL + t;

    m_CPURegisters.regs.regF = m_SZ35Table[m_CPURegisters.regs.regB];
    m_CPURegisters.regs.regF |= ((t & 0x80) == 0x80) ? FLAG_N : 0;
    m_CPURegisters.regs.regF |= (temp > 255) ? (FLAG_H | FLAG_C) : 0;
    m_CPURegisters.regs.regF |= m_ParityTable[((temp & 7) ^ m_CPURegisters.regs.regB)];

    m_MEMPTR = m_CPURegisters.reg_pairs.regBC + 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LDD(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regDE, t);

    // Add for flags
    t += m_CPURegisters.regs.regA;

    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE, 1);

    m_CPURegisters.reg_pairs.regDE--;
    m_CPURegisters.reg_pairs.regHL--;
    m_CPURegisters.reg_pairs.regBC--;

    m_CPURegisters.regs.regF &= (FLAG_C | FLAG_S | FLAG_Z);
    m_CPURegisters.regs.regF |= (m_CPURegisters.reg_pairs.regBC != 0) ? FLAG_V : 0;
    m_CPURegisters.regs.regF |= (t & (1 << 1)) ? FLAG_5 : 0;
    m_CPURegisters.regs.regF |= (t & (1 << 3)) ? FLAG_3 : 0;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CPD(uint8_t)
{
    static uint8_t halfcarry_lookup[] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };

    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    uint16_t full_answer = m_CPURegisters.regs.regA - t;

    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);

    m_CPURegisters.reg_pairs.regHL--;
    m_CPURegisters.reg_pairs.regBC--;

    int lookup = ((m_CPURegisters.regs.regA & 0x08) >> 3) | ((t & 0x08) >> 2) | ((full_answer & 0x08) >> 1);
    m_CPURegisters.regs.regF &= FLAG_C;
    m_CPURegisters.regs.regF |= (full_answer == 0) ? FLAG_Z : 0;
    m_CPURegisters.regs.regF |= ((full_answer & 0x80) == 0x80) ? FLAG_S : 0;
    m_CPURegisters.regs.regF |= (halfcarry_lookup[lookup] | FLAG_N);
    m_CPURegisters.regs.regF |= (m_CPURegisters.reg_pairs.regBC != 0) ? FLAG_V : 0;

    if (m_CPURegisters.regs.regF & FLAG_H)
    {
        full_answer--;
    }

    m_CPURegisters.regs.regF |= (full_answer & (1 << 1)) ? FLAG_5 : 0;
    m_CPURegisters.regs.regF |= (full_answer & (1 << 3)) ? FLAG_3 : 0;

    m_MEMPTR--;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::IND(uint8_t)
{
    m_MEMPTR = m_CPURegisters.reg_pairs.regBC - 1;

    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    uint8_t t = Z80CoreIORead(m_CPURegisters.reg_pairs.regBC);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
    m_CPURegisters.reg_pairs.regHL--;
    m_CPURegisters.regs.regB--;

    uint16_t temp = ((m_CPURegisters.regs.regC - 1) & 0xff) + t;

    m_CPURegisters.regs.regF = m_SZ35Table[m_CPURegisters.regs.regB];
    m_CPURegisters.regs.regF |= ((t & 0x80) == 0x80) ? FLAG_N : 0;
    m_CPURegisters.regs.regF |= (temp > 255) ? (FLAG_H | FLAG_C) : 0;
    m_CPURegisters.regs.regF |= m_ParityTable[((temp & 7) ^ m_CPURegisters.regs.regB)];
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OUTD(uint8_t)
{
    Z80CoreMemoryContention((m_CPURegisters.regI << 8) | m_CPURegisters.regR, 1);
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    m_CPURegisters.regs.regB--;
    Z80CoreIOWrite(m_CPURegisters.reg_pairs.regBC, t);
    m_CPURegisters.reg_pairs.regHL--;

    uint16_t temp = m_CPURegisters.regs.regL + t;

    m_CPURegisters.regs.regF = m_SZ35Table[m_CPURegisters.regs.regB];
    m_CPURegisters.regs.regF |= ((t & 0x80) == 0x80) ? FLAG_N : 0;
    m_CPURegisters.regs.regF |= (temp > 255) ? (FLAG_H | FLAG_C) : 0;
    m_CPURegisters.regs.regF |= m_ParityTable[((temp & 7) ^ m_CPURegisters.regs.regB)];

    m_MEMPTR = m_CPURegisters.reg_pairs.regBC - 1;
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LDIR(uint8_t opcode)
{
    LDI(opcode);

    if (m_CPURegisters.reg_pairs.regBC != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE - 1, 1);
        m_CPURegisters.regPC -= 2;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CPIR(uint8_t opcode)
{
    CPI(opcode);

    if (m_CPURegisters.reg_pairs.regBC != 0 && (m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        m_CPURegisters.regPC -= 2;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INIR(uint8_t opcode)
{
    INI(opcode);

    if (m_CPURegisters.regs.regB != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL - 1, 1);
        m_CPURegisters.regPC -= 2;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OTIR(uint8_t opcode)
{
    OUTI(opcode);

    if (m_CPURegisters.regs.regB != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        m_CPURegisters.regPC -= 2;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::LDDR(uint8_t opcode)
{
    LDD(opcode);

    if (m_CPURegisters.reg_pairs.regBC != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regDE + 1, 1);
        m_CPURegisters.regPC -= 2;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::CPDR(uint8_t opcode)
{
    CPD(opcode);

    if (m_CPURegisters.reg_pairs.regBC != 0 && (m_CPURegisters.regs.regF & FLAG_Z) != FLAG_Z)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        m_CPURegisters.regPC -= 2;
        m_MEMPTR = m_CPURegisters.regPC + 1;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::INDR(uint8_t opcode)
{
    IND(opcode);

    if (m_CPURegisters.regs.regB != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL + 1, 1);
        m_CPURegisters.regPC -= 2;
    }
}

//-----------------------------------------------------------------------------------------

void CZ80Core::OTDR(uint8_t opcode)
{
    OUTD(opcode);

    if (m_CPURegisters.regs.regB != 0)
    {
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regBC, 1);
        m_CPURegisters.regPC -= 2;
    }
}

//-----------------------------------------------------------------------------------------

