#include "test_helper.hpp"
#include "markdown/parser.hpp"
#include "markdown/dom_builder.hpp"

#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>

using namespace markdown;

int main() {
    auto parser = make_cmark_parser();
    DomBuilder builder;

    // Test 1: Parser produces Table node with one header row and one body row
    {
        auto ast = parser->parse("| A | B |\n|---|---|\n| 1 | 2 |");
        ASSERT_EQ(ast.children.size(), 1u);
        auto& table = ast.children[0];
        ASSERT_EQ(table.type, NodeType::Table);

        // Find header and body rows
        int header_rows = 0;
        int body_rows = 0;
        for (auto const& row : table.children) {
            ASSERT_EQ(row.type, NodeType::TableRow);
            ASSERT_EQ(row.children.size(), 2u);
            for (auto const& cell : row.children) {
                ASSERT_EQ(cell.type, NodeType::TableCell);
            }
            if (row.level == 1) {
                ++header_rows;
            } else {
                ++body_rows;
            }
        }
        ASSERT_EQ(header_rows, 1);
        ASSERT_EQ(body_rows, 1);
    }

    // Test 2: DOM builder renders table with header and body cell texts
    {
        auto ast = parser->parse("| A | B |\n|---|---|\n| 1 | 2 |");
        auto element = builder.build(ast);
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(40),
                                            ftxui::Dimension::Fixed(3));
        ftxui::Render(screen, element);
        auto output = screen.ToString();
        ASSERT_CONTAINS(output, "A");
        ASSERT_CONTAINS(output, "B");
        ASSERT_CONTAINS(output, "1");
        ASSERT_CONTAINS(output, "2");
    }

    // Test 3: Multi-column table (3 columns) renders all column headers
    {
        auto ast = parser->parse(
            "| X | Y | Z |\n|---|---|---|\n| a | b | c |");
        auto element = builder.build(ast);
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(60),
                                            ftxui::Dimension::Fixed(3));
        ftxui::Render(screen, element);
        auto output = screen.ToString();
        ASSERT_CONTAINS(output, "X");
        ASSERT_CONTAINS(output, "Y");
        ASSERT_CONTAINS(output, "Z");
    }

    // Test 4: Table with only a header row (no body rows) does not crash
    {
        auto ast = parser->parse("| H1 | H2 |\n|---|---|");
        auto element = builder.build(ast);
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(40),
                                            ftxui::Dimension::Fixed(2));
        ftxui::Render(screen, element);
        auto output = screen.ToString();
        ASSERT_CONTAINS(output, "H1");
        ASSERT_CONTAINS(output, "H2");
    }

    // Test 5: Cell containing **bold** inline formatting renders without crash
    {
        auto ast = parser->parse(
            "| **Bold** | Normal |\n|---|---|\n| val | val2 |");
        auto element = builder.build(ast);
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(40),
                                            ftxui::Dimension::Fixed(3));
        ftxui::Render(screen, element);
        auto output = screen.ToString();
        ASSERT_CONTAINS(output, "Bold");
        ASSERT_CONTAINS(output, "Normal");
    }

    return 0;
}
