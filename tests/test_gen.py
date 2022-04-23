import random
var_start = True
time0_start = False
count = 0 # tmp counter to count the number of x's
n_count = 1 # the count of variables is from 1
with open("I_test.txt") as f:
    instruction_in = f.readline()
# print(len(instruction_in))
with open("gen_pattern.vcd", "w") as f:
    with open("final.vcd") as l:
        while True:
            text = l.readline()
            count = 0
            if text == "" or text == "#5\n":
                break
            if text == "" or text == "#0\n":
                var_start = False
                f.write("#0\n")
            if var_start == True:
                f.write(text)
            if time0_start == True:
                f.write("b")
                if n_count == 13:
                    for i, t in enumerate(text):
                        if t == "x":
                            f.write(instruction_in[i - 1])
                    f.write(" n" + str(n_count) + "\n")
                    # print(" n" + str(n_count) + "\n")
                    n_count += 1
                else:
                    # print(len(text[:text.index("n") - 1]))
                    text = text[:text.index("n") - 1]
                    for t in text:
                        if t == "x" or t == "0" or t == "1":
                            count += 1
                    f.write("0" * count)
                    f.write(" n" + str(n_count) + "\n")
                    n_count += 1
            if text == "" or text == "#0\n":
                time0_start = True
    for i in range(1, 3000):
        f.write("#" + str(100 * i) + "\n")
        f.write("b" + str(i % 2) + " n6\n")
        f.write("b1 n25\n")
    
