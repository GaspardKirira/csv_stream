#include <cassert>
#include <sstream>
#include <string>

#include <csv_stream/csv_stream.hpp>

namespace
{
  void test_parse_simple_row()
  {
    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("name,age,city", row);

    assert(result.ok);
    assert(row.size() == 3);
    assert(row[0] == "name");
    assert(row[1] == "age");
    assert(row[2] == "city");
  }

  void test_parse_empty_fields()
  {
    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("alice,,kampala,", row);

    assert(result.ok);
    assert(row.size() == 4);
    assert(row[0] == "alice");
    assert(row[1].empty());
    assert(row[2] == "kampala");
    assert(row[3].empty());
  }

  void test_parse_quoted_fields()
  {
    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("\"alice, bob\",42,\"kampala\"", row);

    assert(result.ok);
    assert(row.size() == 3);
    assert(row[0] == "alice, bob");
    assert(row[1] == "42");
    assert(row[2] == "kampala");
  }

  void test_parse_escaped_quotes()
  {
    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("\"he said \"\"hello\"\"\",ok", row);

    assert(result.ok);
    assert(row.size() == 2);
    assert(row[0] == "he said \"hello\"");
    assert(row[1] == "ok");
  }

  void test_parse_custom_delimiter()
  {
    csv_stream::Options opt;
    opt.delimiter = ';';

    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("alice;30;uganda", row, opt);

    assert(result.ok);
    assert(row.size() == 3);
    assert(row[0] == "alice");
    assert(row[1] == "30");
    assert(row[2] == "uganda");
  }

  void test_parse_unmatched_quote()
  {
    csv_stream::Row row;
    const csv_stream::Result result =
        csv_stream::parse_row("\"alice,42", row);

    assert(!result.ok);
    assert(!result.error.empty());
  }

  void test_reader_without_header()
  {
    std::istringstream input(
        "alice,30,kampala\n"
        "bob,28,nairobi\n");

    csv_stream::Reader reader(input);

    csv_stream::Row row1;
    csv_stream::Row row2;

    const bool has_first = reader.read_row(row1);
    const bool has_second = reader.read_row(row2);
    const bool has_third = reader.read_row(row2);

    assert(has_first);
    assert(has_second);
    assert(!has_third);

    assert(row1.size() == 3);
    assert(row1[0] == "alice");
    assert(row1[1] == "30");
    assert(row1[2] == "kampala");

    assert(row2.size() == 3);
    assert(row2[0] == "bob");
    assert(row2[1] == "28");
    assert(row2[2] == "nairobi");

    assert(reader.rows_read() == 2);
  }

  void test_reader_with_header()
  {
    std::istringstream input(
        "name,age,city\n"
        "alice,30,kampala\n"
        "bob,28,nairobi\n");

    csv_stream::Options opt;
    opt.has_header = true;

    csv_stream::Reader reader(input, opt);

    assert(reader.has_header());
    assert(reader.header().size() == 3);
    assert(reader.header()[0] == "name");
    assert(reader.header()[1] == "age");
    assert(reader.header()[2] == "city");

    csv_stream::Row row;
    const bool has_row = reader.read_row(row);

    assert(has_row);
    assert(row.size() == 3);
    assert(row[0] == "alice");
    assert(row[1] == "30");
    assert(row[2] == "kampala");
  }

  void test_reader_skip_empty_lines()
  {
    std::istringstream input(
        "\n"
        "alice,30,kampala\n"
        "\n"
        "bob,28,nairobi\n"
        "\n");

    csv_stream::Options opt;
    opt.skip_empty_lines = true;

    csv_stream::Reader reader(input, opt);

    csv_stream::Row row1;
    csv_stream::Row row2;

    assert(reader.read_row(row1));
    assert(reader.read_row(row2));
    assert(!reader.read_row(row2));

    assert(row1[0] == "alice");
    assert(row2[0] == "bob");
    assert(reader.rows_read() == 2);
  }

  void test_reader_strict_column_count_success()
  {
    std::istringstream input(
        "name,age\n"
        "alice,30\n"
        "bob,28\n");

    csv_stream::Options opt;
    opt.has_header = true;
    opt.strict_column_count = true;

    csv_stream::Reader reader(input, opt);

    csv_stream::Row row;
    assert(reader.read_row(row));
    assert(row.size() == 2);

    assert(reader.read_row(row));
    assert(row.size() == 2);
  }

  void test_reader_try_read_row_error()
  {
    std::istringstream input(
        "name,age\n"
        "\"alice,30\n");

    csv_stream::Options opt;
    opt.has_header = true;

    csv_stream::Reader reader(input, opt);

    csv_stream::Row row;
    bool has_row = false;

    const csv_stream::Result result = reader.try_read_row(row, has_row);

    assert(!result.ok);
    assert(!has_row);
    assert(!result.error.empty());
  }

  void test_parse_row_or_throw()
  {
    const csv_stream::Row row =
        csv_stream::parse_row_or_throw("x,y,z");

    assert(row.size() == 3);
    assert(row[0] == "x");
    assert(row[1] == "y");
    assert(row[2] == "z");
  }

} // namespace

int main()
{
  test_parse_simple_row();
  test_parse_empty_fields();
  test_parse_quoted_fields();
  test_parse_escaped_quotes();
  test_parse_custom_delimiter();
  test_parse_unmatched_quote();
  test_reader_without_header();
  test_reader_with_header();
  test_reader_skip_empty_lines();
  test_reader_strict_column_count_success();
  test_reader_try_read_row_error();
  test_parse_row_or_throw();

  return 0;
}
