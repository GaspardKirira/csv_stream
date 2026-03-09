#include <iostream>
#include <csv_stream/csv_stream.hpp>

int main()
{
  csv_stream::Options opt;
  opt.delimiter = ';';

  csv_stream::Reader reader("data_semicolon.csv", opt);

  csv_stream::Row row;

  while (reader.read_row(row))
  {
    for (const auto &field : row.fields)
      std::cout << field << " ";

    std::cout << std::endl;
  }

  return 0;
}
