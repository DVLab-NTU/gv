import random
var_start = True
time0_start = False
count = 0
n_count = 0
with open("gen_pattern.vcd", "w") as f:
    with open("../../yosys/Final.vcd") as l:
        while True:
            text = l.readline()
            count = 0
            if text == "" or text == "#5\n":
                break
            # print(text)
            if text == "" or text == "#0\n":
                var_start = False
                f.write("#0\n")
            if var_start == True:
                f.write(text)
            if time0_start == True:
                for t in text:
                    if t == "x":
                        count += 1
                f.write("b")
                f.write("0" * count)
                f.write(" n" + str(n_count) + "\n")
                n_count += 1
            if text == "" or text == "#0\n":
                time0_start = True
    for i in range(1, 10):
        f.write("#" + str(i*10) + "\n")
        f.write("b1 n16\n")
        f.write("b")
        for j in range(128):
            f.write(random.choice(("0","1")))
        f.write(" n17\n")
        f.write("b1 n22\n")
        f.write("b")
        for j in range(128):
            f.write(random.choice(("0","1")))
        f.write(" n23\n")
        f.write("b" + str(i % 2) + " n29\n")
        