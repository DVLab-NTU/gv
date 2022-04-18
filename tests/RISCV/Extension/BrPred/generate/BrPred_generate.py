def little_endian_conv(str):
    str = str.replace("_","")
    substr_0 = "{:x}".format(int(str[0:8]  , base=2)).zfill(2).upper()
    substr_1 = "{:x}".format(int(str[8:16] , base=2)).zfill(2).upper()
    substr_2 = "{:x}".format(int(str[16:24], base=2)).zfill(2).upper()
    substr_3 = "{:x}".format(int(str[24:32], base=2)).zfill(2).upper()
    out_str = substr_3+"_"+substr_2+"_"+substr_1+"_"+substr_0
    return out_str

# modify # of the pattern here
nb_notBr   = 1
nb_interBr = 1
nb_Br      = 1

assert(nb_notBr > 0)
assert(nb_interBr > 0)
assert(nb_Br > 0)

print("a = {:d}, b = {:d}, c = {:d}".format(nb_notBr, nb_interBr, nb_Br))

I_mem_BrPred_file = open("I_mem_BrPred","w")
with open("I_mem_BrPred_ref", "r") as f:
    for line in f:
        if "//0x04//" in line:
            front = line.find("000000001010_00000_000_01000_0010011")
            back  = front + len("000000001010_00000_000_01000_0010011")
            line = line.replace("a = 10", "a = {:d}".format(nb_notBr))
            line = line.replace("000000001010", "{:b}".format(nb_notBr).zfill(12))
            line = line.replace("0x00A", "0x"+"{:x}".format(nb_notBr).zfill(3).upper())
            line = line.replace("13_04_A0_00", little_endian_conv(line[front:back]))
        if "//0x08//" in line:
            front = line.find("000000010100_00000_000_01001_0010011")
            back  = front + len("000000010100_00000_000_01001_0010011")
            line = line.replace("b = 20", "b = {:d}".format(nb_interBr))
            line = line.replace("000000010100", "{:b}".format(nb_interBr).zfill(12))
            line = line.replace("0x014", "0x"+"{:x}".format(nb_interBr).zfill(3).upper())
            line = line.replace("93_04_40_01", little_endian_conv(line[front:back]))
        if "//0x0C//" in line:
            front = line.find("000000011110_00000_000_01010_0010011")
            back  = front + len("000000011110_00000_000_01010_0010011")
            line = line.replace("c = 30", "c = {:d}".format(nb_Br))
            line = line.replace("000000011110", "{:b}".format(nb_Br).zfill(12))
            line = line.replace("0x01E", "0x"+"{:x}".format(nb_Br).zfill(3).upper())
            line = line.replace("13_05_E0_01", little_endian_conv(line[front:back]))
        I_mem_BrPred_file.write(line)        
I_mem_BrPred_file.close()

import shutil
shutil.copyfile("TestBed_BrPred_ref.v", "TestBed_BrPred.v")
