#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvMsg.h"
#include "gvSimCmd.h"
#include "gvAbcMgr.h"                                                                                                           
#include <string>
#include "util.h"
#include <fstream>

#include "gvNtk.h"
bool GVinitSimCmd() {
    return (
         gvCmdMgr->regCmd("RAndom Sim",   2, 1, new GVRandomSimCmd   ) &&
         gvCmdMgr->regCmd("SEt SAfe"  ,   2, 2, new GVRandomSetSafe  ) &&
         gvCmdMgr->regCmd("CRP G",        3, 1, new GVCRPG)
    );
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
    command += "-top " + yosys_design->top_module()->name.str().substr(1,strlen(yosys_design->top_module()->name.c_str()) - 1);
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
            in_file_name = options[i];
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

    if(!file_name_set)
        command += " -input " + gvModMgr -> getInputFileName();
    // cout << "safe  =========================== " + gvModMgr -> getSafe() << endl;
    if(gvModMgr -> getSafe() != -1)
        command += " -safe " + std::to_string((gvModMgr -> getSafe()));

    cout << command <<"\n";
    // execute "random_sim" command
    run_pass(command);
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] [-rst rst_name] [-rst_n rst_n_name] [-clk clk_name] [-output out_file_name] [-v verbose print result] [-file stimulus]" << endl;
}

void
GVRandomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: " << "Conduct random simulation and print the results." << endl;
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------

GVCmdExecStatus
GVRandomSetSafe::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSetSafe " << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if(options.size() != 1)
    {
        gvMsg(GV_MSG_IFO) << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    gvModMgr -> setSafe(stoi(options[0]));
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSetSafe::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SAfe <#PO>" << endl;
}

void
GVRandomSetSafe::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt SAfe: " << "Set safe rpoperty for random sim." << endl;
}

//----------------------------------------------------------------------
// CRPG
//----------------------------------------------------------------------

// simplest mkdir
void
GVmkdir(const string& folder_path){
    string remove_cmd = "rm -fr " + folder_path;
    string build_cmd = "mkdir -p " + folder_path;
    system(remove_cmd.c_str());
    system(build_cmd.c_str());
}

// CRPG step 1. Initialize
void
CRPG_init(string build_path, string work_path){
    gvMsg(GV_MSG_IFO) << "[INFO] Initializing build file" << endl;
    GVmkdir(build_path);
    GVmkdir(work_path);
}

// CRPG step 2. Extract RTL info
void
CRPG_getDesignInfo(string top_module, string build_path){
    gvMsg(GV_MSG_IFO) << "[INFO] Extracting information from design" << endl;
    run_pass("prep -top " + top_module);
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
    run_pass("write_json " + build_path + "/design_under_test.json");
}

// CRPG step 3. DUT preprocessing
void
CRPG_dutPreprocessing(string build_path){
    gvMsg(GV_MSG_IFO) << "[INFO] DUT preprocessing..." << endl;
    ofstream ofs_assertion_hpp;
    string dpi_template = \
    "// DPI Header\
    import \"DPI-C\" function void set_assert_flag (input int index);\
    import \"DPI-C\" function void set_constraint_flag (input int index);\
    import \"DPI-C\" function void set_stable_flag (input int index);\
    ";
    vector<RTLIL::Module*> modules_vec = yosys_design->modules().to_vector();
    for(int i= 0; i< modules_vec.size(); i++){
        cout << modules_vec[i]->name.str() << endl;
    }
}

GVCmdExecStatus
GVCRPG ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "[INFO] I am GVCRPG " << endl;
    
    string top_module = yosys_design->top_module()->name.str().substr(1,strlen(yosys_design->top_module()->name.c_str()) - 1);
    
    string curr_path = getcwd(NULL, 0);
    string build_path = curr_path + "/build";
    string work_path = curr_path + "/work";
    
    // Step 1. Initialize
    CRPG_init(build_path, work_path);
    // Step 2. Extract RTL info
    CRPG_getDesignInfo(top_module, build_path);
    // Step 3. DUT preprocessing
    CRPG_dutPreprocessing(build_path);
    
    /*
    // TODO
    // 3. Use verilator to generate C++ simulator code
    gvMsg(GV_MSG_IFO) << "[INFO] Generating C++ simulator code" << endl;
    string VFLAGS;
    VFLAGS = "--cc --public --trace --assert -O3 ";
    VFLAGS += "-CFLAGS -o3 ";
    VFLAGS += "-CFLAGS -I/home/clare/gv0/src/sim/include ";
    VFLAGS += "-y build ";
    VFLAGS += "--Mdir build/obj_dir ";
    VFLAGS += "--exe " + work_path + "/main.cpp";
    script_cmd = "verilator " + VFLAGS + " build/design_under_test.v";
    //system(script_cmd.c_str());
    return GV_CMD_EXEC_DONE;*/
}

void
GVCRPG ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: CRPG" << endl;
}

void
GVCRPG ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "CRPG: " << "Constrained Random Pattern Generation" << endl;
}
#endif