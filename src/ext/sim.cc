#include <kernel/consteval.h>
#include <kernel/yosys.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>
#include <vector>

#include "sim_random_compile_config.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct randomSim : public Pass {
    randomSim() : Pass("random_sim", "") {}
    void help() override {
        //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
        log("\n");
        log("    randomSim [options]\n");
        log("\n");
        log("Warning : you sould run read_verilog & hierarchy -top first\n");
        log("Run random simulation on the given verilog file\n");
        log("\n");
        log("    <-input>\n");
        log("    		The name of the verilog file you want to simulate\n");
        log("    <-top>\n");
        log("    		The top module name of the verilog file you want to simulate\n");
        log("    [-clk]\n");
        log("    		The clock name of the verilog file you want to simulate\n");
        log("    		Default to be \"clk\"\n");
        log("    [-reset]\n");
        log("    		The reset name of the verilog file you want to simulate\n");
        log("    		Default to be \"reset\"\n");
        log("    		reset when reset = 1\n");
        log("    [-reset_n]\n");
        log("    		The reset_n name of the verilog file you want to simulate\n");
        log("    		Default to be \"reset_n\"\n");
        log("    		reset when reset_n = 0\n");
        log("    [-continue]\n");
        log("    		Continue from previous cycle and state\n");
        log("    		Only one of reset, reset_n and continue should be set\n");
        log("    [-output]\n");
        log("    		The name of output file which contains the simulation result\n");
        log("    [-Verbose_printing_results]\n");
        log("    		verbose print the result of simulation on the command line (also supports -V)\n");
    }
    void execute(std::vector<std::string> args, Design* design) override {
        int sim_cycle = 20, property = -1;
        size_t argidx, num_inputs = 0;
        bool reset_set = false, property_set = false;
        bool reset_n_set = false;
        bool continue_set = false;
        bool clk_set = false;
        bool verbose = false, verilog_file_name_set = false;
        bool output_file_set = false, top_module_name_set = false, stimulus = false;
        bool vcd_file_set = false;
        std::string reset_name = "reset";
        std::string reset_n_name = "reset_n";
        std::string clk_name = "clk";
        std::string output_file_name = "sim.txt";
        std::string verilog_file_name;
        std::string top_module_name;
        std::string stimulus_file_name;
        std::string vcd_file_name;

        for (argidx = 1; argidx < args.size(); argidx++) {
            if (args[argidx] == "-sim_cycle" && argidx + 1 < args.size()) {
                sim_cycle = atoi(args[++argidx].c_str());
                continue;
            }
            if (args[argidx] == "-reset" && argidx + 1 < args.size()) {
                reset_name = args[++argidx];
                reset_set = true;
                if ((reset_set ? 1 : 0) + (reset_n_set ? 1 : 0) + (continue_set ? 1 : 0) > 1)
                    log_error("Only one of -reset, -reset_n and -continue should be set\n");
                continue;
            }
            if (args[argidx] == "-reset_n" && argidx + 1 < args.size() && reset_set == false) {
                reset_n_name = args[++argidx];
                reset_n_set = true;
                if ((reset_set ? 1 : 0) + (reset_n_set ? 1 : 0) + (continue_set ? 1 : 0) > 1)
                    log_error("Only one of -reset, -reset_n and -continue should be set\n");
                continue;
            }
            if (args[argidx] == "-continue" || args[argidx] == "-Continue") {
                continue_set = true;
                if ((reset_set ? 1 : 0) + (reset_n_set ? 1 : 0) + (continue_set ? 1 : 0) > 1)
                    log_error("Only one of -reset, -reset_n and -continue should be set\n");
                continue;
            }
            if (args[argidx] == "-clk" && argidx + 1 < args.size()) {
                clk_name = args[++argidx];
                clk_set = true;
                continue;
            }
            if (args[argidx].rfind("-V", 0) == 0) {
                // "-V" is mandatory substring, and the suffix "erbose_printing_results" is optional.
                // Supported examples: "-V", "-Verbose_printing_results".
                std::string suffix = args[argidx].substr(2); // after "-V"
                if (suffix.empty() || suffix == "erbose_printing_results")
                    verbose = true;
                continue;
            }
            if (args[argidx] == "-file" && argidx + 1 < args.size()) {
                stimulus_file_name = args[++argidx];
                stimulus = true;
                // log("innnnnnnnnnnnnn %s\n", stimulus_file_name.c_str());
                continue;
            }
            if (args[argidx] == "-output" && argidx + 1 < args.size()) {
                output_file_set = true;
                output_file_name = args[++argidx];
                continue;
            }
            if (args[argidx] == "-top" && argidx + 1 < args.size()) {
                top_module_name = args[++argidx];
                top_module_name_set = true;
                continue;
            }
            if (args[argidx] == "-input" && argidx + 1 < args.size()) {
                verilog_file_name = args[++argidx];
                verilog_file_name_set = true;
                continue;
            }
            if (args[argidx] == "-vcd" && argidx + 1 < args.size()) {
                vcd_file_name = args[++argidx];
                vcd_file_set = true;
                continue;
            }
            if (args[argidx] == "-safe" && argidx + 1 < args.size()) {
                property = stoi(args[++argidx]);
                property_set = true;
                continue;
            }
            break;
        }
        for (auto wire : design->top_module()->wires()) {
            if (wire->port_input && strcmp(wire->name.c_str(), ("\\" + reset_n_name).c_str()) &&
                strcmp(wire->name.c_str(), ("\\" + reset_name).c_str()) &&
                strcmp(wire->name.c_str(), ("\\" + clk_name).c_str()))  // check the wire is not rst or clk
                ++num_inputs;
        }
        std::string command =
            "yosys -qp \"read_verilog " + verilog_file_name + "; hierarchy -top " + top_module_name + "; write_cxxrtl .sim.cpp;\"";
        run_command(command);
        std::string wire_name;
        std::string module_name;
        std::ofstream ofs;
        ofs.open(".sim_main.cpp");
        ofs << "#include <iostream>\n";
        ofs << "#include <fstream>\n";
        ofs << "#include <stdlib.h>\n";
        ofs << "#include <time.h>\n";
        ofs << "#include <math.h>\n";
        ofs << "#include <sstream>\n";
        ofs << "#include <unordered_map>\n";
        ofs << "#include <vector>\n";
        ofs << "#include <backends/cxxrtl/cxxrtl_vcd.h>\n";
        module_name = log_id(design->top_module()->name);
        ofs << "#include \".sim.cpp\"\n";
        ofs << "using namespace std;\n";
        ofs << " int main()\n";
        ofs << "{\n";
        ofs << "srand(time(NULL));";
        ofs << "unsigned random_value = 0;\n";
        ofs << "unsigned upper_bound = 0;\n";

        ofs << "char buffer[100];\n";
        if (stimulus) {
            ofs << "vector<vector<unsigned>> stimulus_signal;\n";
            ofs << "vector<unsigned> stimulus_cycle;\n";
            ofs << "    std::ifstream ifs;\n";
            ofs << "    ifs.open(\"";
            ofs << stimulus_file_name;
            ofs << "\");\n";
            ofs << "    for(size_t i = 0; i < ";
            ofs << sim_cycle;
            ofs << "; ++i)\n";
            ofs << "    {\n";
            ofs << "        stimulus_cycle.clear();\n";
            ofs << "        for(size_t j = 0; j < ";
            ofs << num_inputs;
            ofs << "; ++j)\n";
            ofs << "        {\n";
            ofs << "            ifs >> buffer;\n";
            ofs << "            std::string tok(buffer);\n";
            ofs << "            unsigned val = 0;\n";
            ofs << "            bool numeric = !tok.empty();\n";
            ofs << "            for(char c : tok) {\n";
            ofs << "                if (c < '0' || c > '9') { numeric = false; break; }\n";
            ofs << "            }\n";
            ofs << "            if (numeric) val = static_cast<unsigned>(std::stoul(tok));\n";
            ofs << "            else val = 0;\n";
            ofs << "            stimulus_cycle.push_back(val);\n";
            ofs << "        }\n";
            ofs << "        stimulus_signal.push_back(stimulus_cycle);\n";
            ofs << "    }\n";
            ofs << "    ifs.close();\n";
        }
        if (output_file_set) {
            ofs << "ofstream ofs;\n";
            ofs << "ofs.open(\"" << output_file_name << "\");\n";
        }
        ofs << "     cxxrtl_design::p_" + module_name + " top;\n";
        ofs << "long long base_cycle = 0;\n";
        // Checkpoint file for the next -continue run: must be written after every run that
        // advances simulation (including -reset/-reset_n), not only when -continue is set.
        ofs << "const char* state_file = \".sim_state.txt\";\n";
        ofs << "cxxrtl::debug_items state_items;\n";
        ofs << "top.debug_info(state_items);\n";
        if (continue_set) {
            ofs << "std::unordered_map<std::string, cxxrtl::debug_item*> state_map;\n";
            ofs << "for (auto &entry : state_items.table) {\n";
            ofs << "    for (size_t part = 0; part < entry.second.size(); ++part) {\n";
            ofs << "        auto *dbg = &entry.second[part];\n";
            ofs << "        if (dbg->next == nullptr) continue;\n";
            ofs << "        state_map[entry.first + \"#\" + std::to_string(part)] = dbg;\n";
            ofs << "    }\n";
            ofs << "}\n";
            ofs << "std::ifstream state_ifs(state_file);\n";
            ofs << "if (state_ifs.good()) {\n";
            ofs << "    std::string line;\n";
            ofs << "    while (std::getline(state_ifs, line)) {\n";
            ofs << "        if (line.empty()) continue;\n";
            ofs << "        std::istringstream iss(line);\n";
            ofs << "        std::string key;\n";
            ofs << "        iss >> key;\n";
            ofs << "        if (key == \"__cycle__\") {\n";
            ofs << "            iss >> base_cycle;\n";
            ofs << "            continue;\n";
            ofs << "        }\n";
            ofs << "        auto it = state_map.find(key);\n";
            ofs << "        if (it == state_map.end()) continue;\n";
            ofs << "        cxxrtl::debug_item* item = it->second;\n";
            ofs << "        if (item->next == nullptr) continue;\n";
            ofs << "        size_t width = 0, chunks = 0;\n";
            ofs << "        iss >> width >> chunks;\n";
            ofs << "        if (width != item->width) continue;\n";
            ofs << "        for (size_t c = 0; c < chunks; ++c) {\n";
            ofs << "            std::string tok;\n";
            ofs << "            if (!(iss >> tok)) break;\n";
            ofs << "            uint32_t value = static_cast<uint32_t>(std::stoul(tok, nullptr, 16));\n";
            ofs << "            item->curr[c] = value;\n";
            ofs << "            item->next[c] = value;\n";
            ofs << "        }\n";
            ofs << "    }\n";
            ofs << "}\n";
            ofs << "state_ifs.close();\n";
        }
        // For VCD file.
        if (vcd_file_set) {
            ofs << "cxxrtl::debug_items all_debug_items;\n";
            ofs << "top.debug_info(all_debug_items);\n";
            ofs << "cxxrtl::vcd_writer vcd;\n";
            ofs << "vcd.timescale(1, \"us\");\n";
            ofs << "vcd.add_without_memories(all_debug_items);\n";
            ofs << "std::ofstream waves(\"" + vcd_file_name + "\");\n";
            ofs << "vcd.sample(0);\n";
        }

        if (!continue_set)
            ofs << "top.step();\n";
        ofs << "for(int cycle=0;cycle<" << sim_cycle << ";++cycle){\n";
        ofs << "long long display_cycle = base_cycle + cycle + 1;\n";
        ofs << "top.p_" << clk_name << ".set<bool>(false);\n";
        ofs << "top.step();\n";

        // For VCD file.
        if (vcd_file_set) ofs << "vcd.sample(cycle*2 + 0);\n";

        if (!continue_set && (reset_set || reset_n_set)) {
            ofs << "if(cycle == 0)\n";
            if (reset_set) {
                ofs << "	top.p_" << reset_name << ".set<bool>(true);\n";
                ofs << "else\n";
                ofs << "	top.p_" << reset_name << ".set<bool>(false);\n";
            }
            if (reset_n_set) {
                ofs << "	top.p_" << reset_n_name << ".set<bool>(false);\n";
                ofs << "else\n";
                ofs << "	top.p_" << reset_n_name << ".set<bool>(true);\n";
            }
        }

        if (continue_set)
            ofs << "if(cycle >= 0)\n";
        else
            ofs << "if(cycle > 0)\n";
        ofs << "{\n";
        ofs << "size_t idx = 0;\n";
        for (auto wire : design->top_module()->wires()) {
            wire_name = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);

            // replace '_' with '+'
            while (wire_name.find("_") != -1) {
                wire_name.replace(wire_name.find("_"), 1, "+");
                // log("1111111111 wire_name = %s\n", wire_name.c_str());
            }

            // replace '+' with '__'
            while (wire_name.find("+") != -1) {
                wire_name.replace(wire_name.find("+"), 1, "__");
                // log("222222222222 wire_name = %s\n", wire_name.c_str());
            }

            // if(wire_name.find("_") != -1)
            // 	wire_name.replace(wire_name.find("_"), 1, "__");
            if (wire->port_input && strcmp(wire->name.c_str(), ("\\" + reset_n_name).c_str()) &&
                strcmp(wire->name.c_str(), ("\\" + reset_name).c_str()) &&
                strcmp(wire->name.c_str(), ("\\" + clk_name).c_str()))  // check the wire is not rst or clk
            {
                if (!stimulus)  // if no stimulus is given, we generate a random one
                {
                    // (1) If there's a large-bits wire (e.g. 256-bits memory) that exceed "unsigned range (0 ~ 65535)",
                    //     please slice it to smaller wires, otherwise will cause error in Yosys
                    // (2) Also, if assigns a wrong size value to wires (e.g. assign "pow(2, 2)" to "256-bits memory"), it will violate Yosys assertion
                    // TODO: preprocess the large-bits wire with ".slice()" and ".concat()" to assign value
                    if (wire->width > 16) {
                        log_error("There's a large-bits wire that exceed \"unsigned range (0 ~ 65535)\", please slice it to smaller wires all within 16 bits\n");
                    } else {
                        // randomly assign value
                        ofs << "upper_bound = pow(2, " << wire->width << ");\n";
                        ofs << "random_value = rand() % upper_bound;\n";
                        ofs << "top.p_" << wire_name << ".set<unsigned>(random_value)"
                            << ";\n";
                    }
                } else {
                    ofs << "top.p_" << wire_name << ".set<unsigned>(stimulus_signal[cycle][idx])"
                        << ";\n";
                    ofs << "++idx;\n";
                }

                // log("str len = %d\n", wire_name);
            }
        }
        ofs << "}\n";

        ofs << "top.p_" << clk_name << ".set<bool>(true);\n";
        ofs << "top.step();\n";
        // For VCD file.
        if (vcd_file_set) ofs << "vcd.sample(cycle*2 + 1);\n";
        for (auto wire : design->top_module()->wires()) {
            wire_name = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);
            string wire_name_long = wire_name;
            // replace '_' with '+'
            while (wire_name_long.find("_") != -1) {
                wire_name_long.replace(wire_name_long.find("_"), 1, "+");
            }

            // replace '+' with '__'
            while (wire_name_long.find("+") != -1) {
                wire_name_long.replace(wire_name_long.find("+"), 1, "__");
            }

            // char *wire_name;
            if (wire->port_output) {
                // strncpy(wire_name, &wire->name.c_str()[1], strlen(wire->name.c_str()) - 1);
                // log("width = %d", wire->width);
                // log("output name = %s\n", wire->name.str().substr(1,strlen(wire->name.c_str()) - 1));
                ofs << "uint32_t " << wire_name << "  = top.p_" << wire_name_long << ".get<uint32_t>();"
                    << "\n";
                // log("str len = %d\n", wire_name);
            }
        }
        // For VCD file.
        if (vcd_file_set) {
            ofs << "waves << vcd.buffer;\n";
            ofs << "vcd.buffer.clear();\n";
        }
        if (verbose) {
            ofs << "cout << \"==========================================\\n\";\n";
            ofs << "cout << \"= cycle \""
                << " << display_cycle "
                << "<< \"\\n\";\n";
            ofs << "cout << \"==========================================\\n\";\n";
            for (auto wire : design->top_module()->wires()) {
                std::string wire_name = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);
                // char *wire_name;
                if (wire->port_output) {
                    ofs << "cout << \"" << wire_name << "= \""
                        << " << " << wire_name << " << \"\\n\""
                        << ";\n";
                }
            }
            ofs << "cout << endl;";
        }

        size_t idx = 0;
        if (property_set) {
            for (auto wire : design->top_module()->wires()) {
                if (wire->port_output) {
                    if (idx == property) {
                        std::string wire_name = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);
                        ofs << "if(" + wire_name + " == 1)\n";
                        ofs << "{\n";
                        ofs << "cout << \"property unsafe!!!\" << endl;\n";
                        ofs << "break;\n";
                        ofs << "}\n";
                    }
                    idx++;
                }
            }
        }

        if (output_file_set) {
            ofs << "ofs << \"==========================================\\n\";\n";
            ofs << "ofs << \"= cycle \""
                << " << display_cycle "
                << "<< \"\\n\";\n";
            ofs << "ofs << \"==========================================\\n\";\n";
            for (auto wire : design->top_module()->wires()) {
                std::string wire_name = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);
                // char *wire_name;
                if (wire->port_output) {
                    ofs << "ofs << \"" << wire_name << "= \""
                        << " << " << wire_name << " << \"\\n\""
                        << ";\n";
                }
            }
        }

        ofs << "}\n";
        ofs << "std::ofstream state_ofs(state_file);\n";
        ofs << "state_ofs << \"__cycle__ \" << (base_cycle + " << sim_cycle << ") << \"\\n\";\n";
        ofs << "for (auto &entry : state_items.table) {\n";
        ofs << "    for (size_t part = 0; part < entry.second.size(); ++part) {\n";
        ofs << "        auto &item = entry.second[part];\n";
        ofs << "        if (item.depth != 1 || item.curr == nullptr || item.next == nullptr) continue;\n";
        ofs << "        size_t chunks = (item.width + 31) / 32;\n";
        ofs << "        state_ofs << entry.first << \"#\" << part << \" \" << item.width << \" \" << chunks;\n";
        ofs << "        for (size_t c = 0; c < chunks; ++c)\n";
        ofs << "            state_ofs << \" \" << std::hex << item.curr[c] << std::dec;\n";
        ofs << "        state_ofs << \"\\n\";\n";
        ofs << "    }\n";
        ofs << "}\n";
        ofs << "state_ofs.close();\n";
        if (output_file_set) ofs << "ofs.close();\n";
        ofs << "}\n";
        ofs << "\n";
        ofs.close();

        // Derive absolute path to the bundled yosys-config based on GV_YOSYS_BIN_PATH.
        std::string yosysBin   = GV_YOSYS_BIN_PATH;
        std::string yosysConfig = "yosys-config";
        size_t slashPos        = yosysBin.rfind('/');
        if (slashPos != std::string::npos)
            yosysConfig = yosysBin.substr(0, slashPos) + "/yosys-config";

        // Must use the same toolchain as the GV build: bare "g++" may be Homebrew GCC without
        // libc++ paths, while the project is often built with Apple or Homebrew Clang.
        std::string compileCmd = "\"";
        compileCmd += GV_RANDOM_SIM_CXX;
        compileCmd += "\" ";
        compileCmd += GV_RANDOM_SIM_CXXFLAGS;
        compileCmd += " -g -O3 -std=c++14 ";
#ifdef __APPLE__
        compileCmd += "-stdlib=libc++ ";
#endif
        compileCmd += "-I `" + yosysConfig + " --datdir`/include "
                      "-w "
                      ".sim_main.cpp -o .tb ";

        int compileRet = run_command(compileCmd);
        if (compileRet != 0)
            log_error("random_sim failed: could not compile generated simulator (.sim_main.cpp).\n");
        int simRet = run_command(" ./.tb ");
        if (simRet != 0)
            log_error("random_sim failed: simulator process exited abnormally (./.tb).\n");
    }
} randomSim;

PRIVATE_NAMESPACE_END
