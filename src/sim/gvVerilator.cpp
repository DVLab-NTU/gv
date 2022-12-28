#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "gvMsg.h"
#include "gvNtk.h"
#include "gvSimCmd.h"
#include "gvVerilator.hpp"
#include "json.hpp"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

using json = nlohmann::json;

void
VerilatorAPI::writeData(string data) {
    //// IPC - Shared Memory
    VerilatorAPI::shm = (char*)shmat(VerilatorAPI::get_shmid(), NULL, 0);
    if (VerilatorAPI::shm == (char*)-1) {
        cout << strerror(errno) << endl;
        perror("Error in Shared Memory attachment");
        exit(1);
    }

    string pattern_str = data + " ";
    cout << "[INFO] GV - Write data (size=" << pattern_str.size() - 1 << ") => Shared memory (id=" << VerilatorAPI::get_shmid() << ")" << endl;
    cout << pattern_str << endl;
    memcpy(VerilatorAPI::shm, pattern_str.c_str(), pattern_str.size());
}

void
VerilatorAPI::update() {
    system("obj_dir/Vtestf --update");
}

void
VerilatorAPI::evalOneCycle() {
    system("obj_dir/Vtestf --eval 1");
}

void
VerilatorAPI::printState() {
    system("obj_dir/Vtestf --print");
}

string
VerilatorAPI::readData() {
    //// IPC - Shared Memory
    VerilatorAPI::shm = (char*)shmat(VerilatorAPI::get_shmid(), NULL, 0);
    if (VerilatorAPI::shm == (char*)-1) {
        cout << strerror(errno) << endl;
        perror("Error in Shared Memory attachment");
        exit(1);
    }

    string pattern_str(VerilatorAPI::shm);
    string token;
    size_t pos = pattern_str.find(" ");
    token      = pattern_str.substr(0, pos);
    cout << "[INFO] GV - Shared memory (id=" << VerilatorAPI::get_shmid() << ") => Read data (size=" << token.size() << ")" << endl;
    cout << token << endl;
    return token;
}

void
VerilatorAPI::getSequence() {
    system("obj_dir/Vtestf --sequence");
}

void
VerilatorAPI::reset() {
    system("obj_dir/Vtestf --reset");
}

// Extracting all key from map
vector<string>
getAllKey(json myJson) {
    vector<string> keys;
    for (auto& item : myJson.items()) {
        keys.push_back(item.key());
    }
    // for (map<string, string>::iterator it = myMap.begin(); it != myMap.end(); it++){
    //     keys.push_back(it->first);
    // }
    return keys;
}

string
signalWidth2Type(string width) {
    int width_int = stoi(width);
    if (width_int <= 8) return "C";
    else if (width_int <= 16) return "S";
    else if (width_int <= 32) return "I";
    else if (width_int <= 64) return "Q";
    else return "W";
}

void
VerilatorAPI::verilogModeling(string testbed) {
    gvMsg(GV_MSG_IFO) << "[INFO] Generating Verilator simulator code" << endl;

    string VFLAGS, system_cmd;
    VFLAGS = "--cc -sv -Wno-lint --build --public --trace --assert -O3 ";
    VFLAGS += "--exe " + testbed;
    VFLAGS += " -CFLAGS -o3 ";
    // VFLAGS += "-CFLAGS -I/home/clare/gv0/src/sim/include ";
    // VFLAGS += "-y build ";
    // VFLAGS += "--Mdir build/obj_dir ";
    // VFLAGS += "--main ";
    // VFLAGS += "--top-module " + top_module;
    // system_cmd = "verilator " + VFLAGS + " build/design_under_test.v";
    system_cmd = "verilator " + VFLAGS + gvModMgr->getInputFileName();
    gvMsg(GV_MSG_DBG) << "[DBG] Verilator command -> " << system_cmd << endl;
    system(system_cmd.c_str());
}

// Extracting RTL info by yosys
string
VerilatorAPI::getDesignInfo() {
    string output_path = "obj_dir/design_under_test.json";
    gvMsg(GV_MSG_IFO) << "[INFO] Extracting information from design => " << output_path << endl;
    run_pass("prep");
    run_pass("proc_clean");
    run_pass("proc_init");
    run_pass("proc_arst");
    run_pass("proc_mux");
    run_pass("proc_dff");
    run_pass("proc_clean");
    run_pass("fsm_detect");
    run_pass("fsm_extract");
    run_pass("fsm_info");
    run_pass("proc");
    run_pass("write_json " + output_path);
    return output_path;
}

