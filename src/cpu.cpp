#include <ios>
#include <iomanip>
#include "cpu.hpp"
#include "console.hpp"
#include "mapper.hpp"

namespace CPUMemory
{
    u8 read(u16 addr)
    {
        if      (addr  < 0x2000) return Console::ram[addr % 0x0800];
        else if (addr  < 0x4000) return 0; /* PPU */
        else if (addr == 0x4014) return 0; /* PPU */
        else if (addr == 0x4015) return 0; /* APU */
        else if (addr == 0x4016) return 0; /* Controller1 */
        else if (addr == 0x4017) return 0; /* Controller2 */
        else if (addr  < 0x6000) return 0; /* IO Registers? */
        else if (addr >= 0x6000) return Console::mapper->read(addr);


        throw "invalid read";
        return 0;
    }

    void write(u16 addr, u8 value)
    {
        if      (addr  < 0x2000) Console::ram[addr % 0x0800] = value;
        else if (addr  < 0x4000) return; /* PPU */
        else if (addr  < 0x4014) return; /* APU */
        else if (addr == 0x4014) return; /* PPU */
        else if (addr == 0x4015) return; /* APU */
        else if (addr == 0x4016) return; /* Controllers */
        else if (addr  < 0x6000) return; /* IO or something */
        else if (addr >= 0x6000) Console::mapper->write(addr, value);
    }
}

namespace CPU
{
    using CPUMemory::read;
    using CPUMemory::write;

    u64 cycles = 0;
    u16 PC = 0;
    u8 SP = 0;
    u8 A = 0;
    u8 X = 0;
    u8 Y = 0;
    u8 C = 0;
    u8 Z = 0;
    u8 I = 0;
    u8 D = 0;
    u8 B = 0;
    u8 U = 0;
    u8 V = 0;
    u8 N = 0;
    InterruptType interrupt = InterruptType::None;
    i32 stall = 0;
    stepinfo_t info;


    void setPC(u16 pc)
    {
        PC = pc;
    }

    u32 step()
    {
        u64 oldCycles = cycles;

        if (stall >  0)
        {
            stall--;
            return 1;
        }

        switch (interrupt)
        {
            case InterruptType::None:
                break;
            case InterruptType::NMI:
                nmi();
                break;
            case InterruptType::IRQ:
                irq();
                break;
            default:
                throw "unhandled interrupt";
                break;
        }

        interrupt = InterruptType::None;

        u8 opcode = read(PC);
        AddressingMode mode = addressingModes[opcode];

        u16 address = 0;
        bool pageCrossed = false;
        u16 offset = 0;

        switch (mode)
        {
            case AddressingMode::Absolute:
                address = read16(PC + 1);
                break;
            case AddressingMode::AbsoluteX:
                address = read16(PC + 1) + X;
                pageCrossed = pagesDiffer(address, address - X);
                break;
            case AddressingMode::AbsoluteY:
                address = read16(PC + 1) + Y;
                pageCrossed = pagesDiffer(address, address - Y);
                break;
            case AddressingMode::Accumulator:
                address = 0;
                break;
            case AddressingMode::Immediate:
                address = PC + 1;
                break;
            case AddressingMode::Implied:
                address = 0;
                break;
            case AddressingMode::IndexedIndirect:
                address = read16bug(static_cast<u8>(read(PC + 1) + X));
                break;
            case AddressingMode::Indirect:
                address = read16bug(read16(PC + 1));
                break;
            case AddressingMode::IndirectIndexed:
                address = read16bug(read(PC + 1)) + Y;
                pageCrossed = pagesDiffer(address, address - Y);
                break;
            case AddressingMode::Relative:
                offset = read(PC + 1);
                address = (PC + 2 + offset) - ((offset >= 0x80) ? 0x100 : 0);
                break;
            case AddressingMode::ZeroPage:
                address = read(PC + 1);
                break;
            case AddressingMode::ZeroPageX:
                address = static_cast<u8>(read(PC + 1) + X);
                break;
            case AddressingMode::ZeroPageY:
                address = static_cast<u8>(read(PC + 1) + Y);
                break;
        }

        PC += instructionSizes[opcode];
        cycles += instructionCycles[opcode];

        if (pageCrossed) cycles += instructionPageCycles[opcode];

        info.addr = address;
        info.PC = PC;
        info.mode = mode;

        opcodeList[opcode]();

        return static_cast<u32>(cycles - oldCycles);
    }

    void runAndLog(u32 number)
    {
        for (u32 i = 0; i < number; i++)
        {
            step();
        }

    }

    void nmi()
    {
        push16(PC);
        Instructions::php();
        PC = read16(0xFFFA);
        I = 1;
        cycles += 7;
    }

