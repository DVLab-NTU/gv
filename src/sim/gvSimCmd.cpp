#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvSimCmd.h"
#include "gvAbcMgr.h"
#include "gvMsg.h"
#include "util.h"
#include <fstream>
#include <string>
#include "gvVerilator.hpp"

#include "gvNtk.h"
bool
GVinitSimCmd() {
    return (gvCmdMgr->regCmd("RAndom Sim", 2, 1, new GVRandomSimCmd) && gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafe) &&
            //gvCmdMgr->regCmd("CRP G", 3, 1, new GVCRPG) &&
            gvCmdMgr->regCmd("Verilator Env", 1, 1, new GVVerilatorEnv) &&
            gvCmdMgr->regCmd("Verilator Test", 1, 1, new GVVerilatorTest));
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------
GVCmdExecStatus
GVRandomSimCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSimCmd " << endl;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    string opt, rst = "reset", rst_n = "reset", clk = "clk", in_file_name, out_file_name, command = "random_sim ", vcd_file_name = ".waves.vcd";
    string stimulus_file_name, cycles;

    bool verbose = false, rst_set = false, rst_n_set, clk_set = false;
    bool out_file_name_set = false, file_name_set = false;
    command += "-top " + yosys_design->top_module()->name.str().substr(1, strlen(yosys_design->top_module()->name.c_str()) - 1);
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-v", token, 1) == 0) {
            verbose = true;
            command += " -v ";
            continue;
        }
        if (myStrNCmp("-rst", token, 3) == 0) {
            rst_set = true;
            ++i;
            rst = options[i];
            command += " -reset " + rst;
            continue;
        }
        if (myStrNCmp("-rst_n", token, 4) == 0) {
            rst_n_set = true;
            ++i;
            rst_n = options[i];
            command += " -reset_n " + rst_n;
            continue;
        }
        if (myStrNCmp("-clk", token, 1) == 0) {
            clk_set = true;
            ++i;
            clk = options[i];
            command += " -clk " + clk;
            continue;
        }
        if (myStrNCmp("-sim_cycle", token, 1) == 0) {
            rst_n_set = true;
            ++i;
            cycles = options[i];
            command += " -sim_cycle " + cycles;
            continue;
        }
        if (myStrNCmp("-input", token, 1) == 0) {
            ++i;
            in_file_name  = options[i];
            file_name_set = true;
            command += " -input " + in_file_name;
            continue;
        }
        if (myStrNCmp("-output", token, 1) == 0) {
            ++i;
            out_file_name = options[i];
            command += " -output " + out_file_name;
            continue;
        }
        if (myStrNCmp("-file", token, 1) == 0) {
            ++i;
            stimulus_file_name = options[i];
            command += " -file " + stimulus_file_name;
            continue;
        }
        if (myStrNCmp("-vcd", token, 4) == 0) {
            ++i;
            vcd_file_name = options[i];
            command += " -vcd " + vcd_file_name;
            continue;
        }
    }
    // load the random_sim plugin in yosys
    run_pass("plugin -i ./src/ext/sim.so");

    if (!file_name_set) command += " -input " + gvModMgr->getInputFileName();
    // cout << "safe  =========================== " + gvModMgr -> getSafe() << endl;
    if (gvModMgr->getSafe() != -1) command += " -safe " + std::to_string((gvModMgr->getSafe()));

    cout << command << "\n";
    // execute "random_sim" command
    run_pass(command);
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] [-rst rst_name] [-rst_n rst_n_name] [-clk clk_name] "
                         "[-output out_file_name] [-v verbose print result] [-file stimulus]"
                      << endl;
}

void
GVRandomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: "
                      << "Conduct random simulation and print the results." << endl;
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------

GVCmdExecStatus
GVRandomSetSafe::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSetSafe " << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() != 1) {
        gvMsg(GV_MSG_IFO) << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    gvModMgr->setSafe(stoi(options[0]));
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSetSafe::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SAfe <#PO>" << endl;
}

void
GVRandomSetSafe::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt SAfe: "
                      << "Set safe rpoperty for random sim." << endl;
}

//----------------------------------------------------------------------
// CRPG
//----------------------------------------------------------------------

