#pragma once

#include "types.hpp"
#include "cpumemory.hpp"

class CPU
{
public:
    CPU(Console &console);
    ~CPU();

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
private:
    CPUMemory cpuMemory;
    u64 cycles;
    u16 PC;
    u8 SP;
    u8 A;
    u8 X;
    u8 Y;
    u8 C;
    u8 Z;
    u8 I;
    u8 D;
    u8 B;
    u8 U;
    u8 V;
    u8 N;
    u8 interrupt;
    i32 stall;
    struct stepinfo_t {
        u16 addr;
        u16 PC;
        AddressingMode mode;
    } info;
    static const AddressingMode addressingModes[];
    static const u8 instructionSizes[];
    static const u8 instructionCycles[];
    static const u8 instructionPageCycles[];
    static const string instructionNames[];

    void write(u16 addr, u8 value);
    void setZ(u8 value);
    void setN(u8 value);
    void setZN(u8 value);
    void addBranchCycles();
    void push(u8 value);
    void push16(u16 value);
    void setFlags(u8 flags);
    void compare(u8 a, u8 b);

    u8 pagesDiffer(u16 a, u16 b);
    u8 pull();
    u8 read(u16 addr);
    u8 flags();

    u16 pull16();
    u16 read16(u16 addr);
    u16 read16bug(u16 addr);

    /* Instructions below */
    void in_adc();
    void in_and();
    void in_asl();
    void in_bcc();
    void in_bcs();
    void in_beq();
    void in_bit();
    void in_bmi();
    void in_bne();
    void in_bpl();
    void in_brk();
    void in_bvc();
    void in_bvs();
    void in_clc();
    void in_cld();
    void in_cli();
    void in_clv();
    void in_cmp();
    void in_cpx();
    void in_cpy();
    void in_dec();
    void in_dex();
    void in_dey();
    void in_eor();
    void in_inc();
    void in_inx();
    void in_iny();
    void in_jmp();
    void in_jsr();
    void in_lda();
    void in_ldx();
    void in_ldy();
    void in_lsr();
    void in_nop();
    void in_ora();
    void in_pha();
    void in_php();
    void in_pla();
    void in_plp();
    void in_rol();
    void in_ror();
    void in_rti();
    void in_rts();
    void in_sbc();
    void in_sec();
    void in_sed();
    void in_sei();
    void in_sta();
    void in_stx();
    void in_sty();
    void in_tax();
    void in_tay();
    void in_tsx();
    void in_txa();
    void in_txs();
    void in_tya();
    void in_ahx();
    void in_alr();
    void in_anc();
    void in_arr();
    void in_axs();
    void in_dcp();
    void in_isc();
    void in_kil();
    void in_las();
    void in_lax();
    void in_rla();
    void in_rra();
    void in_sax();
    void in_shx();
    void in_shy();
    void in_slo();
    void in_sre();
    void in_tas();
    void in_xaa();
};