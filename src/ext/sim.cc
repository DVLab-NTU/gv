#include <kernel/consteval.h>
#include <iostream>
#include <kernel/yosys.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
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
            log("    		Only one of reset and reset_n should be set\n");
            log("    [-output]\n");
            log("    		The name of output file which contains the simulation result\n");
            log("    [-v]\n");
            log("    		verbose print the result of simulation on the command line\n");
        }
        void execute(std::vector<std::string> args, Design* design) override {
            int         sim_cycle = 20, property = -1;
            size_t      argidx, num_inputs       = 0;
            bool        reset_set = false, property_set = false;
            bool        reset_n_set = false;
            bool        clk_set     = false;
            bool        verbose = false, verilog_file_name_set = false;
            bool        output_file_set = false, top_module_name_set = false, stimulus = false;
            bool        vcd_file_set     = false;
            std::string reset_name       = "reset";
            std::string reset_n_name     = "reset_n";
            std::string clk_name         = "clk";
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
                    reset_set  = true;
                    continue;
                }
                if (args[argidx] == "-reset_n" && argidx + 1 < args.size() && reset_set == false) {
                    reset_n_name = args[++argidx];
                    reset_n_set  = true;
                    continue;
                }
                if (args[argidx] == "-clk" && argidx + 1 < args.size()) {
                    clk_name = args[++argidx];
                    clk_set  = true;
                    continue;
                }
                if (args[argidx] == "-v") {
                    verbose = true;
                    continue;
                }
                if (args[argidx] == "-file" && argidx + 1 < args.size()) {
                    stimulus_file_name = args[++argidx];
                    stimulus           = true;
                    // log("innnnnnnnnnnnnn %s\n", stimulus_file_name.c_str());
                    continue;
                }
                if (args[argidx] == "-output" && argidx + 1 < args.size()) {
                    output_file_set  = true;
                    output_file_name = args[++argidx];
                    continue;
                }
                if (args[argidx] == "-top" && argidx + 1 < args.size()) {
                    top_module_name     = args[++argidx];
                    top_module_name_set = true;
                    continue;
                }
                if (args[argidx] == "-input" && argidx + 1 < args.size()) {
                    verilog_file_name     = args[++argidx];
                    verilog_file_name_set = true;
                    continue;
                }
                if (args[argidx] == "-vcd" && argidx + 1 < args.size()) {
                    vcd_file_name = args[++argidx];
                    vcd_file_set  = true;
                    continue;
                }
                if (args[argidx] == "-safe" && argidx + 1 < args.size()) {
                    property     = stoi(args[++argidx]);
                    property_set = true;
                    continue;
                }
                break;
            }
            for (auto wire : design->top_module()->wires()) {
                if (wire->port_input && strcmp(wire->name.c_str(), ("\\" + reset_n_name).c_str()) &&
                    strcmp(wire->name.c_str(), ("\\" + reset_name).c_str()) &&
                    strcmp(wire->name.c_str(), ("\\" + clk_name).c_str())) // check the wire is not rst or clk
                    ++num_inputs;
            }
            std::string command =
                "yosys -p \"read_verilog " + verilog_file_name + "; hierarchy -top " + top_module_name + "; write_cxxrtl .sim.cpp;\"";
            run_command(command);
            std::string   wire_name;
            std::string   module_name;
            std::ofstream ofs;
            ofs.open(".sim_main.cpp");
            ofs << "#include <iostream>\n";
            ofs << "#include <fstream>\n";
            ofs << "#include <stdlib.h>\n";
            ofs << "#include <time.h>\n";
            ofs << "#include <math.h>\n";
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
                ofs << "	std::ifstream ifs;\n";
                ofs << "	ifs.open(\"";
                ofs << stimulus_file_name;
                ofs << "\");\n";
                ofs << "	for(size_t i = 0; i < ";
                ofs << sim_cycle;
                ofs << "; ++i)\n";
                ofs << "	{\n";
                ofs << "		stimulus_cycle.clear();\n";
                ofs << "		for(size_t j = 0; j < ";
                ofs << num_inputs;
                ofs << "; ++j)\n";
                ofs << "		{\n";
                ofs << "			ifs >> buffer;\n";
                ofs << "			stimulus_cycle.push_back(std::stoi(std::string(buffer)));\n";
                ofs << "		}\n";
                ofs << "		stimulus_signal.push_back(stimulus_cycle);\n";
                ofs << "	}\n";
                ofs << "	ifs.close();\n";
            }
            if (output_file_set) {
                ofs << "ofstream ofs;\n";
                ofs << "ofs.open(\"" << output_file_name << "\");\n";
            }
            ofs << "     cxxrtl_design::p_" + module_name + " top;\n";
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

            ofs << "top.step();\n";
            ofs << "for(int cycle=0;cycle<" << sim_cycle << ";++cycle){\n";
            ofs << "top.p_" << clk_name << ".set<bool>(false);\n";
            ofs << "top.step();\n";

            // For VCD file.
            if (vcd_file_set) ofs << "vcd.sample(cycle*2 + 0);\n";

            if (reset_set || reset_n_set) {
                ofs << "if(cycle == 0)\n";
                if (reset_set) {
                    ofs << "	top.p_" << reset_name << ".set<bool>(true);\n";
                    ofs << "else\n";
                    ofs << "	top.p_" << reset_name << ".set<bool>(false);\n";
                }
                if (reset_n_set) {
                    ofs << "	top.p_" << reset_name << ".set<bool>(false);\n";
                    ofs << "else\n";
                    ofs << "	top.p_" << reset_name << ".set<bool>(true);\n";
                }
            }

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
                    strcmp(wire->name.c_str(), ("\\" + clk_name).c_str())) // check the wire is not rst or clk
                {
                    if (!stimulus) // if no stimulus is given, we generate a random one
                    {
                        // (1) If there's a large-bits wire (e.g. 256-bits memory) that exceed "unsigned range (0 ~ 65535)", 
                        //     please slice it to smaller wires, otherwise will cause error in Yosys 
                        // (2) Also, if assigns a wrong size value to wires (e.g. assign "pow(2, 2)" to "256-bits memory"), it will violate Yosys assertion
                        // TODO: preprocess the large-bits wire with ".slice()" and ".concat()" to assign value
                        if (wire->width > 16) {
                            log_error("There's a large-bits wire that exceed \"unsigned range (0 ~ 65535)\", please slice it to smaller wires all within 16 bits\n");
                        }
                        else {
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
                wire_name             = wire->name.str().substr(1, strlen(wire->name.c_str()) - 1);
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
                    << " << cycle + 1 "
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
                    << " << cycle + 1 "
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
            if (output_file_set) ofs << "ofs.close();\n";
            ofs << "}\n";
            ofs << "\n";
            ofs.close();
            run_command(" g++ -g -O3 -std=c++14 -I `yosys-config --datdir`/include .sim_main.cpp -o .tb ");
            run_command(" ./.tb ");
        }
} randomSim;

PRIVATE_NAMESPACE_END
