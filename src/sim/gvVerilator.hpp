#include <string>
#include <vector>

void verilogModeling(string testbed);
void genInterfaceFile();
void genAssertionFile(vector<string> assertion_vec);
vector<string> collectAssertion(string filename);
string getDesignInfo();
