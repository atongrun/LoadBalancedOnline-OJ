#include "compiler.hpp"
#include "runner.hpp"
using namespace ns_runner; 
using namespace ns_compiler;

int main()
{
    std::string code = "code";
    Compiler::Compile(code);
    Runner::Run(code, 5, 102400);
    return 0;
}