void
VerilatorAPI::genInterfaceFile() {
    gvMsg(GV_MSG_IFO) << "[INFO] Generating interface.hpp ..." << endl;
    ifstream input_file("obj_dir/design_under_test.json");
    ofstream output_file("obj_dir/interface.hpp");
    if (!input_file.good()) gvMsg(GV_MSG_ERR) << "[ERR] design_under_test.json doesn't exist" << endl;
    else {
        json                rtl_info;
        vector<string>      input_vec, output_vec, net_vec, key_vec;
        map<string, string> name2bit_map, module2cell;
        string              port_type, port_name, port_bits, module_name, net_name, net_bits, cell_name, cell_type;
        regex               dot("\\.");
        rtl_info << input_file;
        key_vec = getAllKey(rtl_info["modules"]);

        // get PI/PO
        for (auto& port : rtl_info["modules"]["test"]["ports"].items()) {
            port_name = port.key();
            port_type = port.value()["direction"];
            port_bits = to_string(rtl_info["modules"]["zipsystem"]["ports"][port_name]["bits"].size());
            if (port_type == "input") {
                input_vec.push_back(port_name);
                name2bit_map[port_name] = port_bits;
                // cout << port_name << endl;
                // type_info &ai = typeid(rtl_info["modules"]["zipsystem"]["ports"][key]["bits"]);
                // cout << rtl_info["modules"]["zipsystem"]["ports"][key]["bits"].size() << endl;
            } else if (port_type == "output") {
                output_vec.push_back(port_name);
                name2bit_map[port_name] = port_bits;
            }
            // cout << "Port name: " << port_name << endl;
            // cout << "Port type: " << port_type << endl;
            // cout << "Port bits: " << port_bits << endl;
        }

        // get net's info
        for (auto& module : rtl_info["modules"].items()) {
            module_name = module.key();
            for (auto& net : rtl_info["modules"][module_name]["netnames"].items()) {
                net_name = net.key();
                net_name = regex_replace(net_name, dot, "__DOT__");
                net_bits = to_string(rtl_info["modules"][module_name]["netnames"][net_name]["bits"].size());
                net_vec.push_back(net_name);
                name2bit_map[net_name] = net_bits;
                // cout << "Module name: " << module_name << endl;
                // cout << "Net name: " << net_name << endl;
                // cout << "Net bits: " << net_bits << endl;
                // for (const auto& val : net.value().items())
                //     std::cout << "  " << val.key() << ": " << val.value() << "\n";
            }
            for (auto& cell : rtl_info["modules"][module_name]["cells"].items()) {
                cell_name = cell.key();
                cell_type = rtl_info["modules"][module_name]["cells"][cell_name]["type"];
                if (find(key_vec.begin(), key_vec.end(), cell_type) != key_vec.end()) {
                    module2cell[cell_type] = cell_name;
                    // cout << "test" << cell_type << "   " << cell_name << endl;
                }
                // cout << "Cell name: " << cell_name << endl;
                // cout << "Cell type: " << cell_type << endl;
            }
        }

        // write to interface.hpp

        // signal class
        output_file << "#ifndef interface_hpp" << endl;
        output_file << "#define interface_hpp" << endl;
        output_file << "class Signal{" << endl;
        output_file << "public:" << endl;
        output_file << "    Signal(" << endl;
        output_file << "        std::string newName," << endl;
        output_file << "        void*       newValue," << endl;
        output_file << "        unsigned    newWidth" << endl;
        output_file << "    ):" << endl;
        output_file << "        name(newName)," << endl;
        output_file << "        value(newValue)," << endl;
        output_file << "        width(newWidth)" << endl;
        output_file << "    {}" << endl;
        output_file << "    " << endl;
        output_file << "    virtual const char getType() = 0;" << endl;
        output_file << "" << endl;
        output_file << "    std::string     name;" << endl;
        output_file << "    void*           value;" << endl;
        output_file << "    unsigned        width;" << endl;
        output_file << "};" << endl;
        output_file << "" << endl;
        output_file << "class SignalI : public Signal{" << endl;
        output_file << "public:" << endl;
        output_file << "    SignalI(" << endl;
        output_file << "        std::string newName," << endl;
        output_file << "        void*       newValue," << endl;
        output_file << "        unsigned    newWidth" << endl;
        output_file << "    ):" << endl;
        output_file << "        Signal(newName, newValue, newWidth)" << endl;
        output_file << "    {}" << endl;
        output_file << "    " << endl;
        output_file << "    virtual const char getType() override { return 'I'; }" << endl;
        output_file << "};" << endl;
        output_file << "" << endl;
        output_file << "class SignalC : public Signal{" << endl;
        output_file << "public:" << endl;
        output_file << "    SignalC(" << endl;
        output_file << "        std::string newName," << endl;
        output_file << "        void*       newValue," << endl;
        output_file << "        unsigned    newWidth" << endl;
        output_file << "    ):" << endl;
        output_file << "        Signal(newName, newValue, newWidth)" << endl;
        output_file << "    {}" << endl;
        output_file << "    " << endl;
        output_file << "    virtual const char getType() override { return 'C'; }" << endl;
        output_file << "};" << endl;
        output_file << "" << endl;
        output_file << "class SignalS : public Signal{" << endl;
        output_file << "public:" << endl;
        output_file << "    SignalS(" << endl;
        output_file << "        std::string newName," << endl;
        output_file << "        void*       newValue," << endl;
        output_file << "        unsigned    newWidth" << endl;
        output_file << "    ):" << endl;
        output_file << "        Signal(newName, newValue, newWidth)" << endl;
        output_file << "    {}" << endl;
        output_file << "    " << endl;
        output_file << "    virtual const char getType() override { return 'S'; }" << endl;
        output_file << "};" << endl;
        output_file << "" << endl;
        output_file << "class SignalQ : public Signal{" << endl;
        output_file << "public:" << endl;
        output_file << "    SignalQ(" << endl;
        output_file << "        std::string newName," << endl;
        output_file << "        void*       newValue," << endl;
        output_file << "        unsigned    newWidth" << endl;
        output_file << "    ):" << endl;
        output_file << "        Signal(newName, newValue, newWidth)" << endl;
        output_file << "    {}" << endl;
        output_file << "    " << endl;
        output_file << "    virtual const char getType() override { return 'Q'; }" << endl;
        output_file << "};" << endl;

        output_file << endl;
        output_file << "class Interface{" << endl;
        output_file << "public:" << endl;
        output_file << "\tInterface( Vtestf___024root* rootp ){" << endl;
        output_file << "\t\t// Primary Input" << endl;
        for (string pi : input_vec) {
            size_t           clk_found = pi.find("clk");
            array<string, 2> rst_name{"reset", "rst"};
            auto             rst_found = find_if(begin(rst_name), end(rst_name), [&](const string& s) { return pi.find(s) != string::npos; });
            if (clk_found != string::npos)
                output_file << "\t\tclk.push_back(new SignalC"
                            << "(\"" << pi << "\", &rootp->" << pi << ", 1));" << endl;
            else if (rst_found != end(rst_name))
                output_file << "\t\trst.push_back(new SignalC"
                            << "(\"" << pi << "\", &rootp->" << pi << ", 1));" << endl;
            else
                output_file << "\t\tpi.push_back(new Signal" << signalWidth2Type(name2bit_map[pi]) << "(\"" << pi << "\", &rootp->" << pi << ", "
                            << name2bit_map[pi] << "));" << endl;
        }
        output_file << "\t\t// Primary Output" << endl;
        for (string po : output_vec) {
            output_file << "\t\tpo.push_back(new Signal" << signalWidth2Type(name2bit_map[po]) << "(\"" << po << "\", &rootp->" << po << ", "
                        << name2bit_map[po] << "));" << endl;
        }

        // Search registers
        output_file << "\t\t// Registers" << endl;
        smatch m;
        regex  reg_line("gvEx_r");
        for (string reg : output_vec) {
            regex_search(reg, m, reg_line);
            if (!m.empty())
                output_file << "\t\treg.push_back(new Signal" << signalWidth2Type(name2bit_map[reg]) << "(\"" << reg << "\", &rootp->" << reg << ", "
                            << name2bit_map[reg] << "));" << endl;
        }
        // output_file << "//Register" << endl;
        output_file << "\t}" << endl;
        output_file << "\tstd::vector<Signal*> clk;" << endl;
        output_file << "\tstd::vector<Signal*> rst;" << endl;
        output_file << "\tstd::vector<Signal*> pi;" << endl;
        output_file << "\tstd::vector<Signal*> po;" << endl;
        output_file << "\tstd::vector<Signal*> reg;" << endl;
        output_file << "};" << endl;
        output_file << "#endif /* interface_hpp */" << endl;
    }
    input_file.close();
    output_file.close();
}

