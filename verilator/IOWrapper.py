#!/usr/bin/env python3

import re
import json
import argparse

def signalWidth2Type(width):
    if(width <= 8):
        return 'C'
    elif(width <= 16):
        return 'S'
    elif(width <= 32):
        return 'I'
    elif(width <= 64):
        return 'Q'
    else:
        return 'W'

parser = argparse.ArgumentParser()
parser.add_argument('top')
parser.add_argument('Vdesign_under_test___024root_h')
parser.add_argument('design_under_test_json')
parser.add_argument('design_under_test_log')
parser.add_argument('interface_template_hpp')
parser.add_argument('interface_hpp')
parser.add_argument('control_template')
parser.add_argument('control_hpp')
args = parser.parse_args()

design_top_name = args.top
verilater_root_header_path = args.Vdesign_under_test___024root_h
yosys_design_json_path = args.design_under_test_json
yosys_design_log_path = args.design_under_test_log
dut_interface_template_path = args.interface_template_hpp
dut_interface_hpp_path = args.interface_hpp
dut_control_template_path = args.control_template
dut_control_hpp_path = args.control_hpp

print('Generating {} ...'.format(dut_interface_hpp_path))

modules2cell = {}
name2bit = {}
input_list = []
output_list = []
net_list = []
dff_list = []
reg_list = []
reg_type_list = []
mem_list = []
mem_type_list = []
mem_num_list = []

pattern_dff = re.compile(r"Creating register for signal `(.*)' using process `.*")
pattern_reg = re.compile(r"\s+([CSIQEW])Data\/\*(\d+).* (.*);")
pattern_mem = re.compile(r"\s+VlUnpacked<([CSIQEW])Data\/\*(\d+):0\*\/, (\d+)> (.*);")
dut_interface_signal = '\t\t{}.push_back(new Signal{}("{}", &rootp->{}, {}));\n'
dut_interface_type_dict = {}

with open(yosys_design_json_path) as yosys_design_json_f:
    yosys_design_json = json.load(yosys_design_json_f)
        
    for port in yosys_design_json['modules'][design_top_name]['ports']:
        if(yosys_design_json['modules'][design_top_name]['ports'][port]['direction'] == 'input'):
            input_list.append(port)
            name2bit[port] = len(yosys_design_json['modules'][design_top_name]['ports'][port]['bits'])
        elif(yosys_design_json['modules'][design_top_name]['ports'][port]['direction'] == 'output'):
            output_list.append(port)
            name2bit[port] = len(yosys_design_json['modules'][design_top_name]['ports'][port]['bits'])

    for module in yosys_design_json['modules']:
        for net in yosys_design_json['modules'][module]['netnames']:
            net_list.append(net.replace('.', '__DOT__'))
            name2bit[net] = len(yosys_design_json['modules'][module]['netnames'][net]['bits'])

        for cell in yosys_design_json['modules'][module]['cells']:
            if(yosys_design_json['modules'][module]['cells'][cell]['type'] in yosys_design_json['modules'].keys()):
                modules2cell[yosys_design_json['modules'][module]['cells'][cell]['type']] = cell

with open(yosys_design_log_path) as yosys_design_log_f:
    while True:
        line = yosys_design_log_f.readline()
        if(not line):
            break
        else:
            if(line.find('Executing PROC_DFF pass (convert process syncs to FFs).') != -1):
                break

    while True:
        line = yosys_design_log_f.readline()
        if(not line):
            break
        else:
            m = pattern_dff.match(line)
            if(m):
                name = m.group(1)
                for module in modules2cell.keys():
                    name = name.replace(module, modules2cell[module])
                dff_list.append(name.replace('\\', '').replace('.', '__DOT__'))

#print(*dff_list, sep='\n')

with open(verilater_root_header_path) as verilater_root_header_f:
    while True:
        line = verilater_root_header_f.readline()
        if(not line):
            break
        else:
            m = pattern_reg.match(line)
            if(m and [x for x in dff_list if m.group(3).find(x) != -1]):
                reg_type_list.append(m.group(1))
                name2bit[m.group(3)] = int(m.group(2))+1
                reg_list.append(m.group(3))
            else:
                m_mem = pattern_mem.match(line)
                if(m_mem):
                    mem_list.append(m_mem.group(4))
                    mem_type_list.append(m_mem.group(1))
                    mem_num_list.append(m_mem.group(3))
                    name2bit[m_mem.group(4)] = int(m_mem.group(2))+1

dut_interface_template = []
with open(dut_interface_template_path) as dut_interface_template_f:
    dut_interface_template = dut_interface_template_f.readlines()

