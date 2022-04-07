def num_check(n):
    assert(n < 2**16)
    assert(n >= 0)

def little_endian_conv(str):
    normal = (str.find("|")==-1)and(str.find("/")==-1)
    str = str.replace("_","")
    str = str.replace(" ","")
    str = str.replace("/","")
    str = str.replace("|","")
    substr_0 = "{:x}".format(int(str[0:8]  , base=2)).zfill(2).upper()
    substr_1 = "{:x}".format(int(str[8:16] , base=2)).zfill(2).upper()
    substr_2 = "{:x}".format(int(str[16:24], base=2)).zfill(2).upper()
    substr_3 = "{:x}".format(int(str[24:32], base=2)).zfill(2).upper()
    if (normal):
        out_str = substr_3+"_"+substr_2+"_"+substr_1+"_"+substr_0
    else:
        out_str = substr_1+"_"+substr_0+"_"+substr_3+"_"+substr_2
    return out_str

def underline(n,std=1):
    if (std):
        num_str = "{:b}".format(n).zfill(32)
        out_str = num_str[0:4]
        for i in range(1,8):
            out_str = out_str + "_" + num_str[4*i:4*i+4]
    else:
        num_str = "{:b}".format(n).zfill(16)
        out_str = num_str[0:3]
        for i in range(1,4):
            out_str = out_str + "_" + num_str[4*i-1:4*i+3]
        out_str = out_str + "_" + num_str[15]
    return out_str

# modify multiplicand and multiplier here
# you can also modify the base
a = int("CCCC", base=16)
b = int("8763", base=16)

num_check(a)
num_check(b)

a_shift = a << 15

a_LSB = a%(2**8)
a_MSB = (a>>8)%(2**8)
b_LSB = b%(2**8)
b_MSB = (b>>8)%(2**8)

tb_seq = ["{:x}".format(a).zfill(8).upper(), "{:x}".format(b).zfill(8).upper()]
LSB_seq = []
add_seq = []
op_1_seq = []
op_2_seq = []
op_3_seq = []
renew_1_seq = []
renew_2_seq = []

output = 0
b_op = b
for i in range(16):
    # LSB
    LSB = b_op % 2
    LSB_seq = LSB_seq + [LSB]
    # add
    add_seq = add_seq + [LSB*a]
    # shift-addition
    op_1_seq = op_1_seq + [output]
    output = output >> 1
    op_2_seq = op_2_seq + [output]
    output = int(output + (LSB*a_shift))
    op_3_seq = op_3_seq + [output]
    # renew LSB
    renew_1_seq = renew_1_seq + [b_op]
    b_op = b_op >> 1
    renew_2_seq = renew_2_seq + [b_op]
    # for tb
    tb_seq = tb_seq + ["{:x}".format(output).zfill(8).upper()]

print(tb_seq[0]+" * "+tb_seq[1]+" = "+tb_seq[-1])

