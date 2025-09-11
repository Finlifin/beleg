#include <gtest/gtest.h>
#include <sstream>
#include "diag/diag.hh"
#include "source_map/source_map.hh"

class DiagTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup source map with test content
        test_file_id = source_map.add_file("test.bl",
                                           "fn main() {\n"
                                           "    let x = "
                                           "undefined_variable;\n"
                                           "    print(x);\n"
                                           "}");
    }

    void TearDown() override {
        // Cleanup code for each test
    }

    SourceMap source_map;
    FileId test_file_id;
};

// Test basic diagnostic creation and emission
TEST_F(DiagTest, BasicDiagnostic) {
    std::ostringstream output;
    auto emitter = create_terminal_emitter(output, false, false, &source_map);

    auto ctxt    = DiagCtxt({}, &source_map);
    ctxt.add_emitter(std::move(emitter));

    // Create a diagnostic for undefined variable
    Span error_span(35, 53); // "undefined_variable"

    ctxt.diag_builder(DiagLevel::Error, "undefined variable", error_span)
        .code(4002)
        .label(error_span, "not found in this scope")
        .note("perhaps you meant to import this variable?")
        .emit();

    std::string output_str = output.str();
    EXPECT_FALSE(output_str.empty());
    EXPECT_NE(output_str.find("Error"), std::string::npos);
    EXPECT_NE(output_str.find("4002"), std::string::npos);
    EXPECT_NE(output_str.find("undefined variable"), std::string::npos);
    EXPECT_NE(output_str.find("note"), std::string::npos);
}

// Test colored output
TEST_F(DiagTest, ColoredOutput) {
    std::ostringstream output;
    auto emitter = create_terminal_emitter(output, true, false, &source_map);

    auto ctxt    = DiagCtxt({}, &source_map);
    ctxt.add_emitter(std::move(emitter));

    Span error_span(35, 53);

    ctxt.diag_builder(DiagLevel::Error, "test error", error_span).emit();

    std::string output_str = output.str();
    EXPECT_NE(output_str.find("\x1b[91m"),
              std::string::npos); // BRIGHT_RED
    EXPECT_NE(output_str.find("\x1b[0m"),
              std::string::npos); // RESET
}

// Test Unicode output
TEST_F(DiagTest, UnicodeOutput) {
    std::ostringstream output;
    auto emitter = create_terminal_emitter(output, false, true, &source_map);

    auto ctxt    = DiagCtxt({}, &source_map);
    ctxt.add_emitter(std::move(emitter));

    Span error_span(35, 53);

    ctxt.diag_builder(DiagLevel::Error, "test error", error_span)
        .label(error_span, "test label")
        .emit();

    std::string output_str = output.str();
    EXPECT_NE(output_str.find("╭"),
              std::string::npos); // Unicode box chars
    EXPECT_NE(output_str.find("│"), std::string::npos);
}

// Test multiple diagnostic levels
TEST_F(DiagTest, MultipleLevels) {
    std::ostringstream output;
    auto emitter = create_terminal_emitter(output, true, false, &source_map);

    auto ctxt    = DiagCtxt({}, &source_map);
    ctxt.add_emitter(std::move(emitter));

    Span span1(10, 15);
    Span span2(20, 25);

    // Error
    ctxt.diag_builder(DiagLevel::Error, "error message", span1).emit();

    // Warning
    ctxt.diag_builder(DiagLevel::Warning, "warning message", span2).emit();

    // Note
    ctxt.diag_builder(DiagLevel::Note, "note message", span1).emit();

    std::string output_str = output.str();
    EXPECT_NE(output_str.find("error"), std::string::npos);
    EXPECT_NE(output_str.find("warning"), std::string::npos);
    EXPECT_NE(output_str.find("note"), std::string::npos);

    // Check that different colors are used
    EXPECT_NE(output_str.find("\x1b[91m"),
              std::string::npos); // Error - BRIGHT_RED
    EXPECT_NE(output_str.find("\x1b[93m"),
              std::string::npos); // Warning - BRIGHT_YELLOW
    EXPECT_NE(output_str.find("\x1b[94m"),
              std::string::npos); // Note - BRIGHT_BLUE
}

// Test error/warning limits
TEST_F(DiagTest, ErrorLimits) {
    std::ostringstream output;
    auto emitter = create_terminal_emitter(output, false, false, &source_map);

    DiagCtxtOptions options;
    options.max_errors   = 2;
    options.max_warnings = 1;

    auto ctxt            = DiagCtxt(options, &source_map);
    ctxt.add_emitter(std::move(emitter));

    Span span(10, 15);

    // Emit 3 errors (only 2 should be processed)
    ctxt.diag_builder(DiagLevel::Error, "error 1", span).emit();
    ctxt.diag_builder(DiagLevel::Error, "error 2", span).emit();
    ctxt.diag_builder(DiagLevel::Error, "error 3", span).emit();

    EXPECT_EQ(ctxt.error_count(), 2u);

    // Emit 2 warnings (only 1 should be processed)
    ctxt.diag_builder(DiagLevel::Warning, "warning 1", span).emit();
    ctxt.diag_builder(DiagLevel::Warning, "warning 2", span).emit();

    EXPECT_EQ(ctxt.warning_count(), 1u);
}

TEST_F(DiagTest, AnotherTestCase) {
    // TODO: Add more specific diag tests
    EXPECT_EQ(1, 1); // Placeholder test
}

// Main function is provided by gtest_main_dep, so no need
// to define it
