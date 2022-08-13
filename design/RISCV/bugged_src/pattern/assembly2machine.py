def generate_immediate(decimal, bits):
    length = '0' + str(bits)
    if decimal < 0:
        binary = bin(decimal + (1 << 32))[-bits:]
    else:
        binary = f'{decimal:{length}b}'
    # print(decimal, bits, binary)
    return binary

def bin2hex(binaryString):
    if(len(binaryString) != 32):
        print('Error machine code length !!')
        return 'XX_XX_XX_XX'
    HEX = []
    for i in range(4):
        str1 = binaryString[i * 8 : i * 8 + 4]
        str2 = binaryString[i * 8 + 4 : i * 8 + 8]
        HEX.append(hex(int(str1, 2))[2:].upper() + hex(int(str2, 2))[2:].upper())
    return HEX[3] + '_' + HEX[2] + '_' + HEX[1] + '_' + HEX[0]

def dec2bin(decimal):
    binary = ['0' for i in range(5)]
    if type(decimal) != 'int':
        decimal = int(decimal)
    for i in range(5):
        if (decimal >> i) & 1 :
            binary[4 - i] = '1'
        else:
            binary[4 - i] = '0'
    return ''.join(binary)

def parseData(data):
    instruction = data.split()[0]
    operand = data.split()[1].split(',')
    for oper in operand:
        oper = oper.strip()
    instruction_type = ''
    opcode = ''
    func3 = ''
    func7 = ''
    machine = ''
    comment = ''

    if instruction == 'add':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '000'
        func7               = '0000000'
    elif instruction == 'addi':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '000'
        func7               = ''
    elif instruction == 'sub':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '000'
        func7               = '0100000'
    elif instruction == 'and':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '111'
        func7               = '0000000'
    elif instruction == 'andi':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '111'
        func7               = ''
    elif instruction == 'or':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '110'
        func7               = '0000000'
    elif instruction == 'ori':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '110'
        func7               = ''
    elif instruction == 'xor':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '100'
        func7               = '0000000'
    elif instruction == 'xori':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '100'
        func7               = ''
    elif instruction == 'slli':
        instruction_type = 'I'
        opcode              = '0010011'
        func3               = '001'
        func7               = '0000000'
    elif instruction == 'srai':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '101'
        func7               = '0100000'
    elif instruction == 'srli':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '101'
        func7               = '0000000'
    elif instruction == 'slt':
        instruction_type    = 'R'
        opcode              = '0110011'
        func3               = '010'
        func7               = '0000000'
    elif instruction == 'slti':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '010'
        func7               = ''
    elif instruction == 'beq':
        instruction_type    = 'SB'
        opcode              = '1100011'
        func3               = '000'
        func7               = ''
    elif instruction == 'bne':
        instruction_type    = 'SB'
        opcode              = '1100011'
        func3               = '001'
        func7               = ''
    elif instruction == 'jal':
        instruction_type    = 'UJ'
        opcode              = '1101111'
        func3               = ''
        func7               = ''
    elif instruction == 'jalr':
        instruction_type    = 'I'
        opcode              = '1100111'
        func3               = '000'
        func7               = ''
    elif instruction == 'lw':
        instruction_type    = 'I'
        opcode              = '0000011'
        func3               = '010'
        func7               = ''
    elif instruction == 'sw':
        instruction_type    = 'S'
        opcode              = '0100011'
        func3               = '010'
        func7               = ''
    elif instruction == 'NOP':
        instruction_type    = 'I'
        opcode              = '0010011'
        func3               = '000'
        func7               = ''
    else:
        instruction_type    = 'UNKNOWN'
        opcode              = ''
        func3               = ''
        func7               = ''

    if instruction_type == 'R':
        if len(operand) != 3:
            print('======== Error ========')
            print(f'{instruction} expect 3 operands !!')
            print('=======================')
        else:
            comment = func7 + '_' + dec2bin(int(operand[2][1:])) + '_' + dec2bin(int(operand[1][1:])) + '_' + func3 + '_' + dec2bin(int(operand[0][1:])) + '_' + opcode
            machine = func7 + dec2bin(int(operand[2][1:])) + dec2bin(int(operand[1][1:])) + func3 + dec2bin(int(operand[0][1:])) + opcode
    elif instruction_type == 'I':
        if instruction == 'lw':
            if len(operand) != 2:
                print('======== Error ========')
                print(f'{instruction} expect 2 operands !!')
                print('=======================')
            else:
                leftBracket = operand[1].find('(')
                immediate = generate_immediate(int(operand[1][:leftBracket]), 12)
                comment = immediate + '_' + dec2bin(operand[1][leftBracket + 2 : -1]) + '_' + func3 + '_' + dec2bin(int(operand[0][1:])) + '_' + opcode
                machine = immediate + dec2bin(operand[1][leftBracket + 2 : -1]) + func3 + dec2bin(int(operand[0][1:])) + opcode
        else:
            if len(operand) != 3:
                print('======== Error ========')
                print(f'{instruction} expect 3 operands !!')
                print('=======================')
            else:
                immediate = generate_immediate(int(operand[2]), 12)
                comment = immediate + '_' + dec2bin(operand[1][1:]) + '_' + func3 + '_' + dec2bin(int(operand[0][1:])) + '_' + opcode
                machine = immediate + dec2bin(operand[1][1:]) + func3 + dec2bin(int(operand[0][1:])) + opcode
    elif instruction_type == 'S':
        if len(operand) != 2:
            print('======== Error ========')
            print(f'{instruction} expect 2 operands !!')
            print('=======================')
        else:
            leftBracket = operand[1].find('(')
            immediate = generate_immediate(int(operand[1][:leftBracket]), 12)
            comment = immediate[0:7] + '_' + dec2bin(int(operand[0][1:])) + '_' + dec2bin(int(operand[1][leftBracket + 2 : -1])) + '_' + func3 + '_' + immediate[7:] + '_' + opcode
            machine = immediate[0:7] + dec2bin(int(operand[0][1:])) + dec2bin(int(operand[1][leftBracket + 2 : -1])) + func3 + immediate[7:] + opcode
    elif instruction_type == 'SB':
        if len(operand) != 3:
            print('======== Error ========')
            print(f'{instruction} expect 3 operands !!')
            print('=======================')
        else:
            immediate = generate_immediate(int(operand[2]), 13)
            comment = immediate[0] + '_' + immediate[2:8] + '_' + dec2bin(int(operand[1][1:])) + '_' + dec2bin(int(operand[0][1:])) + '_' + func3 + '_' + immediate[8:-1] + '_' + immediate[1] + '_' + opcode
            machine = immediate[0] + immediate[2:8] + dec2bin(int(operand[1][1:])) + dec2bin(int(operand[0][1:])) + func3 + immediate[8:-1] + immediate[1] + opcode
    elif instruction_type == 'UJ':
        if len(operand) != 2:
            print('======== Error ========')
            print(f'{instruction} expect 2 operands !!')
            print('=======================')
        else:
            immediate = generate_immediate(int(operand[1]), 21)
            comment = immediate[0] + '_' + immediate[10:20] + '_' + immediate[9] + '_' + immediate[1:9] + '_' + dec2bin(int(operand[0][1:])) + '_' + opcode
            machine = immediate[0] + immediate[10:20] + immediate[9] + immediate[1:9] + dec2bin(int(operand[0][1:])) + opcode
    else:
        print('======== Error ========')
        print(f'{data} is unknown instruction !!')
        print('=======================')

    machineHexBig = bin2hex(machine)
    if machineHexBig == 'XX_XX_XX_XX':
        print(data)
    return f"{machineHexBig : <8}    //{comment : <38}     //{data}\n";

def generate(inputfile, outputfile):
    instructions = []
    with open(inputfile, 'r') as f:
        for line in f:
            if line != '':
                instructions.append(parseData(line.strip()))
    with open(outputfile, 'w+') as f:
        for instruction in instructions:
            f.write(instruction)

if __name__ == '__main__':
    inputfile = 'D:\\IC-design\\DSD\\RISCV\\RISCV\\Baseline\\src\\pattern\\assembly.dat'
    outputfile = 'D:\\IC-design\\DSD\\RISCV\\RISCV\\Baseline\\src\\pattern\\machine.dat'
    generate(inputfile, outputfile)