I_mem_compression_file = open("I_mem_compression","w")
with open("I_mem_compression_ref", "r") as f:
    for line in f:
        # Inst_mem part
        if "//0x48//" in line:
            front = line.find("000011011110_00000_000_01000_0010011")
            back  = front + len("000011011110_00000_000_01000_0010011")
            line = line.replace("000011011110", "{:b}".format(a_MSB).zfill(12))
            line = line.replace("0x0DE", "0x"+"{:x}".format(a_MSB).zfill(3).upper())
            line = line.replace("13_04_E0_0D", little_endian_conv(line[front:back]))
        if "//0x4C//" in line:
            line = line.replace("0x0AD", "0x"+"{:x}".format(a_LSB).zfill(3).upper())
        if "//0x50//" in line:
            front = line.find("000010101101_0100_______/_0_010_00000_0100011")
            back  = front + len("000010101101_0100_______/_0_010_00000_0100011")
            line = line.replace("000010101101", "{:b}".format(a_LSB).zfill(12))
            line = line.replace("D4_0A_23_20", little_endian_conv(line[front:back]))
        if "//0x58//" in line:
            front = line.find("000011110110_00000_000_01000_0010011")
            back  = front + len("000011110110_00000_000_01000_0010011")
            line = line.replace("000011110110", "{:b}".format(b_MSB).zfill(12))
            line = line.replace("0x0F6", "0x"+"{:x}".format(b_MSB).zfill(3).upper())
            line = line.replace("13_04_60_0F", little_endian_conv(line[front:back]))
        if "//0x5C//" in line:
            line = line.replace("0x020", "0x"+"{:x}".format(b_LSB).zfill(3).upper())
        if "//0x60//" in line:
            front = line.find("000000100000_0100_______/_0_010_00100_0100011")
            back  = front + len("000000100000_0100_______/_0_010_00100_0100011")
            line = line.replace("000000100000", "{:b}".format(b_LSB).zfill(12))
            line = line.replace("04_02_23_22", little_endian_conv(line[front:back]))
        # Assembly part
        if "//    0x10     //" in line:
            line = line.replace("DEAD", "{:x}".format(a).zfill(4).upper())
            line = line.replace("F620", "{:x}".format(b).zfill(4).upper())
        if "//    0x48     //" in line:
            line = line.replace("0x0DE", "0x"+"{:x}".format(a_MSB).zfill(3).upper())
        if "//    0x4C     //" in line:
            line = line.replace("0x0AD", "0x"+"{:x}".format(a_LSB).zfill(3).upper())
        if "//    0x58     //" in line:
            line = line.replace("0x0F6", "0x"+"{:x}".format(b_MSB).zfill(3).upper())
        if "//    0x5C     //" in line:
            line = line.replace("0x020", "0x"+"{:x}".format(b_LSB).zfill(3).upper())
        # Explanation part
        if "0x0000DEAD * 0x0000F620 = 0xD61613A0" in line:
            line = line.replace("DEAD", "{:x}".format(a).zfill(4).upper())
            line = line.replace("F620", "{:x}".format(b).zfill(4).upper())
            line = line.replace("D61613A0", "{:x}".format(a*b).zfill(8).upper())
        if "Explanation" in line:
            break
        I_mem_compression_file.write(line)

    # Multiplication
    I_mem_compression_file.write("//\n")
    I_mem_compression_file.write("//                "+underline(a)+"\n")
    I_mem_compression_file.write("//             *) "+underline(b)+"\n")
    I_mem_compression_file.write("//             -------------------------------------------------\n")
    I_mem_compression_file.write("//                "+underline(a*b)+"\n")
    I_mem_compression_file.write("//\n")

    # Iterative shift-add operation
    for i in range(16):
        I_mem_compression_file.write("//\n")
        I_mem_compression_file.write("//     "+"{:>2}".format(i)+")\n")
        I_mem_compression_file.write("//         LSB = "+"{:b}".format(LSB_seq[i]))
        I_mem_compression_file.write(" -> add "+underline(add_seq[i],std=0)+"\n")
        I_mem_compression_file.write("//         Addition:\n")
        I_mem_compression_file.write("//                "+underline(op_1_seq[i])+" >> 1\n")
        I_mem_compression_file.write("//             -------------------------------------------------\n")
        I_mem_compression_file.write("//                "+underline(op_2_seq[i])+"\n")
        I_mem_compression_file.write("//             +)  "+underline(add_seq[i],std=0)+"\n")
        I_mem_compression_file.write("//             -------------------------------------------------\n")
        I_mem_compression_file.write("//                "+underline(op_3_seq[i])+"\n")
        I_mem_compression_file.write("//         Renew LSB:\n")
        I_mem_compression_file.write("//                "+underline(renew_1_seq[i])+" >> 1\n")
        I_mem_compression_file.write("//             -------------------------------------------------\n")
        I_mem_compression_file.write("//                "+underline(renew_2_seq[i])+"\n")       
    I_mem_compression_file.write("//\n//\n//\n")
        
I_mem_compression_file.close()

