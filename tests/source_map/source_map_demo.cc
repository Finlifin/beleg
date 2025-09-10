#include "source_map/source_map.hh"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Source Map Module Functional Demo ===" << std::endl;

    // Create a source map
    SourceMap source_map;

    // Demo 1: Add multiple source files
    std::cout << "\n1. Adding Source Files Demo:" << std::endl;

    FileId main_file = source_map.add_file("main.bl",
                                           "fn main() {\n"
                                           "    let x = 42;\n"
                                           "    let y = \"hello\";\n"
                                           "    print(x + y);\n"
                                           "}");

    FileId lib_file = source_map.add_file("lib.bl",
                                          "fn add(a: int, b: int) -> int {\n"
                                          "    return a + b;\n"
                                          "}\n"
                                          "\n"
                                          "fn multiply(a: int, b: int) -> int {\n"
                                          "    return a * b;\n"
                                          "}");

    std::cout << "Added main.bl with FileId: " << main_file.id << std::endl;
    std::cout << "Added lib.bl with FileId: " << lib_file.id << std::endl;

    // Demo 2: Location lookup
    std::cout << "\n2. Location Lookup Demo:" << std::endl;

    // Look up some positions in the first file
    for (u32 pos : {0u, 10u, 25u, 50u}) {
        auto loc = source_map.lookup_location(pos);
        if (loc) {
            std::cout << "Global position " << pos << " -> " << source_map.format_location(*loc)
                      << std::endl;
        }
    }

    // Demo 3: Line content retrieval
    std::cout << "\n3. Line Content Retrieval Demo:" << std::endl;

    Location test_loc(main_file, 2, 0);
    auto line_content = source_map.get_line_at_location(test_loc);
    if (line_content) {
        std::cout << "Line 2 in main.bl: \"" << *line_content << "\"" << std::endl;
    }

    // Demo 4: Span creation and text extraction
    std::cout << "\n4. Span Operations Demo:" << std::endl;

    // Create a span covering "let x = 42" in main.bl (line 2, columns 4-14)
    Span variable_span = source_map.make_span(main_file, 2, 4, 2, 14);
    auto span_text = source_map.get_span_text(variable_span);

    if (span_text) {
        std::cout << "Span covering variable declaration: \"" << *span_text << "\"" << std::endl;
    } else {
        std::cout << "Failed to get span text" << std::endl;
    }

    auto span_location = source_map.format_span(variable_span);
    if (span_location) {
        std::cout << "Span location: " << *span_location << std::endl;
    } else {
        std::cout << "Failed to format span location" << std::endl;
    }

    // Demo 5: Error reporting simulation
    std::cout << "\n5. Error Reporting Simulation:" << std::endl;

    // Simulate an error at a specific location
    Location error_loc(main_file, 4, 10);
    std::cout << "Error at " << source_map.format_location(error_loc) << ":" << std::endl;

    auto error_line = source_map.get_line_at_location(error_loc);
    if (error_line) {
        std::cout << "    " << *error_line << std::endl;
        std::cout << "    ";
        for (u32 i = 0; i < error_loc.column; ++i) {
            std::cout << " ";
        }
        std::cout << "^" << std::endl;
        std::cout << "    undefined variable 'y'" << std::endl;
    }

    // Demo 6: Multi-file span
    std::cout << "\n6. Cross-File Operations Demo:" << std::endl;

    // Show file information
    const auto& files = source_map.get_files();
    std::cout << "Total files in source map: " << files.size() << std::endl;
    for (size_t i = 0; i < files.size(); ++i) {
        const auto& file = files[i];
        std::cout << "  File " << i << ": " << file.name << " (" << file.content.size() << " bytes)"
                  << std::endl;
    }

    // Demo 7: Span validation
    std::cout << "\n7. Span Validation Demo:" << std::endl;

    Span valid_span(10, 20);
    Span invalid_span(20, 10);

    std::cout << "Valid span (10, 20): valid=" << valid_span.is_valid()
              << ", length=" << valid_span.len() << std::endl;
    std::cout << "Invalid span (20, 10): valid=" << invalid_span.is_valid() << std::endl;

    // Demo 8: Load file from filesystem
    std::cout << "\n8. Load File From Filesystem Demo:" << std::endl;

    auto loaded_file = source_map.load_file("test_source.txt");
    if (loaded_file) {
        std::cout << "Successfully loaded test_source.txt with FileId: " << loaded_file->id
                  << std::endl;
        const auto* file = source_map.get_file(*loaded_file);
        if (file) {
            std::cout << "File content preview: "
                      << file->content.substr(0,
                                              std::min(50u, static_cast<u32>(file->content.size())))
                      << "..." << std::endl;
        }

        // Test loading the same file again (should return same ID)
        auto loaded_again = source_map.load_file("test_source.txt");
        if (loaded_again && *loaded_again == *loaded_file) {
            std::cout << "Loading same file again returned same FileId (caching works!)"
                      << std::endl;
        }
    } else {
        std::cout << "Failed to load test_source.txt (file not found)" << std::endl;
    }

    // Test loading non-existent file
    auto nonexistent = source_map.load_file("nonexistent.txt");
    if (!nonexistent) {
        std::cout << "Correctly failed to load nonexistent.txt" << std::endl;
    }

    std::cout << "\nSource Map demo completed successfully!" << std::endl;
    return 0;
}
