with open('instruction_list.txt', 'r') as file:
    lines = [line.rstrip() for line in file]

assert len(lines) == 256
assert all(len(a) == 6 for a in lines)

with open('output.txt', 'w') as out:
    out.write('void CPU::dispatch(u8 opcode) { switch (opcode) { ')
    for i in range(256):
        out.write('case {}: {}(); break; '.format(hex(i), lines[i]))

    out.write('} }')