// simplest mkdir
// void
// GVmkdir(const string& folder_path) {
//     string remove_cmd = "rm -fr " + folder_path;
//     string build_cmd  = "mkdir -p " + folder_path;
//     system(remove_cmd.c_str());
//     system(build_cmd.c_str());
// }
// 
// // Extract filename
// string
// extractFileName(const string& fullPath) {
//     const size_t lastSlashIndex = fullPath.find_last_of("/\\");
//     return fullPath.substr(lastSlashIndex + 1);
// }
// 
// // Reformat assertion
// vector<string>
// CRPG_reformatAssertion(vector<string> assertion_vec, string input_path, string output_path) {
//     gvMsg(GV_MSG_IFO) << "[INFO] Reformat the file => " << input_path << endl;
//     ifstream input_file(input_path);
//     ofstream output_file(output_path);
//     string   line;
//     string   assertion_info;
//     string   result;
//     regex    assertion_line("assert\s*\\((.*)\\)\s*;");
//     regex    assertion_word("assert");
//     regex    semicolon_regex(";");
//     smatch   m;
//     string   top_module    = yosys_design->top_module()->name.str().substr(1, strlen(yosys_design->top_module()->name.c_str()) - 1);
//     string   dpi_template  = "// DPI Header\
//     \rimport \"DPI-C\" function void set_assert_flag (input int index);\
//     \rimport \"DPI-C\" function void set_constraint_flag (input int index);\
//     \rimport \"DPI-C\" function void set_stable_flag (input int index);\n";
//     int      line_num      = 1;
//     int      assertion_cnt = assertion_vec.size();
// 
//     if (top_module + ".v" == extractFileName(input_path)) {
//         output_file << dpi_template << endl;
//     }
//     while (getline(input_file, line)) {
//         regex_search(line, m, assertion_line);
//         if (!m.empty()) {
//             result         = regex_replace(line, assertion_word, "Assertion_" + to_string(assertion_cnt));
//             result         = regex_replace(result, semicolon_regex, " else set_assert_flag\(" + to_string(assertion_cnt) + "\)");
//             assertion_info = input_path + ", " + to_string(assertion_cnt) + ", " + to_string(line_num) + ", " + m[1].str();
//             assertion_vec.push_back(assertion_info);
//             // cout << assertion_info << endl;
//             // cout << result << endl;
//             assertion_cnt += 1;
//         }
//         output_file << line << endl;
//         line_num += 1;
//     }
//     input_file.close();
//     output_file.close();
//     gvMsg(GV_MSG_IFO) << "[INFO] Finish => " << output_path << endl;
//     return assertion_vec;
// }
// 
// // Generate hpp file for verilator
// void
// CRPG_genAssertionFile(string output_path, vector<string> assertion_vec) {
//     ofstream output_file(output_path);
//     output_file << "#ifndef assertion_hpp" << endl;
//     output_file << "#define assertion_hpp" << endl;
//     output_file << "#include \"crpg.hpp\"\n" << endl;
//     output_file << "class Assertion{" << endl;
//     output_file << "public:" << endl;
//     output_file << "\tAssertion(){" << endl;
//     for (string assertion : assertion_vec) {
//         output_file << "\t\tassertion.emplace(assertion.end(), " << assertion << ")" << endl;
//     }
//     output_file << "\t}" << endl;
//     output_file << "\tstd::vector<Property> assertion;" << endl;
//     output_file << "};" << endl;
//     output_file << "#endif /* assertion_hpp */" << endl;
// }
// 
// // CRPG step 1. Initialize
// void
// CRPG_init(string build_path, string work_path) {
//     gvMsg(GV_MSG_IFO) << "[INFO] Initializing build file" << endl;
//     GVmkdir(build_path);
//     GVmkdir(work_path);
// }
// 
// // CRPG step 2. Extract RTL info
// void
// CRPG_getDesignInfo(string top_module, string build_path) {
//     gvMsg(GV_MSG_IFO) << "[INFO] Extracting information from design" << endl;
//     run_pass("prep -top " + top_module);
//     run_pass("proc_clean");
//     run_pass("proc_init");
//     run_pass("proc_arst");
//     run_pass("proc_mux");
//     run_pass("proc_dff");
//     run_pass("proc_clean");
//     run_pass("fsm_detect");
//     run_pass("fsm_extract");
//     run_pass("fsm_info");
//     run_pass("proc");
//     run_pass("write_json " + build_path + "/design_under_test.json");
// }
// 
// // CRPG step 3. DUT preprocessing
// void
// CRPG_dutPreprocessing(string build_path, vector<string> filelist) {
//     gvMsg(GV_MSG_IFO) << "[INFO] DUT preprocessing..." << endl;
//     ofstream       ofs_assertion_hpp;
//     vector<string> assertion_vec;
// 
//     // vector<RTLIL::Module*> modules_vec = yosys_design->modules().to_vector();
//     // for(int i= 0; i< modules_vec.size(); i++){
//     //     cout << modules_vec[i]->name.str() << endl;
//     // }
//     for (string file : filelist) {
//         assertion_vec = CRPG_reformatAssertion(assertion_vec, file, build_path + "/" + extractFileName(file));
//     }
// 
//     CRPG_genAssertionFile(build_path + "/obj_dir/assertion.hpp", assertion_vec);
// }
// 
// GVCmdExecStatus
// GVCRPG ::exec(const string& option) {
//     gvMsg(GV_MSG_IFO) << "[INFO] I am GVCRPG " << endl;
// 
//     string top_module = yosys_design->top_module()->name.str().substr(1, strlen(yosys_design->top_module()->name.c_str()) - 1);
// 
//     string curr_path  = getcwd(NULL, 0);
//     string build_path = curr_path + "/build";
//     string work_path  = curr_path + "/work";
// 
//     // temp
//     vector<string> options;
//     vector<string> filelist;
//     string         line;
//     GVCmdExec::lexOptions(option, options);
//     ifstream input_file(options[1]);
// 
//     if (myStrNCmp("-f", options[0], 1) == 0) {
//         while (getline(input_file, line)) {
//             filelist.push_back(line);
//         }
//     }
//     input_file.close();
// 
//     // Step 1. Initialize
//     // CRPG_init(build_path, work_path);
//     // Step 2. Extract RTL info
//     // CRPG_getDesignInfo(top_module, build_path);
//     // Step 3. DUT preprocessing
//     CRPG_dutPreprocessing(build_path, filelist);
// 
//     
//     // TODO
//     // 3. Use verilator to generate C++ simulator code
//     gvMsg(GV_MSG_IFO) << "[INFO] Generating C++ simulator code" << endl;
//     string VFLAGS;
//     VFLAGS = "--cc --public --trace --assert -O3 ";
//     VFLAGS += "-CFLAGS -o3 ";
//     VFLAGS += "-CFLAGS -I/home/clare/gv0/src/sim/include ";
//     VFLAGS += "-y build ";
//     VFLAGS += "--Mdir build/obj_dir ";
//     VFLAGS += "--exe " + work_path + "/main.cpp";
//     script_cmd = "verilator " + VFLAGS + " build/design_under_test.v";
//     //system(script_cmd.c_str());
//     return GV_CMD_EXEC_DONE;
// }
// 
// 
// void
// GVCRPG ::usage(const bool& verbose) const {
//     gvMsg(GV_MSG_IFO) << "Usage: CRPG" << endl;
// }
// 
// void
// GVCRPG ::help() const {
//     gvMsg(GV_MSG_IFO) << setw(20) << left << "CRPG: "
//                       << "Constrained Random Pattern Generation" << endl;
// }