I_mem_decompression_file = open("I_mem_decompression","w")
with open("I_mem_decompression_ref", "r") as f:
    for line in f:
        # Inst_mem part
        if "//0x70//" in line:
            front = line.find("000011011110_00000_000_01000_0010011")
            back  = front + len("000011011110_00000_000_01000_0010011")
            line = line.replace("000011011110", "{:b}".format(a_MSB).zfill(12))
            line = line.replace("0x0DE", "0x"+"{:x}".format(a_MSB).zfill(3).upper())
            line = line.replace("13_04_E0_0D", little_endian_conv(line[front:back]))
        if "//0x78//" in line:
            front = line.find("000010101101_01000_000_01000_0010011")
            back  = front + len("000010101101_01000_000_01000_0010011")
            line = line.replace("000010101101", "{:b}".format(a_LSB).zfill(12))
            line = line.replace("0x0AD", "0x"+"{:x}".format(a_LSB).zfill(3).upper())
            line = line.replace("13_04_D4_0A", little_endian_conv(line[front:back]))
        if "//0x84//" in line:
            front = line.find("000011110110_00000_000_01000_0010011")
            back  = front + len("000011110110_00000_000_01000_0010011")
            line = line.replace("000011110110", "{:b}".format(b_MSB).zfill(12))
            line = line.replace("0x0F6", "0x"+"{:x}".format(b_MSB).zfill(3).upper())
            line = line.replace("13_04_60_0F", little_endian_conv(line[front:back]))
        if "//0x8C//" in line:
            front = line.find("000000100000_01000_000_01000_0010011")
            back  = front + len("000000100000_01000_000_01000_0010011")
            line = line.replace("000000100000", "{:b}".format(b_LSB).zfill(12))
            line = line.replace("0x020", "0x"+"{:x}".format(b_LSB).zfill(3).upper())
            line = line.replace("13_04_04_02", little_endian_conv(line[front:back]))
        # Assembly part
        if "//    0x18     //" in line:
            line = line.replace("DEAD", "{:x}".format(a).zfill(4).upper())
        if "//    0x1C     //" in line:
            line = line.replace("F620", "{:x}".format(b).zfill(4).upper())
        if "//    0x70     //" in line:
            line = line.replace("0x0DE", "0x"+"{:x}".format(a_MSB).zfill(3).upper())
        if "//    0x78     //" in line:
            line = line.replace("0x0AD", "0x"+"{:x}".format(a_LSB).zfill(3).upper())
        if "//    0x84     //" in line:
            line = line.replace("0x0F6", "0x"+"{:x}".format(b_MSB).zfill(3).upper())
        if "//    0x8C     //" in line:
            line = line.replace("0x020", "0x"+"{:x}".format(b_LSB).zfill(3).upper())
        # Explanation part
        if "0x0000DEAD * 0x0000F620 = 0xD61613A0" in line:
            line = line.replace("DEAD", "{:x}".format(a).zfill(4).upper())
            line = line.replace("F620", "{:x}".format(b).zfill(4).upper())
            line = line.replace("D61613A0", "{:x}".format(a*b).zfill(8).upper())
        if "Explanation" in line:
            break
        I_mem_decompression_file.write(line)

    # Multiplication
    I_mem_decompression_file.write("//\n")
    I_mem_decompression_file.write("//                "+underline(a)+"\n")
    I_mem_decompression_file.write("//             *) "+underline(b)+"\n")
    I_mem_decompression_file.write("//             -------------------------------------------------\n")
    I_mem_decompression_file.write("//                "+underline(a*b)+"\n")
    I_mem_decompression_file.write("//\n")

    # Iterative shift-add operation
    for i in range(16):
        I_mem_decompression_file.write("//\n")
        I_mem_decompression_file.write("//     "+"{:>2}".format(i)+")\n")
        I_mem_decompression_file.write("//         LSB = "+"{:b}".format(LSB_seq[i]))
        I_mem_decompression_file.write(" -> add "+underline(add_seq[i],std=0)+"\n")
        I_mem_decompression_file.write("//         Addition:\n")
        I_mem_decompression_file.write("//                "+underline(op_1_seq[i])+" >> 1\n")
        I_mem_decompression_file.write("//             -------------------------------------------------\n")
        I_mem_decompression_file.write("//                "+underline(op_2_seq[i])+"\n")
        I_mem_decompression_file.write("//             +)  "+underline(add_seq[i],std=0)+"\n")
        I_mem_decompression_file.write("//             -------------------------------------------------\n")
        I_mem_decompression_file.write("//                "+underline(op_3_seq[i])+"\n")
        I_mem_decompression_file.write("//         Renew LSB:\n")
        I_mem_decompression_file.write("//                "+underline(renew_1_seq[i])+" >> 1\n")
        I_mem_decompression_file.write("//             -------------------------------------------------\n")
        I_mem_decompression_file.write("//                "+underline(renew_2_seq[i])+"\n")       
    I_mem_decompression_file.write("//\n//\n//\n")
        
I_mem_decompression_file.close()

TestBed_compression_file = open("TestBed_compression.v","w")
with open("TestBed_compression_ref.v", "r") as f:
    for line in f:
        if "5'd0 :	answer" in line:
            line = line.replace("0000DEAD", tb_seq[0])
        if "5'd1 :	answer" in line:
            line = line.replace("0000F620", tb_seq[1])
        if "5'd2 :	answer" in line:
            line = line.replace("00000000", tb_seq[2])
        if "5'd3 :	answer" in line:
            line = line.replace("00000000", tb_seq[3])
        if "5'd4 :	answer" in line:
            line = line.replace("00000000", tb_seq[4])
        if "5'd5 :	answer" in line:
            line = line.replace("00000000", tb_seq[5])
        if "5'd6 :	answer" in line:
            line = line.replace("00000000", tb_seq[6])
        if "5'd7 :	answer" in line:
            line = line.replace("6F568000", tb_seq[7])
        if "5'd8 :	answer" in line:
            line = line.replace("37AB4000", tb_seq[8])
        if "5'd9 :	answer" in line:
            line = line.replace("1BD5A000", tb_seq[9])
        if "5'd10:	answer" in line:
            line = line.replace("0DEAD000", tb_seq[10])
        if "5'd11:	answer" in line:
            line = line.replace("764BE800", tb_seq[11])
        if "5'd12:	answer" in line:
            line = line.replace("AA7C7400", tb_seq[12])
        if "5'd13:	answer" in line:
            line = line.replace("553E3A00", tb_seq[13])
        if "5'd14:	answer" in line:
            line = line.replace("99F59D00", tb_seq[14])
        if "5'd15:	answer" in line:
            line = line.replace("BC514E80", tb_seq[15])
        if "5'd16:	answer" in line:
            line = line.replace("CD7F2740", tb_seq[16])
        if "5'd17:	answer" in line:
            line = line.replace("D61613A0", tb_seq[17])
        TestBed_compression_file.write(line)
TestBed_compression_file.close()
