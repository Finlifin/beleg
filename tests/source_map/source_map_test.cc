#include <gtest/gtest.h>
#include "source_map/source_map.hh"

class SourceMapTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup code for each test
    }

    void TearDown() override {
        // Cleanup code for each test
    }
};

// Test FileId functionality
TEST_F(SourceMapTest, FileIdComparison) {
    FileId id1(1);
    FileId id2(1);
    FileId id3(2);

    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
    EXPECT_LT(id1, id3);
}

// Test Span functionality
TEST_F(SourceMapTest, SpanOperations) {
    Span span(10, 20);

    EXPECT_TRUE(span.is_valid());
    EXPECT_EQ(span.len(), 10u);
    EXPECT_TRUE(span.contains(15));
    EXPECT_FALSE(span.contains(5));
    EXPECT_FALSE(span.contains(25));

    Span invalid_span(20, 10);
    EXPECT_FALSE(invalid_span.is_valid());
}

// Test SourceFile basic functionality
TEST_F(SourceMapTest, SourceFileBasic) {
    std::string content = "line 1\nline 2\nline 3";
    SourceFile file("test.txt", content, 0);
    FileId file_id(0);

    // Test line starts computation
    auto line1 = file.get_line(1);
    ASSERT_TRUE(line1.has_value());
    EXPECT_EQ(*line1, "line 1");

    auto line2 = file.get_line(2);
    ASSERT_TRUE(line2.has_value());
    EXPECT_EQ(*line2, "line 2");

    auto line3 = file.get_line(3);
    ASSERT_TRUE(line3.has_value());
    EXPECT_EQ(*line3, "line 3");

    // Test invalid line
    auto invalid_line = file.get_line(4);
    EXPECT_FALSE(invalid_line.has_value());
}

// Test byte position to location conversion
TEST_F(SourceMapTest, BytePosToLocation) {
    std::string content = "hello\nworld\ntest";
    SourceFile file("test.txt", content, 0);
    FileId file_id(0);

    // Position 0 should be line 1, column 0
    Location loc0 = file.byte_pos_to_location(0, file_id);
    EXPECT_EQ(loc0.line, 1u);
    EXPECT_EQ(loc0.column, 0u);

    // Position 6 should be line 2, column 0 (after "hello\n")
    Location loc6 = file.byte_pos_to_location(6, file_id);
    EXPECT_EQ(loc6.line, 2u);
    EXPECT_EQ(loc6.column, 0u);

    // Position 12 should be line 3, column 0 (after "hello\nworld\n")
    Location loc12 = file.byte_pos_to_location(12, file_id);
    EXPECT_EQ(loc12.line, 3u);
    EXPECT_EQ(loc12.column, 0u);
}

// Test location to byte position conversion
TEST_F(SourceMapTest, LocationToBytePos) {
    std::string content = "hello\nworld\ntest";
    SourceFile file("test.txt", content, 0);

    // Line 1, column 0 should be byte position 0
    auto pos0 = file.location_to_byte_pos(1, 0);
    ASSERT_TRUE(pos0.has_value());
    EXPECT_EQ(*pos0, 0u);

    // Line 2, column 0 should be byte position 6
    auto pos6 = file.location_to_byte_pos(2, 0);
    ASSERT_TRUE(pos6.has_value());
    EXPECT_EQ(*pos6, 6u);

    // Invalid position
    auto invalid_pos = file.location_to_byte_pos(10, 0);
    EXPECT_FALSE(invalid_pos.has_value());
}

// Test SourceMap file management
TEST_F(SourceMapTest, SourceMapFileManagement) {
    SourceMap source_map;

    // Add first file
    FileId id1 = source_map.add_file("file1.txt", "content1");
    EXPECT_EQ(id1.id, 0u);

    // Add second file
    FileId id2 = source_map.add_file("file2.txt", "content2");
    EXPECT_EQ(id2.id, 1u);

    // Adding same file should return same ID
    FileId id1_again = source_map.add_file("file1.txt", "content1");
    EXPECT_EQ(id1, id1_again);

    // Test file retrieval
    const SourceFile* file1 = source_map.get_file(id1);
    ASSERT_NE(file1, nullptr);
    EXPECT_EQ(file1->name, "file1.txt");
    EXPECT_EQ(file1->content, "content1");

    // Test file ID lookup
    auto lookup_id = source_map.get_file_id("file2.txt");
    ASSERT_TRUE(lookup_id.has_value());
    EXPECT_EQ(*lookup_id, id2);
}

