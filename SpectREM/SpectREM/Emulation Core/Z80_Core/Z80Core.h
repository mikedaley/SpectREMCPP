//
// TZT ZX Spectrum Emulator
//

#ifndef Z80CORE_H
#define Z80CORE_H

#include <cstdint>

//-----------------------------------------------------------------------------------------

#ifndef nullptr
#ifdef __cplusplus
//#define NULL    0
//#else
//#define nullptr    ((void *)0)
#endif
#endif

//-----------------------------------------------------------------------------------------

typedef uint8_t (*Z80CoreRead)(uint16_t address, void *param);
typedef void (*Z80CoreWrite)(uint16_t address, uint8_t data, void *param);
typedef void (*Z80CoreContention)(uint16_t address, uint32_t tstates, void *param);
typedef uint8_t(*Z80CoreDebugRead)(uint16_t address, void *param, void *data);
typedef void (*Z80CoreDebugWrite)(uint16_t address, uint8_t byte, void *param, void *data);
typedef bool (*Z80OpcodeCallback)(uint8_t opcode, uint16_t address, void *param);
typedef char *(*Z80DebugCallback)(char *buffer, uint32_t variableType, uint16_t address, uint32_t value, void *param, void *data);

//-----------------------------------------------------------------------------------------

class CZ80Core
{
public:
    typedef enum
    {
        eREG_A,
        eREG_F,
        eREG_B,
        eREG_C,
        eREG_D,
        eREG_E,
        eREG_H,
        eREG_L,

        eREG_ALT_A,
        eREG_ALT_F,
        eREG_ALT_B,
        eREG_ALT_C,
        eREG_ALT_D,
        eREG_ALT_E,
        eREG_ALT_H,
        eREG_ALT_L,

        eREG_I,
        eREG_R,
    } eZ80BYTEREGISTERS;

    typedef enum
    {
        eREG_AF,
        eREG_HL,
        eREG_BC,
        eREG_DE,
        eREG_ALT_AF,
        eREG_ALT_HL,
        eREG_ALT_BC,
        eREG_ALT_DE,

        eREG_IX,
        eREG_IY,
        eREG_SP,
        eREG_PC,

    } eZ80WORDREGISTERS;

    typedef enum
    {
        eCPUTYPE_Zilog,
        eCPUTYPE_Nec
    } eCPUTYPE;

    typedef enum
    {
        eVARIABLETYPE_Byte,
        eVARIABLETYPE_Word,
        eVARIABLETYPE_IndexOffset,
        eVARIABLETYPE_RelativeOffset,
    } eVARIABLETYPE;

    static const uint8_t FLAG_C = 0x01;
    static const uint8_t FLAG_N = 0x02;
    static const uint8_t FLAG_P = 0x04;
    static const uint8_t FLAG_V = FLAG_P;
    static const uint8_t FLAG_3 = 0x08;
    static const uint8_t FLAG_H = 0x10;
    static const uint8_t FLAG_5 = 0x20;
    static const uint8_t FLAG_Z = 0x40;
    static const uint8_t FLAG_S = 0x80;

private:

    static const uint32_t OPCODEFLAG_AltersFlags = (1 << 0);

