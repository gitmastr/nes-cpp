#pragma once

#include "types.hpp"
#include <ostream>
#include <functional>

namespace CPU
{
    void logInstruction();
    void setPC(u16 pc);

    enum class AddressingMode
    {
        Absolute,
        AbsoluteX,
        AbsoluteY,
        Accumulator,
        Immediate,
        Implied,
        IndexedIndirect,
        Indirect,
        IndirectIndexed,
        Relative,
        ZeroPage,
        ZeroPageX,
        ZeroPageY 
    };

    enum class InterruptType
    {
        None,
        NMI,
        IRQ,
        Reset
    };

    u32 step();
    void runAndLog(u32 number);

    extern u64 cycles;
    extern u16 PC;
    extern u8 SP;
    extern u8 A;
    extern u8 X;
    extern u8 Y;
    extern u8 C;
    extern u8 Z;
    extern u8 I;
    extern u8 D;
    extern u8 B;
    extern u8 U;
    extern u8 V;
    extern u8 N;
    extern InterruptType interrupt;
    extern i32 stall;
    extern struct stepinfo_t {
        u16 addr;
        u16 PC;
        AddressingMode mode;
    } info;

    void setZ(u8 value);
    void setN(u8 value);
    void setZN(u8 value);
    void addBranchCycles();
    void push(u8 value);
    void push16(u16 value);
    void setFlags(u8 flags);
    void compare(u8 a, u8 b);
    void reset();
    void nmi();
    void irq();
    void dispatch(u8 opcode);

    bool pagesDiffer(u16 a, u16 b);

    u8 pull();
    u8 flags();

    u16 pull16();
    u16 read16(u16 addr);
    u16 read16bug(u16 addr);

    extern const AddressingMode addressingModes[];
    extern const u8 instructionSizes[];
    extern const u8 instructionCycles[];
    extern const u8 instructionPageCycles[];
    extern const string instructionNames[];
    extern const std::function<void()> opcodeList[];

    /* Instructions below */
    namespace Instructions
    {
        void adc();
        void anx();
        void asl();
        void bcc();
        void bcs();
        void beq();
        void bit();
        void bmi();
        void bne();
        void bpl();
        void brk();
        void bvc();
        void bvs();
        void clc();
        void cld();
        void cli();
        void clv();
        void cmp();
        void cpx();
        void cpy();
        void dec();
        void dex();
        void dey();
        void eor();
        void inc();
        void inx();
        void iny();
        void jmp();
        void jsr();
        void lda();
        void ldx();
        void ldy();
        void lsr();
        void nop();
        void ora();
        void pha();
        void php();
        void pla();
        void plp();
        void rol();
        void ror();
        void rti();
        void rts();
        void sbc();
        void sec();
        void sed();
        void sei();
        void sta();
        void stx();
        void sty();
        void tax();
        void tay();
        void tsx();
        void txa();
        void txs();
        void tya();
        void ahx();
        void alr();
        void anc();
        void arr();
        void axs();
        void dcp();
        void isc();
        void kil();
        void las();
        void lax();
        void rla();
        void rra();
        void sax();
        void shx();
        void shy();
        void slo();
        void sre();
        void tas();
        void xaa();
    }
};