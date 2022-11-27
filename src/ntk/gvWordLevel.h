/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

// [[CITE]] Btor2 , BtorMC and Boolector 3.0
// Aina Niemetz, Mathias Preiner, C. Wolf, Armin Biere
// Computer Aided Verification - 30th International Conference, CAV 2018
// https://cs.stanford.edu/people/niemetz/publication/2018/niemetzpreinerwolfbiere-cav18/
#ifndef GV_WNTK_H
#define GV_WNTK_H


#include "kernel/rtlil.h"
#include "kernel/register.h"
#include "kernel/sigtools.h"
#include "kernel/celltypes.h"
#include "kernel/log.h"
#include "kernel/mem.h"
#include "kernel/ff.h"
extern "C"{
#include "boolector.h"
}
#include <string>
USING_YOSYS_NAMESPACE
int CallBtorWorker(std::ostream &f, RTLIL::Module *module, bool verbose, bool single_bad, bool cover_mode, bool print_internal_names, string info_filename);
string whiteBoxAllSignal(std::ofstream *&f, std::string filename,RTLIL::Design *design,string patch);
int  patchOutput(string file,string top,string patch);
int boolectorTest();
class wordWrapper{
	public:
	wordWrapper();
	private:
	
};



#endif