    typedef struct
    {
        union
        {
            struct
            {
                // Start with the main regs
                uint16_t	regAF;
                uint16_t	regBC;
                uint16_t	regDE;
                uint16_t	regHL;
                uint16_t	regIX;
                uint16_t	regIY;

                // Exchange registers
                uint16_t	regAF_;
                uint16_t	regBC_;
                uint16_t	regDE_;
                uint16_t	regHL_;
            } reg_pairs;

            // These are the initial byte registers
            struct
            {
                uint8_t	regF;
                uint8_t	regA;
                uint8_t	regC;
                uint8_t	regB;
                uint8_t	regE;
                uint8_t	regD;
                uint8_t	regL;
                uint8_t	regH;
                uint8_t	regIXl;
                uint8_t	regIXh;
                uint8_t	regIYl;
                uint8_t	regIYh;
                uint8_t	regF_;
                uint8_t	regA_;
                uint8_t	regC_;
                uint8_t	regB_;
                uint8_t	regE_;
                uint8_t	regD_;
                uint8_t	regL_;
                uint8_t	regH_;
            } regs;
        };

        // These dont have byte pairs
        uint16_t	regSP;
        uint16_t	regPC;

        uint8_t	regI;
        uint8_t	regR;

        uint8_t	IFF1;
        uint8_t	IFF2;
        uint8_t	IM;

        bool			Halted;
        bool			EIHandled;
        bool			IntReq;
        bool            NMIReq;

        bool            DDFDmultiByte;

        uint32_t	TStates;
    } Z80State;

    typedef struct
    {
        void (CZ80Core::*function)(uint8_t opcode);
        uint32_t flags;
        const char* format;
    } Z80Opcode;

    typedef struct
    {
        Z80Opcode entries[256];
    } Z80OpcodeTable;


public:
    CZ80Core();
    ~CZ80Core();

public:
    void					Initialise(Z80CoreRead mem_read, Z80CoreWrite mem_write, Z80CoreRead io_read, Z80CoreWrite io_write,Z80CoreContention mem_contention_handling, Z80CoreDebugRead debug_read_handler, Z80CoreDebugWrite debug_write_handler,void *member_class);

    void					Reset(bool hardReset = true);
    uint32_t			    Debug_Disassemble(char *pStr, uint32_t StrLen, uint16_t address, bool hexFormat, void *data);
    uint32_t			    Debug_GetOpcodeLength(uint16_t address, void *data);
    bool					Debug_HasValidOpcode(uint16_t address, void *data);
    uint32_t 			    Execute(uint32_t num_tstates = 0, uint32_t int_t_states = 32);

    void					RegisterOpcodeCallback(Z80OpcodeCallback callback);
    void					RegisterDebugCallback(Z80DebugCallback callback);

    void					SignalInterrupt();

    bool					IsInterruptRequesting() const { return (m_CPURegisters.IntReq != 0); }

    uint8_t			        GetRegister(eZ80BYTEREGISTERS reg) const;
    uint16_t			    GetRegister(eZ80WORDREGISTERS reg) const;
    void					SetRegister(eZ80BYTEREGISTERS reg, uint8_t data);
    void					SetRegister(eZ80WORDREGISTERS reg, uint16_t data);

    void					SetIMMode(uint8_t im) { m_CPURegisters.IM = im; m_CPURegisters.IntReq = 0; }
    uint8_t			        GetIMMode() const { return m_CPURegisters.IM; }
    void					SetIFF1(uint8_t iff1) { m_CPURegisters.IFF1 = iff1; }
    uint8_t			        GetIFF1(void) const { return m_CPURegisters.IFF1;  }
    void					SetIFF2(uint8_t iff2) { m_CPURegisters.IFF2 = iff2; }
    uint8_t			        GetIFF2(void) const { return m_CPURegisters.IFF2; }
    bool					GetHalted(void) const { return m_CPURegisters.Halted; }
    void					SetHalted(bool halted) { m_CPURegisters.Halted = halted; }
    void                    setNMIReq(bool nmi) { m_CPURegisters.NMIReq = nmi; }

    bool                    isLD_I_A() { return m_LD_I_A; }

    void					AddContentionTStates(uint32_t extra_tstates) { m_CPURegisters.TStates += extra_tstates; }
    void					AddTStates(uint32_t extra_tstates) { m_CPURegisters.TStates += extra_tstates; }

    uint32_t			GetTStates() const { return m_CPURegisters.TStates; }
    void					ResetTStates() { m_CPURegisters.TStates = 0; }
    void					ResetTStates(uint32_t tstates_per_frame) { m_CPURegisters.TStates -= tstates_per_frame; }

