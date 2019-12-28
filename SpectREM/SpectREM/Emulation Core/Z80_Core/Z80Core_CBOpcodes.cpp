#include "Z80Core.h"

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_B(uint8_t)
{
    RLC(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_C(uint8_t)
{
    RLC(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_D(uint8_t)
{
    RLC(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_E(uint8_t)
{
    RLC(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_H(uint8_t)
{
    RLC(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_L(uint8_t)
{
    RLC(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    RLC(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RLC_A(uint8_t)
{
    RLC(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_B(uint8_t)
{
    RRC(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_C(uint8_t)
{
    RRC(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_D(uint8_t)
{
    RRC(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_E(uint8_t)
{
    RRC(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_H(uint8_t)
{
    RRC(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_L(uint8_t)
{
    RRC(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    RRC(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RRC_A(uint8_t)
{
    RRC(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_B(uint8_t)
{
    RL(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_C(uint8_t)
{
    RL(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_D(uint8_t)
{
    RL(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_E(uint8_t)
{
    RL(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_H(uint8_t)
{
    RL(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_L(uint8_t)
{
    RL(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    RL(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RL_A(uint8_t)
{
    RL(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_B(uint8_t)
{
    RR(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_C(uint8_t)
{
    RR(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_D(uint8_t)
{
    RR(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_E(uint8_t)
{
    RR(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_H(uint8_t)
{
    RR(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_L(uint8_t)
{
    RR(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    RR(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RR_A(uint8_t)
{
    RR(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_B(uint8_t)
{
    SLA(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_C(uint8_t)
{
    SLA(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_D(uint8_t)
{
    SLA(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_E(uint8_t)
{
    SLA(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_H(uint8_t)
{
    SLA(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_L(uint8_t)
{
    SLA(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    SLA(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLA_A(uint8_t)
{
    SLA(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_B(uint8_t)
{
    SRA(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_C(uint8_t)
{
    SRA(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_D(uint8_t)
{
    SRA(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_E(uint8_t)
{
    SRA(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_H(uint8_t)
{
    SRA(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_L(uint8_t)
{
    SRA(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    SRA(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRA_A(uint8_t)
{
    SRA(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_B(uint8_t)
{
    SLL(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_C(uint8_t)
{
    SLL(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_D(uint8_t)
{
    SLL(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_E(uint8_t)
{
    SLL(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_H(uint8_t)
{
    SLL(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_L(uint8_t)
{
    SLL(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    SLL(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SLL_A(uint8_t)
{
    SLL(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_B(uint8_t)
{
    SRL(m_CPURegisters.regs.regB);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_C(uint8_t)
{
    SRL(m_CPURegisters.regs.regC);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_D(uint8_t)
{
    SRL(m_CPURegisters.regs.regD);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_E(uint8_t)
{
    SRL(m_CPURegisters.regs.regE);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_H(uint8_t)
{
    SRL(m_CPURegisters.regs.regH);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_L(uint8_t)
{
    SRL(m_CPURegisters.regs.regL);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    SRL(t);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SRL_A(uint8_t)
{
    SRL(m_CPURegisters.regs.regA);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_0_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_1_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_2_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_3_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_4_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_5_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_6_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_B(uint8_t)
{
    Bit(m_CPURegisters.regs.regB, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_C(uint8_t)
{
    Bit(m_CPURegisters.regs.regC, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_D(uint8_t)
{
    Bit(m_CPURegisters.regs.regD, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_E(uint8_t)
{
    Bit(m_CPURegisters.regs.regE, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_H(uint8_t)
{
    Bit(m_CPURegisters.regs.regH, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_L(uint8_t)
{
    Bit(m_CPURegisters.regs.regL, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    BitWithMemptr(t, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::BIT_7_A(uint8_t)
{
    Bit(m_CPURegisters.regs.regA, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 0);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_0_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_1_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 2);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_2_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 3);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_3_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 4);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_4_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 5);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_5_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 6);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_6_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_B(uint8_t)
{
    Res(m_CPURegisters.regs.regB, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_C(uint8_t)
{
    Res(m_CPURegisters.regs.regC, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_D(uint8_t)
{
    Res(m_CPURegisters.regs.regD, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_E(uint8_t)
{
    Res(m_CPURegisters.regs.regE, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_H(uint8_t)
{
    Res(m_CPURegisters.regs.regH, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_L(uint8_t)
{
    Res(m_CPURegisters.regs.regL, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Res(t, 7);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::RES_7_A(uint8_t)
{
    Res(m_CPURegisters.regs.regA, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 0);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_0_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 0);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 1);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_1_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 1);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 2);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_2_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 2);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 3);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_3_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 3);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 4);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_4_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 4);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 5);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_5_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 5);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 6);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_6_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 6);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_B(uint8_t)
{
    Set(m_CPURegisters.regs.regB, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_C(uint8_t)
{
    Set(m_CPURegisters.regs.regC, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_D(uint8_t)
{
    Set(m_CPURegisters.regs.regD, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_E(uint8_t)
{
    Set(m_CPURegisters.regs.regE, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_H(uint8_t)
{
    Set(m_CPURegisters.regs.regH, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_L(uint8_t)
{
    Set(m_CPURegisters.regs.regL, 7);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_off_HL(uint8_t)
{
    uint8_t t = Z80CoreMemRead(m_CPURegisters.reg_pairs.regHL);
    Z80CoreMemoryContention(m_CPURegisters.reg_pairs.regHL, 1);
    Set(t, 7);
    Z80CoreMemWrite(m_CPURegisters.reg_pairs.regHL, t);
}

//-----------------------------------------------------------------------------------------

void CZ80Core::SET_7_A(uint8_t)
{
    Set(m_CPURegisters.regs.regA, 7);
}

//-----------------------------------------------------------------------------------------
