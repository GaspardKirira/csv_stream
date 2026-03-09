#include <iostream>
#include <csv_stream/csv_stream.hpp>

int main()
{
  csv_stream::Row row;

  auto result = csv_stream::parse_row("alice,30,kampala", row);

  if (!result.ok)
  {
    std::cerr << "Error: " << result.error << std::endl;
    return 1;
  }

  for (const auto &field : row.fields)
    std::cout << field << std::endl;

  return 0;
}