    uint8_t	                Z80CoreMemRead(uint16_t address, uint32_t tstates = 3);
    void					Z80CoreMemWrite(uint16_t address, uint8_t data, uint32_t tstates = 3);
    uint8_t			        Z80CoreIORead(uint16_t address);
    void					Z80CoreIOWrite(uint16_t address, uint8_t data);
    void					Z80CoreMemoryContention(uint16_t address, uint32_t t_states);
    void					Z80CoreIOContention(uint16_t address, uint32_t t_states);
    uint8_t			        Z80CoreDebugMemRead(uint16_t address, void *data);
    void                    Z80CoreDebugMemWrite(uint16_t address, uint8_t byte, void *data);
protected:
    #include "Z80Core_MainOpcodes.h"
    #include "Z80Core_CBOpcodes.h"
    #include "Z80Core_DDOpcodes.h"
    #include "Z80Core_EDOpcodes.h"
    #include "Z80Core_FDOpcodes.h"
    #include "Z80Core_DDCB_FDCBOpcodes.h"

    void					Inc(uint8_t &r);
    void					Dec(uint8_t &r);
    void					Add8(uint8_t &r);
    void					Adc8(uint8_t &r);
    void					Sub8(uint8_t &r);
    void					Sbc8(uint8_t &r);
    void					Add16(uint16_t &r1, uint16_t &r2);
    void					Adc16(uint16_t &r1, uint16_t &r2);
    void					Sbc16(uint16_t &r1, uint16_t &r2);
    void					And(uint8_t &r);
    void					Or(uint8_t &r);
    void					Xor(uint8_t &r);
    void					Cp(uint8_t &r);
    void					RLC(uint8_t &r);
    void					RRC(uint8_t &r);
    void					RL(uint8_t &r);
    void					RR(uint8_t &r);
    void					SLA(uint8_t &r);
    void					SRA(uint8_t &r);
    void					SRL(uint8_t &r);
    void					SLL(uint8_t &r);
    void					Bit(uint8_t &r, uint8_t b);
    void					BitWithMemptr(uint8_t &r, uint8_t b);
    void					Set(uint8_t &r, uint8_t b);
    void					Res(uint8_t &r, uint8_t b);

    const char			*	Debug_GetOpcodeDetails(uint16_t &address, void *data);
    char				*	Debug_WriteData(uint32_t variableType, char *pStr, uint32_t &StrLen, uint16_t address, bool hexFormat, void *data);

protected:
    static Z80OpcodeTable	Main_Opcodes;
    static Z80OpcodeTable	CB_Opcodes;
    static Z80OpcodeTable	DD_Opcodes;
    static Z80OpcodeTable	ED_Opcodes;
    static Z80OpcodeTable	FD_Opcodes;
    static Z80OpcodeTable	DDCB_Opcodes;
    static Z80OpcodeTable	FDCB_Opcodes;

    Z80State				m_CPURegisters;
    uint8_t			        m_ParityTable[256];
    uint8_t			        m_SZ35Table[256];
    uint16_t			    m_MEMPTR;
    eCPUTYPE				m_CPUType;
    uint32_t			m_PrevOpcodeFlags;
    bool                    m_Iff2_read = false;
    bool                    m_LD_I_A = false;

    bool                    paused = false;

    void *                  m_Param;
    Z80CoreRead				m_MemRead;
    Z80CoreWrite			m_MemWrite;
    Z80CoreRead				m_IORead;
    Z80CoreWrite			m_IOWrite;
    Z80CoreContention		m_MemContentionHandling;
    Z80CoreDebugRead		m_DebugRead;
    Z80CoreDebugWrite       m_Debugwrite;

    Z80OpcodeCallback		m_OpcodeCallback;
    Z80DebugCallback		m_DebugCallback;
};


//-----------------------------------------------------------------------------------------

#endif
