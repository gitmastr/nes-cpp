myLogPath = 'logs/CPU.log'
goodLogPath = 'logs/accurate.log'

with open(myLogPath, 'r') as file:
    myLogLines = [line for line in file] 

with open(goodLogPath, 'r') as file:
    goodLogLines = [line for line in file]

def logVars(line):
    pc = line[:4]
    opc = line[6:8]
    opcStr = line[16:19]
    A = line[50:52]
    X = line[55:57]
    Y = line[60:62]
    P = line[65:67]
    SP = line[71:73]
    cyc = line[78:81]

    return (pc, opc, opcStr, A, X, Y, P, cyc, SP)

def disagreeOn(i):
    print('    MY LOG' + ' ' * 179 + 'GOOD LOG')
    for j in range(max(i - 5, 0), min(i + 6, min(len(myLogLines), len(goodLogLines)))):
        print((' >> ' if i == j else '    ') + myLogLines[j].rstrip() + '   |   ' + goodLogLines[j].rstrip())

    print()
    print()

    print('Mine: {}'.format(myLogLines[i].rstrip()))
    print('Good: {}'.format(goodLogLines[i].rstrip()))

for i in range(min(len(myLogLines), len(goodLogLines))):
    mine = logVars(myLogLines[i])
    good = logVars(goodLogLines[i])
    if mine != good:
        print('Disagreement on line {}'.format(i + 1))
        
        print()

        disagreeOn(i)

        exit(0)

print('All lines matched. CPU is likely correct.')