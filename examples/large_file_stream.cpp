#include <iostream>
#include <csv_stream/csv_stream.hpp>

int main()
{
  csv_stream::Reader reader("large_dataset.csv");

  csv_stream::Row row;

  std::size_t count = 0;

  while (reader.read_row(row))
  {
    ++count;

    if (count % 100000 == 0)
      std::cout << "Processed rows: " << count << std::endl;
  }

  std::cout << "Total rows: " << count << std::endl;

  return 0;
}
