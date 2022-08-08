#include <kernel/yosys.h>
#include "kernel/consteval.h"
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct randomSim : public Pass
{
	randomSim() : Pass("randomSim", "") { }
    // void help() override
	// {
	// 	//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
	// 	log("\n");
	// 	log("    randomSim\n");
    // }
	void execute(std::vector<std::string>, Design *design) override
	{
        // Yosys::run_pass("read_verilog vending.v");
        // Yosys::run_pass("write_cxxrtl vending.cpp");
        // run_command(" clang++ -g -O3 -std=c++14 -I `yosys-config --datdir`/include main.cpp -o tb ");
        // run_command(" ./tb ");
		// Module *module = design->top_module();

		// if (module == nullptr)
		// 	log_error("No top module found!\n");

		// Wire *wire_a = module->wire(ID::A);
		// Wire *wire_y = module->wire(ID::Y);

		// if (wire_a == nullptr)
		// 	log_error("No wire A found!\n");

		// if (wire_y == nullptr)
		// 	log_error("No wire Y found!\n");

		// ConstEval ce(module);
		// for (int v = 0; v < 4; v++) {
		// 	ce.push();
		// 	ce.set(wire_a, Const(v, GetSize(wire_a)));
		// 	SigSpec sig_y = wire_y, sig_undef;
		// 	if (ce.eval(sig_y, sig_undef))
		// 		log("Eval results for A=%d: Y=%s\n", v, log_signal(sig_y));
		// 	else
		// 		log("Eval failed for A=%d: Missing value for %s\n", v, log_signal(sig_undef));
		// 	ce.pop();
		// }
	}
} randomSim;

PRIVATE_NAMESPACE_END
