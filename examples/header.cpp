#include <iostream>
#include <csv_stream/csv_stream.hpp>

int main()
{
  csv_stream::Options opt;
  opt.has_header = true;

  csv_stream::Reader reader("users.csv", opt);

  if (reader.has_header())
  {
    std::cout << "Header: ";

    for (const auto &h : reader.header().fields)
      std::cout << h << " ";

    std::cout << std::endl;
  }

  csv_stream::Row row;

  while (reader.read_row(row))
  {
    std::cout << row[0] << " "
              << row[1] << " "
              << row[2] << std::endl;
  }

  return 0;
}
