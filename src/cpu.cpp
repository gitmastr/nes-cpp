#include "cpu.hpp"
#include "console.hpp"

CPU::CPU(Console &console) :
    cpuMemory(console)
{

}

CPU::~CPU()
{

}

void CPU::setFlags(u8 flags)
{
    C = (flags >> 0) & 1;
    Z = (flags >> 1) & 1;
    I = (flags >> 2) & 1;
    D = (flags >> 3) & 1;
    B = (flags >> 4) & 1;
    U = (flags >> 5) & 1;
    V = (flags >> 6) & 1;
    N = (flags >> 7) & 1;
}

u8 CPU::flags()
{
    return (C << 0
          | Z << 1
          | I << 2
          | D << 3
          | B << 4
          | U << 5
          | V << 6
          | N << 7);
}

u8 CPU::read(u16 addr)
{
    return cpuMemory.read(addr);
}

void CPU::write(u16 addr, u8 value)
{
    cpuMemory.write(addr, value);
}

u16 CPU::read16(u16 addr)
{
    return (static_cast<u16>(read(addr + 1)) << 8) | read(addr);
}

u16 CPU::read16bug(u16 addr)
{
    u16 next = (addr & 0xFF00) | (((addr & 0xFF) + 1) & 0xFF);
    return (static_cast<u16>(read(next)) << 8) | read(addr);
}

void CPU::setZ(u8 value)
{
    Z = (value == 0);
}

void CPU::setN(u8 value)
{
    N = (value & 0x80) != 0;
}

void CPU::setZN(u8 value)
{
    setZ(value);
    setN(value);
}

void CPU::addBranchCycles()
{
    cycles += 1 + pagesDiffer(info.PC, info.addr);
}

u8 CPU::pagesDiffer(u16 a, u16 b)
{
    return (a >> 8) != (b >> 8);
}

void CPU::compare(u8 a, u8 b)
{
    setZN(a - b);
    C = (a >= b);
}

void CPU::push(u8 value)
{
    write(0x100 | static_cast<u16>(SP), value);
    SP--;
}

u8 CPU::pull()
{
    SP++;
    return read(0x100 | static_cast<u16>(SP));
}

void CPU::push16(u16 value)
{
    u8 hi = value >> 8;
    u8 lo = value & 0xFF;
    push(hi);
    push(lo);
}


u16 CPU::pull16()
{
    u16 lo = static_cast<u16>(pull());
    u16 hi = static_cast<u16>(pull());
    return lo | (hi << 8);
}

// It's all instructions below here

/* Add w/ carry */
void CPU::in_adc() 
{ 
    u16 p = A;
    u16 q = read(info.addr);
    u16 r = C;
    A = p + q + r;
    setZN(A);
    C = (A + B + C > 0xFF);
    V = ((((p ^ q) & 0x80) == 0) && (((p ^ A) & 0x80) != 0));
}

/* Logical and */
void CPU::in_and() 
{ 
    A = A & read(info.addr);
    setZN(A);
}

/* Arithmetic shift left */
void CPU::in_asl() 
{ 
    if (info.mode == AddressingMode::Accumulator)
    {
        C = (A >> 7) & 1;
        A <<= 1;
        setZN(A);
    }
    else
    {
        u8 value = read(info.addr);
        C = (value >> 7) & 1;
        value <<= 1;
        write(info.addr, value);
        setZN(value);
    }
}