    void irq()
    {
        push16(PC);
        Instructions::php();
        PC = read16(0xFFFE);
        I = 1;
        cycles += 7;
    }

    void reset()
    {
        cycles = 0;
        PC = 0;
        SP = 0;
        A = 0;
        X = 0;
        Y = 0;
        interrupt = InterruptType::None;
        stall = 0;
        PC = read16(0xFFFC);
        SP = 0xFD;
        setFlags(0x24);
    }

    void setFlags(u8 flags)
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

    u8 flags()
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

    u16 read16(u16 addr)
    {
        return (read(addr + 1) << 8) | read(addr);
    }

    u16 read16bug(u16 addr)
    {
        return (read((addr & 0xFF00) | (((addr & 0xFF) + 1) & 0xFF)) << 8) | read(addr);
    }

    void setZ(u8 value)
    {
        Z = (value == 0);
    }

    void setN(u8 value)
    {
        N = (value & 0x80) != 0;
    }

    void setZN(u8 value)
    {
        setZ(value);
        setN(value);
    }

    void addBranchCycles()
    {
        cycles += 1 + pagesDiffer(info.PC, info.addr);
    }

    bool pagesDiffer(u16 a, u16 b)
    {
        return (a >> 8) != (b >> 8);
    }

    void compare(u8 a, u8 b)
    {
        setZN(a - b);
        C = (a >= b);
    }

    void push(u8 value)
    {
        write(0x100 | SP, value);
        SP--;
    }

    u8 pull()
    {
        SP++;
        return read(0x100 | SP);
    }

    void push16(u16 value)
    {
        u8 hi = value >> 8;
        u8 lo = value & 0xFF;
        push(hi);
        push(lo);
    }


    u16 pull16()
    {
        u8 lo = pull();
        u8 hi = pull();
        return lo | (hi << 8);
    }

    namespace Instructions
    {
        /* Add w/ carry */
        void adc() 
        { 
            u8 a = A;
            u8 r = read(info.addr);
            A = a + r + C;
            setZN(A);
            C = (a + r + C > 0xFF);
            V = ((((a ^ r) & 0x80) == 0) && (((a ^ A) & 0x80) != 0));
        }

        /* Logical and */
        void anx() 
        { 
            A = A & read(info.addr);
            setZN(A);
        }

