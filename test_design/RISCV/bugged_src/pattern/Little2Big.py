import numpy as np


def parseData(inputfile, outputfile):
    instructions = []
    with open(inputfile, 'r') as f:
        for line in f :
            little   = line[2:10]
            comment  = line[10:]
            big      = little[6:].upper() + '_' + little[4:6].upper() + '_' + little[2:4].upper() + '_' + little[0:2].upper()
            print(big + comment)
            instructions.append(big + comment)

    with open(outputfile, 'w+') as f:
        for instruction in instructions:
            f.write(instruction)

if __name__ == "__main__":
    parseData('D:\\IC-design\\DSD\\RISCV\\RISCV\\Baseline\\src\\pattern\\I_mem_mergeSort.txt', 'D:\\IC-design\\DSD\\RISCV\\RISCV\\Baseline\\src\\pattern\\I_mem_mergeSort')