def little_endian_conv(str):
    str = str.replace("_","")
    substr_0 = "{:x}".format(int(str[0:8]  , base=2)).zfill(2).upper()
    substr_1 = "{:x}".format(int(str[8:16] , base=2)).zfill(2).upper()
    substr_2 = "{:x}".format(int(str[16:24], base=2)).zfill(2).upper()
    substr_3 = "{:x}".format(int(str[24:32], base=2)).zfill(2).upper()
    out_str = substr_3+"_"+substr_2+"_"+substr_1+"_"+substr_0
    return out_str

def fib(n):
    output = [0, 1]
    for i in range(2,n):
        a = output[i-2]
        b = output[i-1]
        output = output + [a+b]
    return output

def increment(n, arr):
    output = []
    for num in arr:
        for i in range(incre+1):
            output = output + [num + i]
    return output

# Modify number of sequence here
# Note: nb >= 3 and nb <= 47 (if nb >= 48, there will be overflow)
nb = 20
# Modify number of max increment
# if (incre == 2) generate f(i), f(i)+1, f(i)+2
# Note: nb >= 0 and nb*incre <= 1023 (size of slow memory is 1024 words)
incre = 3

#assertion
assert(nb >= 3)
assert(nb <= 48)
assert(incre >= 0)
assert(nb*incre < 1024)

fib_list  = fib(nb)
incre_list = increment(incre, fib_list)
sort_list = sorted(incre_list, reverse=True)
write_list = incre_list + sort_list
print(incre_list)

TestBed_L2Cache_file = open("TestBed_L2Cache.v","w")
with open("TestBed_L2Cache_ref.v", "r") as f:
    for line in f:
        if "`define CheckNum" in line:
            line = line.replace("10'd7", "10'd{:d}".format(nb*(incre+1)*2+1))
        TestBed_L2Cache_file.write(line)
        if "case( curaddr )" in line:
            break

for i, ans in enumerate(write_list):
    TestBed_L2Cache_file.write("\t\t10'd{:<3} : answer = 32'd{:<6};\n".format(i,ans))

TestBed_L2Cache_file.write("\t\t10'd{:<3} : answer = {};\n".format(nb*(incre+1)*2,"`EndSymbol"))
TestBed_L2Cache_file.write("\t\tendcase\n\tend\nendmodule")
TestBed_L2Cache_file.close()

I_mem_L2Cache_file = open('I_mem_L2Cache','w')
with open("I_mem_L2Cache_ref", "r") as f:
    for line in f:
        if "//0x14//" in line:
            front = line.find("000000000001_00000_000_01101_0010011")
            back  = front + len("000000000001_00000_000_01101_0010011")
            line = line.replace("000000000001", "{:b}".format(incre+1).zfill(12))
            line = line.replace("0x001", "0x"+"{:x}".format(incre+1).zfill(3).upper())
            line = line.replace("93_06_10_00", little_endian_conv(line[front:back]))
        if "//0x38//" in line:
            front = line.find("000000000011_00000_000_01011_0010011")
            back  = front + len("000000000011_00000_000_01011_0010011")
            line = line.replace("000000000011", "{:b}".format(nb).zfill(12))
            line = line.replace("0x003", "0x"+"{:x}".format(nb).zfill(3).upper())
            line = line.replace("93_05_30_00", little_endian_conv(line[front:back]))
        if "//0x7C//" in line:
            front = line.find("000000001000_00000_000_01001_0010011")
            back  = front + len("000000001000_00000_000_01001_0010011")
            line = line.replace("000000001000", "{:b}".format(nb*(incre+1)*4-4).zfill(12))
            line = line.replace("0x008", "0x"+"{:x}".format(nb*(incre+1)*4-4).zfill(3).upper())
            line = line.replace("93_04_80_00", little_endian_conv(line[front:back]))
        if "//0xB4//" in line:
            front = line.find("000000001100_00000_000_01001_0010011")
            back  = front + len("000000001100_00000_000_01001_0010011")
            line = line.replace("000000001100", "{:b}".format(nb*(incre+1)*4).zfill(12))
            line = line.replace("0x00C", "0x"+"{:x}".format(nb*(incre+1)*4).zfill(3).upper())
            line = line.replace("93_04_C0_00", little_endian_conv(line[front:back]))
        I_mem_L2Cache_file.write(line)
I_mem_L2Cache_file.close()