// Test global position lookup
TEST_F(SourceMapTest, GlobalPositionLookup) {
    SourceMap source_map;

    FileId id1 = source_map.add_file("file1.txt", "hello\nworld"); // 11 bytes: "hello\nworld"
    FileId id2 = source_map.add_file("file2.txt", "test\ncode");   // 9 bytes: "test\ncode"

    // Position in first file
    auto loc5 = source_map.lookup_location(5);
    ASSERT_TRUE(loc5.has_value());
    EXPECT_EQ(loc5->file, id1);
    EXPECT_EQ(loc5->line, 1u);
    EXPECT_EQ(loc5->column, 5u);

    // Position in second file (global position 11 + 4 = 15 = position 4 in second file)
    auto loc15 = source_map.lookup_location(15); // 11 + 4 = position 4 in "test\ncode"
    ASSERT_TRUE(loc15.has_value());
    EXPECT_EQ(loc15->file, id2);
    // Position 4 in "test\ncode" is the newline character, so line 1, column 4
    EXPECT_EQ(loc15->line, 1u);
    EXPECT_EQ(loc15->column, 4u);
} // Test span text extraction
TEST_F(SourceMapTest, SpanTextExtraction) {
    SourceMap source_map;

    source_map.add_file("test.txt", "hello world\ntest code");

    // Extract "world"
    Span span(6, 11);
    auto text = source_map.get_span_text(span);
    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(*text, "world");

    // Extract across lines - "world\ntest" (positions 6-16, not 6-17)
    Span multiline_span(6, 16);
    auto multiline_text = source_map.get_span_text(multiline_span);
    ASSERT_TRUE(multiline_text.has_value());
    EXPECT_EQ(*multiline_text, "world\ntest");
}

// Test location formatting
TEST_F(SourceMapTest, LocationFormatting) {
    SourceMap source_map;

    FileId id = source_map.add_file("example.txt", "line1\nline2");
    Location loc(id, 2, 3);

    std::string formatted = source_map.format_location(loc);
    EXPECT_EQ(formatted, "example.txt:2:4"); // Column is 1-based in display
}

// Test span creation from line/column
TEST_F(SourceMapTest, SpanCreation) {
    SourceMap source_map;

    FileId id = source_map.add_file("test.txt", "hello\nworld\ntest");

    // Create span from line 1 col 1 to line 1 col 5 (should cover "ello")
    Span span = source_map.make_span(id, 1, 1, 1, 5);

    auto text = source_map.get_span_text(span);
    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(*text, "ello");
}

// Test invalid span handling
TEST_F(SourceMapTest, InvalidSpanHandling) {
    SourceMap source_map;

    source_map.add_file("test.txt", "hello world");

    // Invalid span (start > end)
    Span invalid_span(10, 5);
    auto invalid_text = source_map.get_span_text(invalid_span);
    EXPECT_FALSE(invalid_text.has_value());

    // Out of bounds span
    Span out_of_bounds(5, 100);
    auto out_of_bounds_text = source_map.get_span_text(out_of_bounds);
    EXPECT_FALSE(out_of_bounds_text.has_value());
}

// Test format_span with invalid spans
TEST_F(SourceMapTest, FormatSpanHandling) {
    SourceMap source_map;

    source_map.add_file("test.txt", "hello world");

    // Valid span
    Span valid_span(0, 5);
    auto valid_format = source_map.format_span(valid_span);
    EXPECT_TRUE(valid_format.has_value());

    // Invalid span (out of range)
    Span invalid_span(100, 200);
    auto invalid_format = source_map.format_span(invalid_span);
    EXPECT_FALSE(invalid_format.has_value());
}

// Test load_file functionality
TEST_F(SourceMapTest, LoadFile) {
    SourceMap source_map;

    // Test loading existing file
    auto file_id = source_map.load_file("../tests/source_map/test_source.txt");
    ASSERT_TRUE(file_id.has_value());

    const auto* file = source_map.get_file(*file_id);
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(file->name, "../tests/source_map/test_source.txt");
    EXPECT_FALSE(file->content.empty());

    // Test that file_id_map works correctly - loading the same file should return same ID
    auto file_id2 = source_map.load_file("../tests/source_map/test_source.txt");
    ASSERT_TRUE(file_id2.has_value());
    EXPECT_EQ(*file_id, *file_id2);

    // Test loading non-existent file
    auto nonexistent = source_map.load_file("../tests/source_map/nonexistent.txt");
    EXPECT_FALSE(nonexistent.has_value());

    // Test that get_file_id works with loaded files
    auto found_id = source_map.get_file_id("../tests/source_map/test_source.txt");
    ASSERT_TRUE(found_id.has_value());
    EXPECT_EQ(*found_id, *file_id);
}
