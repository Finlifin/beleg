#include "diag/diag.hh"
#include "source_map/source_map.hh"
#include <iostream>

int main() {
    std::cout << "=== Diagnostic System Demo ===" << std::endl;

    // Create a source map with some test code
    SourceMap source_map;
    source_map.add_file("example.bl",
                        "fn calculate(a, b) {\n"
                        "    let result = a + undefined_var;\n"
                        "    return result\n"
                        "}\n"
                        "\n"
                        "fn main() {\n"
                        "    let value = calculate(10, 20);\n"
                        "    print(value);\n"
                        "}");

    std::cout << "\n1. Basic Error Diagnostic:" << std::endl;
    {
        auto emitter = create_terminal_emitter(std::cout, true, true, &source_map);
        auto ctxt = DiagCtxt(DiagCtxtOptions{}, &source_map);
        ctxt.add_emitter(std::move(emitter));

        Span error_span(33, 46);
        ctxt.diag_builder(DiagLevel::Error, "undefined variable", error_span)
            .code(4002)
            .label(error_span, "not found in this scope")
            .note("perhaps you meant to declare this variable?")
            .emit();
    }

    std::cout << "\nDemo completed successfully!" << std::endl;
    return 0;
}