// Collecting assertion for all file
vector<string>
VerilatorAPI::collectAssertion(string filename) {
    ifstream       input_file(filename);
    ofstream       output_file("obj_dir/design_under_test.v");
    vector<string> assertion_vec;
    string         output_path;
    string         line;
    string         assertion_info;
    string         result;
    regex          assertion_line("assert\s*\\((.*)\\)\s*;");
    regex          assertion_word("assert");
    regex          semicolon_regex(";");
    smatch         m;
    int            line_num, assertion_cnt = 0;
    string         dpi_template = "// DPI Header\
    \rimport \"DPI-C\" function void set_assert_flag (input int index);\
    \rimport \"DPI-C\" function void set_constraint_flag (input int index);\
    \rimport \"DPI-C\" function void set_stable_flag (input int index);\n";

    gvMsg(GV_MSG_IFO) << "[INFO] Reformat the file..." << endl;
    line_num = 1;

    output_file << dpi_template << endl;
    while (getline(input_file, line)) {
        regex_search(line, m, assertion_line);
        if (!m.empty()) {
            result         = regex_replace(line, assertion_word, "Assertion_" + to_string(assertion_cnt));
            result         = regex_replace(result, semicolon_regex, " else set_assert_flag\(" + to_string(assertion_cnt) + "\)");
            assertion_info = filename + ", " + to_string(assertion_cnt) + ", " + to_string(line_num) + ", " + m[1].str();
            assertion_vec.push_back(assertion_info);
            // cout << assertion_info << endl;
            // cout << result << endl;
            assertion_cnt += 1;
        }
        output_file << line << endl;
        line_num += 1;
    }
    input_file.close();
    output_file.close();
    return assertion_vec;
}