/* Branch if carry clear */
void CPU::in_bcc() 
{ 
    if (C == 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Branch if carry set */
void CPU::in_bcs() 
{ 
    if (C != 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Branch on result zero */
void CPU::in_beq() 
{ 
    if (Z != 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}


void CPU::in_bit() 
{ 
    u8 value = read(info.addr);
    V = (value >> 6) & 1;
    setZ(value & A);
    setN(value);
}

/* Branch if minus */
void CPU::in_bmi() 
{ 
    if (N != 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Branch of result not zero */
void CPU::in_bne() 
{ 
    if (Z == 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Branch if positive */
void CPU::in_bpl() 
{ 
    if (N == 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Force interrupt */
void CPU::in_brk() 
{ 
    push16(PC);
    in_php();
    in_sei();
    PC = read16(0xFFFE);
}

/* Branch if overflow clear */
void CPU::in_bvc() 
{ 
    if (V == 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Branch if overflow set */
void CPU::in_bvs() 
{ 
    if (V != 0)
    {
        PC = info.addr;
        addBranchCycles();
    }
}

/* Clear cary flag */
void CPU::in_clc() 
{ 
    C = 0;
}

/* Clear decimal mode */
void CPU::in_cld() 
{ 
    D = 0;
}

/* Clear interrupt disable */
void CPU::in_cli() 
{ 
    I = 0;
}

/* Clear overflow flag */
void CPU::in_clv() 
{ 
    V = 0;
}

/* Compare */
void CPU::in_cmp() 
{ 
    u8 value = read(info.addr);
    compare(A, value);
}

/* Compare X register */
void CPU::in_cpx() 
{ 
    u8 value = read(info.addr);
    compare(X, value);
}

/* Compare Y register */
void CPU::in_cpy() 
{ 
    u8 value = read(info.addr);
    compare(Y, value);
}

/* Decrement memory */
void CPU::in_dec() 
{
    u8 value = read(info.addr) - 1;
    write(info.addr, value);
    setZN(value);
}

/* Decrement X register */
void CPU::in_dex() 
{ 
    X--;
    setZN(X);
}

/* Decrement Y register */
void CPU::in_dey() 
{ 
    Y--;
    setZN(Y);
}

/* XOR */
void CPU::in_eor() 
{ 
    u8 value = read(info.addr);
    A ^= value;
    setZN(A);
}

/* Increment memory */
void CPU::in_inc() 
{ 
    u8 value = read(info.addr) + 1;
    write(info.addr, value);
    setZN(value);
}

/* Increment X register */
void CPU::in_inx() 
{ 
    X++;
    setZN(X);
}

/* Increment Y register */
void CPU::in_iny() 
{ 
    Y++;
    setZN(Y);
}

/* Jump */
void CPU::in_jmp() 
{ 
    PC = info.addr;
}

/* Jump to subroutine */
void CPU::in_jsr() 
{ 
    push16(PC - 1);
    PC = info.addr;
}

/* Load AddressingMode::accumulator */
void CPU::in_lda() 
{ 
    A = read(info.addr);
    setZN(A);
}

/* Load X register */
void CPU::in_ldx() 
{ 
    X = read(info.addr);
    setZN(X);
}

/* Load Y register */
void CPU::in_ldy() 
{ 
    Y = read(info.addr);
    setZN(Y);
}

/* Logical shift right */
void CPU::in_lsr() 
{ 
    if (info.mode == AddressingMode::Accumulator)
    {
        C = A & 1;
        A >>= 1;
        setZN(A);
    }
    else
    {
        u8 value = read(info.addr);
        C = value & 1;
        value >>= 1;
        write(info.addr, value);
        setZN(value);
    }
}

/* No operation */
void CPU::in_nop() 
{ 
    /* OwO */
}

/* Logical inclusive or */
void CPU::in_ora() 
{ 
    A |= read(info.addr);
    setZN(A);
}

/* Push AddressingMode::accumulator */
void CPU::in_pha() 
{ 
    push(A);
}

/* Push processor status */
void CPU::in_php() 
{ 
    push(flags() | 0x10);
}

/* Pull AddressingMode::accumulator */
void CPU::in_pla() 
{ 
    A = pull();
    setZN(A);
}

/* Pull processor flags */
void CPU::in_plp() 
{
    setFlags((pull() & 0xEF) | 0x20);
}

void CPU::in_rol() 
{ 
    if (info.mode == AddressingMode::Accumulator)
    {
        u8 c = C;
        C = (A >> 7) & 1;
        A = (A << 1) | c;
        setZN(A);
    }
    else
    {
        u8 c = C;
        u8 value = read(info.addr);
        C = (value >> 7) & 1;
        value = (value << 1) | c;
        write(info.addr, value);
        setZN(value);
    }
}
void CPU::in_ror() 
{
    if (info.mode == AddressingMode::Accumulator)
    {
        u8 c = C;
        C = A & 1;
        A = (A >> 1) | (c << 7);
        setZN(A);
    }
    else
    {
        u8 c = C;
        u8 value = read(info.addr);
        C = value & 1;
        value = (value >> 1) | (c << 7);
        write(info.addr, value);
        setZN(value);
    }

}

/* Return from interrupt */
void CPU::in_rti() 
{ 
    setFlags((pull() & 0xEF) | 0x20);
    PC = pull16();
}

/* Return from subroutine */
void CPU::in_rts() 
{
    PC = pull16() + 1;
}

/* Subtract with carry */
void CPU::in_sbc() 
{
    u16 p = A;
    u16 q = read(info.addr);
    u16 r = C;
    A = p - q - (1 - r);
    setZN(A);

    i16 iA = (i16) p;
    i16 iB = (i16) q;
    i16 iC = (i16) r;

    // if (iA - iB - (1 - iC) >= 0)
    // {
    //     this->C = 1;
    // }
    // else
    // {
    //     this->C = 0;
    // }

    this->C = (iA - iB - (1 - iC) >= 0);

    // if ((((A ^ B) & 0x80) != 0) && (((A ^ this->A) & 0x80) != 0))
    // {
    //     this->V = 1;
    // }
    // else
    // {
    //     this->V = 0;
    // }

    V = ((((p ^ q) & 0x80) != 0) && (((p ^ A) & 0x80) != 0));
}

/* Set carry flag */
void CPU::in_sec() 
{
    C = 1;
}

/* Set decimal flag */
void CPU::in_sed() 
{
    D = 1;
}

/* Set interrupt disable flag */
void CPU::in_sei() 
{ 
    I = 1;
}

/* Store AddressingMode::accumulator */
void CPU::in_sta() 
{
    write(info.addr, A);
}

/* Store X register */
void CPU::in_stx() 
{ 
    write(info.addr, X);
}

/* Store Y register */
void CPU::in_sty() 
{ 
    write(info.addr, Y);
}

/* Transfer A -> X */
void CPU::in_tax() 
{ 
    X = A;
    setZN(X);
}

/* Transfer A -> Y */
void CPU::in_tay() 
{ 
    Y = A;
    setZN(Y);
}

/* Transfer stack pointer to X */
void CPU::in_tsx() 
{
    X = SP;
    setZN(X);
}

/* Transfer X to AddressingMode::accumulator */
void CPU::in_txa() 
{
    A = X;
    setZN(A);
}

/* Transfer X to stack pointer */
void CPU::in_txs() 
{
    SP = X;
}

/* Transfer Y to AddressingMode::accumulator */
void CPU::in_tya() 
{ 
    A = Y;
    setZN(A);
}

/* Illegal opcodes below */

void CPU::in_ahx() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_alr() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_anc() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_arr() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_axs() 
{ 
    throw "Unimplemented illegal opcode reached";
}

/* Equivalent to DEC value and then CMP value (ILLEGAL) */
void CPU::in_dcp() 
{ 
    in_dec();
    in_cmp();
}

/* Equivalent to INC value and then SBC value (ILLEGAL) */
void CPU::in_isc() 
{ 
    in_inc();
    in_sbc();
}

void CPU::in_kil() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_las() 
{ 
    throw "Unimplemented illegal opcode reached";
}

/* Load memory location into both A and X (ILLEGAL) */
void CPU::in_lax() 
{ 
    A = read(info.addr);
    X = A;
    setZN(A);
}

/* Equivalent to ROL and then AND  (ILLEGAL) */
void CPU::in_rla() 
{ 
    in_rol();
    in_and();
}

/* Equivalent to ROR and then ADC (ILLEGAL) */
void CPU::in_rra() 
{ 
    in_ror();
    in_adc();
}

/* Store result of bitwise and of A and X at address (ILLEGAL) */
void CPU::in_sax() 
{ 
    write(info.addr, A & X);
}

void CPU::in_shx() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_shy() 
{ 
    throw "Unimplemented illegal opcode reached";
}

/* Equivalent to ASL value then ORA value (ILLEGAL) */
void CPU::in_slo() 
{ 
    in_asl();
    in_ora();
}

/* Equivalent to LSR and then EOR value (ILLEGAL) */
void CPU::in_sre() 
{ 
    in_lsr();
    in_eor();
}

void CPU::in_tas() 
{ 
    throw "Unimplemented illegal opcode reached";
}

void CPU::in_xaa() 
{ 
    throw "Unimplemented illegal opcode reached";
}