with open(dut_interface_hpp_path, 'w') as dut_interface_hpp_f:
    for line in dut_interface_template:
        dut_interface_hpp_f.write(line)
        
        if(line.find('// CLK Init') != -1):
            clk_signal = [x for x in input_list if (x.find('clk') != -1)]
            if(len(clk_signal) != 1):
                raise Exception("DUT clk signal undefined!!!")
            else:
                input_list.remove(clk_signal[0])
            dut_interface_hpp_f.write( dut_interface_signal.format('clk', 'C', clk_signal[0], clk_signal[0], 1) )
            
        elif(line.find('// RST Init') != -1):
            rst_signal = [x for x in input_list if (x.find('rst') != -1)]
            rst_signal += [x for x in input_list if (x.find('reset') != -1)]
            if(len(rst_signal) != 1):
                raise Exception("DUT rst signal undefined!!!")
            else:
                input_list.remove(rst_signal[0])
            dut_interface_hpp_f.write( dut_interface_signal.format('rst', 'C', rst_signal[0], rst_signal[0], 1) )
            
        elif(line.find('// PI Init List') != -1):
            for pi in input_list:
                dut_interface_hpp_f.write( dut_interface_signal.format('pi', signalWidth2Type(name2bit[pi]), pi, pi, name2bit[pi]) )

        elif(line.find('// PO Init List') != -1):
            for po in output_list:
                dut_interface_hpp_f.write( dut_interface_signal.format('po', signalWidth2Type(name2bit[po]), po, po, name2bit[po]) )

        elif(line.find('// REG Init List') != -1):
            for reg in reg_list:
                if(reg.find('i_clk') != -1 or reg.find('i_reset') != -1):
                    continue

                temp = reg.replace(design_top_name, '').replace('__DOT__', '')
                if(temp in output_list):
                    dut_interface_hpp_f.write( dut_interface_signal.format('reg', signalWidth2Type(name2bit[temp]), reg, temp, name2bit[temp]) )
                else:
                    dut_interface_hpp_f.write( dut_interface_signal.format('reg', signalWidth2Type(name2bit[reg]), reg, reg, name2bit[reg]) )

        elif(line.find('// MEM Init List') != -1):
            for i, mem in enumerate(mem_list):
                for j in range(0, int(mem_num_list[i])):
                    idx_str = "[{}]".format(j)
                    dut_interface_hpp_f.write( dut_interface_signal.format('mem', mem_type_list[i], mem+idx_str, mem+idx_str, name2bit[mem]) )


print('{} is generated.'.format(dut_interface_hpp_path))

pattern_module = re.compile(r"FSM .+ from module `(.+)':")
pattern_variable = re.compile(r"\s*Information on FSM .*\(\\(.*)\):")
pattern_encoding = re.compile(r"\s*(\d+):\s*\d+'([0|1]+).*")
pattern_transition = re.compile(r"\s*(\d+):\s*(\d+).*->\s+(\d+).*")
state_signal_name = []
state_module = []
state_variable = []
state_variable_index = []
state_encoding = []
state_transition = []

with open(yosys_design_log_path) as yosys_design_log_f:
    while True:
        line = yosys_design_log_f.readline()
        if(not line):
            break
        else:
            if(line.find('Executing FSM_INFO pass (dumping all available information on FSM cells).') != -1):
                break

    while True:
        line = yosys_design_log_f.readline()
        if(not line):
            break

        elif(line.find('from module') != -1):
            m = pattern_module.match(line)
            state_module.append(m.group(1))

        elif(line.find('Information on FSM') != -1):
            m = pattern_variable.match(line)
            state_variable.append(m.group(1))
        
        elif(line.find('State encoding:') != -1):
            tempList = []
            line = yosys_design_log_f.readline()
            while(True):
                m = pattern_encoding.match(line)
                if(m == None):
                    break
                tempList.append( (m.group(1), m.group(2)) )
                line = yosys_design_log_f.readline()
            state_encoding.append(tempList)

        elif(line.find('Transition Table (state_in, ctrl_in, state_out, ctrl_out):') != -1):
            tempSet = set()
            line = yosys_design_log_f.readline()
            while(True):
                m = pattern_transition.match(line)
                if(m == None):
                    break
                tempSet.add( (m.group(2), m.group(3)) )
                line = yosys_design_log_f.readline()
            state_transition.append(tempSet)

#print(state_module)
#print(state_variable)
#print(state_encoding)
#print(state_transition)

for i in range(0, len(state_variable)):
    if( modules2cell.get(state_module[i]) ):
        state_name = design_top_name + '__DOT__' + (modules2cell[state_module[i]] + '.' + state_variable[i]).replace('.','__DOT__')
    else:
        state_name = design_top_name + '__DOT__' + state_variable[i].replace('.','__DOT__')

    state_signal_name.append(state_name)
    state_variable_index.append( reg_list.index( state_name ) )

dut_control_template = []
with open(dut_control_template_path) as dut_control_template_f:
    dut_control_template = dut_control_template_f.readlines()

with open(dut_control_hpp_path, 'w') as dut_control_hpp_f:
    for line in dut_control_template:
        dut_control_hpp_f.write(line)
        
        if(line.find('// State Inst') != -1):
            for i in range(0, len(state_variable)):
                dut_control_hpp_f.write('\t\tfsm_list.emplace(fsm_list.end(), {}, \"{}\", {});\n'.format(i, state_signal_name[i], state_variable_index[i]))
                for state in state_encoding[i]:
                    dut_control_hpp_f.write("\t\tfsm_list[{}].states.emplace(fsm_list[{}].states.end(),\n".format(i, i))
                    dut_control_hpp_f.write("\t\t\t\t\t\t\t\t\t{},\n".format(state[0]))
                    dut_control_hpp_f.write("\t\t\t\t\t\t\t\t\t0b{},\n".format(state[1]))
                    dut_control_hpp_f.write("\t\t\t\t\t\t\t\t\tstd::vector<unsigned>{")
                    flag = True
                    for tran in state_transition[i]:
                        if(tran[0] == state[0]):
                            if(not flag):
                                dut_control_hpp_f.write(", {}".format(tran[1]));
                            else:
                                dut_control_hpp_f.write("{}".format(tran[1]));
                                flag = False
                    dut_control_hpp_f.write("}\n\t\t);\n")
                dut_control_hpp_f.write("\n")
                    
print('{} is generated.'.format(dut_control_hpp_path))
