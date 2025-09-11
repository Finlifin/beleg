#include "source_map/source_map.hh"
#include <iostream>

int main() {
    std::cout << "=== Load File Feature Test ===" << std::endl;

    SourceMap source_map;

    // Test 1: Load multiple files from filesystem
    std::cout << "\n1. Loading multiple files:" << std::endl;

    auto file1 = source_map.load_file("test_source.txt");
    auto file2 = source_map.load_file("fibonacci.bl");

    if (file1 && file2) {
        std::cout << "✓ Loaded test_source.txt as FileId: " << file1->id
                  << std::endl;
        std::cout << "✓ Loaded fibonacci.bl as FileId: " << file2->id
                  << std::endl;

        // Test 2: Verify file content access
        std::cout << "\n2. File content access:" << std::endl;
        const auto* f1 = source_map.get_file(*file1);
        const auto* f2 = source_map.get_file(*file2);

        if (f1 && f2) {
            std::cout << "✓ test_source.txt: " << f1->content.size() << " bytes"
                      << std::endl;
            std::cout << "✓ fibonacci.bl: " << f2->content.size() << " bytes"
                      << std::endl;
        }

        // Test 3: Location operations on loaded files
        std::cout << "\n3. Location operations:" << std::endl;

        // Find the word "fibonacci" in the loaded file
        auto fib_pos = f2->content.find("fibonacci");
        if (fib_pos != std::string::npos) {
            auto global_pos = source_map.lookup_byte_pos(
                Location(*file2, 1, static_cast<u32>(fib_pos)));
            if (global_pos) {
                auto loc = source_map.lookup_location(*global_pos);
                if (loc) {
                    std::cout << "✓ Found 'fibonacci' at: "
                              << source_map.format_location(*loc) << std::endl;
                }
            }
        }

        // Test 4: Span operations
        std::cout << "\n4. Span operations on loaded files:" << std::endl;

        // Create a span covering the function name
        u32 func_start = static_cast<u32>(f2->content.find("fn ")) + 3;
        u32 func_end   = static_cast<u32>(f2->content.find("("));

        auto start_pos
            = source_map.lookup_byte_pos(Location(*file2, 1, func_start));
        auto end_pos
            = source_map.lookup_byte_pos(Location(*file2, 1, func_end));

        if (start_pos && end_pos) {
            Span func_name_span(*start_pos, *end_pos);
            auto span_text = source_map.get_span_text(func_name_span);
            if (span_text) {
                std::cout << "✓ Function name: '" << *span_text << "'"
                          << std::endl;
            }
        }

        // Test 5: File ID mapping
        std::cout << "\n5. File ID mapping:" << std::endl;

        auto found_id1 = source_map.get_file_id("test_source.txt");
        auto found_id2 = source_map.get_file_id("fibonacci.bl");

        if (found_id1 && found_id2 && *found_id1 == *file1
            && *found_id2 == *file2) {
            std::cout << "✓ File ID mapping works correctly" << std::endl;
        }

    } else {
        std::cout << "✗ Failed to load one or more files" << std::endl;
        return 1;
    }

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