void
VerilatorAPI::genAssertionFile(vector<string> assertion_vec) {
    gvMsg(GV_MSG_IFO) << "[INFO] Generating assertion.hpp ..." << endl;
    ofstream output_file("/obj_dir/assertion.hpp");
    output_file << "#ifndef assertion_hpp" << endl;
    output_file << "#define assertion_hpp" << endl;
    output_file << "#include \"crpg.hpp\"\n" << endl;
    output_file << "class Assertion{" << endl;
    output_file << "public:" << endl;
    output_file << "\tAssertion(){" << endl;
    for (string& assertion : assertion_vec) {
        output_file << "\t\tassertion.emplace(assertion.end(), " << assertion << ")" << endl;
    }
    output_file << "\t}" << endl;
    output_file << "\tstd::vector<Property> assertion;" << endl;
    output_file << "};" << endl;
    output_file << "#endif /* assertion_hpp */" << endl;
}

VerilatorAPI::VerilatorAPI(string testbench, int shm_size) {
    key_t key;

    // IPC - Shared Memory
    key                  = 7899;
    VerilatorAPI::_shmid = shmget(key, shm_size, IPC_CREAT | 0666);
    if (VerilatorAPI::_shmid == -1) {
        cout << strerror(errno) << endl;
        ;
        perror("[ERR] GV - Error in Shared Memory get statement");
        exit(1);
    }
    // Init.
    system("mkdir obj_dir");
    VerilatorAPI::getDesignInfo();
    VerilatorAPI::genInterfaceFile();
    VerilatorAPI::verilogModeling(testbench);
    cout << "[INFO] GV - Build Verilator Env." << endl;
}

VerilatorAPI::~VerilatorAPI() {
    cout << "[INFO] Delete shared memory => (id=" << VerilatorAPI::get_shmid() << ")" << endl;
    // string system_cmd;
    // system_cmd = "ipcrm -m " + to_string(VerilatorAPI::get_shmid());
    // cout << "[DBG] GV - Deconstruct " << system_cmd << endl;
    // system(system_cmd.c_str());
    if (shmctl(VerilatorAPI::get_shmid(), IPC_RMID, 0) == -1) {
        fprintf(stderr, "[ERR] GV - shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
}