//----------------------------------------------------------------------
// Verilator Env.
//----------------------------------------------------------------------

GVCmdExecStatus
GVVerilatorEnv::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "[INFO] I am GVVerilatorEnv" << endl;
    gvMsg(GV_MSG_IFO) << "[INFO] Compile file -> " << gvModMgr->getInputFileName() << endl;
    VerilatorAPI* verilator = new VerilatorAPI(option, 1);
    return GV_CMD_EXEC_DONE;
}

void
GVVerilatorEnv::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Verilator Env" << endl;
}

void
GVVerilatorEnv::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Verilator Env: "
                      << "Build Verilator Environment for simulation use." << endl;
}
//----------------------------------------------------------------------
// Verilator Test
//----------------------------------------------------------------------

GVCmdExecStatus
GVVerilatorTest::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "[INFO] I am GVVerilatorTest" << endl;
    VerilatorAPI* verilator = new VerilatorAPI(option, 1000000);

    // 1. reset
    verilator->reset();
    // 2. read data from shared memory
    string pattern = verilator->readData();
    // 3.1 use verilator to write data into shared memory
    verilator->getSequence();
    // 3.2 check writing correct
    string sequence = verilator->readData();
    
    // 4.1 modify pattern value
    pattern.insert(0, "3,");
    // 4.2 wrtie new pattern to shared memory 
    verilator->writeData(pattern);
    // 4.3 print current state
    verilator->printState();
    // 4.4 update new pattern to verilator
    verilator->update();
    // 4.5 Confirm successful update or not
    verilator->printState();

    // 5.1 print current state
    verilator->printState();
    // 5.2 simulate one cycle
    verilator->evalOneCycle();
    // 5.3 Confirm result
    pattern = verilator->readData();


    delete verilator;
    return GV_CMD_EXEC_DONE;
}

void
GVVerilatorTest::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Verilator Test" << endl;
}

void
GVVerilatorTest::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Verilator Test: "
                      << "Verilator for internal test" << endl;
}
#endif