        /* Arithmetic shift left */
        void asl() 
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
        void bcc() 
        { 
            if (C == 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Branch if carry set */
        void bcs() 
        { 
            if (C != 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Branch on result zero */
        void beq() 
        { 
            if (Z != 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }


        void bit() 
        { 
            u8 value = read(info.addr);
            V = (value >> 6) & 1;
            setZ(value & A);
            setN(value);
        }

        /* Branch if minus */
        void bmi() 
        { 
            if (N != 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Branch of result not zero */
        void bne() 
        { 
            if (Z == 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Branch if positive */
        void bpl() 
        { 
            if (N == 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Force interrupt */
        void brk() 
        { 
            push16(PC);
            php();
            sei();
            PC = read16(0xFFFE);
        }

        /* Branch if overflow clear */
        void bvc() 
        { 
            if (V == 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Branch if overflow set */
        void bvs() 
        { 
            if (V != 0)
            {
                PC = info.addr;
                addBranchCycles();
            }
        }

        /* Clear cary flag */
        void clc() 
        { 
            C = 0;
        }

        /* Clear decimal mode */
        void cld() 
        { 
            D = 0;
        }

        /* Clear interrupt disable */
        void cli() 
        { 
            I = 0;
        }

        /* Clear overflow flag */
        void clv() 
        { 
            V = 0;
        }

        /* Compare */
        void cmp() 
        { 
            u8 value = read(info.addr);
            compare(A, value);
        }

        /* Compare X register */
        void cpx() 
        { 
            u8 value = read(info.addr);
            compare(X, value);
        }

        /* Compare Y register */
        void cpy() 
        { 
            u8 value = read(info.addr);
            compare(Y, value);
        }

        /* Decrement memory */
        void dec() 
        {
            u8 value = read(info.addr) - 1;
            write(info.addr, value);
            setZN(value);
        }

        /* Decrement X register */
        void dex() 
        { 
            X--;
            setZN(X);
        }

        /* Decrement Y register */
        void dey() 
        { 
            Y--;
            setZN(Y);
        }

        /* XOR */
        void eor() 
        { 
            u8 value = read(info.addr);
            A ^= value;
            setZN(A);
        }

        /* Increment memory */
        void inc() 
        { 
            u8 value = read(info.addr) + 1;
            write(info.addr, value);
            setZN(value);
        }

        /* Increment X register */
        void inx() 
        { 
            X++;
            setZN(X);
        }

        /* Increment Y register */
        void iny() 
        { 
            Y++;
            setZN(Y);
        }

        /* Jump */
        void jmp() 
        { 
            PC = info.addr;
        }

        /* Jump to subroutine */
        void jsr() 
        { 
            push16(PC - 1);
            PC = info.addr;
        }

        /* Load AddressingMode::accumulator */
        void lda() 
        { 
            A = read(info.addr);
            setZN(A);
        }

        /* Load X register */
        void ldx() 
        { 
            X = read(info.addr);
            setZN(X);
        }

        /* Load Y register */
        void ldy() 
        { 
            Y = read(info.addr);
            setZN(Y);
        }

        /* Logical shift right */
        void lsr() 
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
        void nop() 
        { 
            /* OwO */
        }

        /* Logical inclusive or */
        void ora() 
        { 
            A |= read(info.addr);
            setZN(A);
        }

        /* Push AddressingMode::accumulator */
        void pha() 
        { 
            push(A);
        }

        /* Push processor status */
        void php() 
        { 
            push(flags() | 0x10);
        }

        /* Pull AddressingMode::accumulator */
        void pla() 
        { 
            A = pull();
            setZN(A);
        }

        /* Pull processor flags */
        void plp() 
        {
            setFlags((pull() & 0xEF) | 0x20);
        }

        void rol() 
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
        void ror() 
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
        void rti() 
        { 
            setFlags((pull() & 0xEF) | 0x20);
            PC = pull16();
        }

        /* Return from subroutine */
        void rts() 
        {
            PC = pull16() + 1;
        }

        /* Subtract with carry */
        void sbc() 
        {
            u8 p = A;
            u8 q = read(info.addr);
            u8 r = C;
            i16 tmp = p - q - (1 - r);
            A = tmp;
            C = tmp >= 0;
            V = ((p ^ q) & 0x80) && ((p ^ A) & 0x80);
            setZN(A);
        }

        /* Set carry flag */
        void sec() 
        {
            C = 1;
        }

        /* Set decimal flag */
        void sed() 
        {
            D = 1;
        }

        /* Set interrupt disable flag */
        void sei() 
        { 
            I = 1;
        }

        /* Store AddressingMode::accumulator */
        void sta() 
        {
            write(info.addr, A);
        }

        /* Store X register */
        void stx() 
        { 
            write(info.addr, X);
        }

        /* Store Y register */
        void sty() 
        { 
            write(info.addr, Y);
        }

        /* Transfer A -> X */
        void tax() 
        { 
            X = A;
            setZN(X);
        }

        /* Transfer A -> Y */
        void tay() 
        { 
            Y = A;
            setZN(Y);
        }

        /* Transfer stack pointer to X */
        void tsx() 
        {
            X = SP;
            setZN(X);
        }

        /* Transfer X to AddressingMode::accumulator */
        void txa() 
        {
            A = X;
            setZN(A);
        }

        /* Transfer X to stack pointer */
        void txs() 
        {
            SP = X;
        }

        /* Transfer Y to AddressingMode::accumulator */
        void tya() 
        { 
            A = Y;
            setZN(A);
        }

        /* Illegal opcodes below */

        void ahx() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void alr() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void anc() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void arr() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void axs() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        /* Equivalent to DEC value and then CMP value (ILLEGAL) */
        void dcp() 
        { 
            dec();
            cmp();
        }

        /* Equivalent to INC value and then SBC value (ILLEGAL) */
        void isc() 
        { 
            inc();
            sbc();
        }

        void kil() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void las() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        /* Load memory location into both A and X (ILLEGAL) */
        void lax() 
        { 
            A = read(info.addr);
            X = A;
            setZN(A);
        }

        /* Equivalent to ROL and then AND  (ILLEGAL) */
        void rla() 
        { 
            rol();
            anx();
        }

        /* Equivalent to ROR and then ADC (ILLEGAL) */
        void rra() 
        { 
            ror();
            adc();
        }

        /* Store result of bitwise and of A and X at address (ILLEGAL) */
        void sax() 
        { 
            write(info.addr, A & X);
        }

        void shx() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void shy() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        /* Equivalent to ASL value then ORA value (ILLEGAL) */
        void slo() 
        { 
            asl();
            ora();
        }

        /* Equivalent to LSR and then EOR value (ILLEGAL) */
        void sre() 
        { 
            lsr();
            eor();
        }

        void tas() 
        { 
            throw "Unimplemented illegal opcode reached";
        }

        void xaa() 
        { 
            throw "Unimplemented illegal opcode reached";
        }
    }
}