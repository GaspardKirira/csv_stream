/**
 * @file csv_stream.hpp
 * @brief Streaming CSV reader utilities for C++.
 *
 * `csv_stream` provides a small, dependency-free, header-only toolkit for
 * reading CSV files row by row without loading the entire file into memory.
 *
 * This library is intentionally simple and focuses on practical CSV streaming
 * for common use cases such as large data imports, log processing, analytics
 * pipelines, ETL jobs, reporting systems, and lightweight tabular parsing.
 *
 * Supported features:
 * - Row-by-row streaming from file paths or input streams
 * - Configurable delimiter
 * - Optional header row support
 * - Quoted field parsing
 * - Escaped quote handling (`""`)
 * - Empty field support
 * * Basic row validation
 *
 * Design goals:
 * - Header-only and dependency-free
 * - Deterministic behavior
 * - Minimal API surface
 * - Practical utility for open source and production usage
 * - Memory-efficient parsing for large files
 *
 * Non-goals:
 * - Full RFC 4180 compliance in every edge case
 * - Schema inference
 * - Type conversion engine
 * - CSV writing
 * - Spreadsheet semantics
 *
 * Notes:
 * - This library reads one row at a time and is suitable for large files.
 * - Multiline quoted fields are not supported in this minimal implementation.
 * - If you need advanced CSV dialect support, build it on top of this layer.
 *
 * Requirements: C++17+
 */

#ifndef CSV_STREAM_CSV_STREAM_HPP
#define CSV_STREAM_CSV_STREAM_HPP

#include <cstddef>
#include <fstream>
#include <istream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace csv_stream
{
  /**
   * @brief Result type used by non-throwing APIs.
   */
  struct Result
  {
    bool ok = false;
    std::string error;

    /**
     * @brief Create a successful result.
     */
    static Result success()
    {
      return Result{true, {}};
    }

    /**
     * @brief Create a failed result.
     */
    static Result failure(std::string message)
    {
      return Result{false, std::move(message)};
    }
  };

  /**
   * @brief Represents a parsed CSV row.
   */
  struct Row
  {
    std::vector<std::string> fields;

    /**
     * @brief Check whether the row is empty.
     */
    bool empty() const noexcept
    {
      return fields.empty();
    }

    /**
     * @brief Number of fields in the row.
     */
    std::size_t size() const noexcept
    {
      return fields.size();
    }

    /**
     * @brief Access a field by index.
     *
     * @throws std::out_of_range if index is invalid.
     */
    const std::string &at(std::size_t index) const
    {
      return fields.at(index);
    }

    /**
     * @brief Access a field by index without bounds checking.
     */
    const std::string &operator[](std::size_t index) const noexcept
    {
      return fields[index];
    }
  };

  /**
   * @brief Options controlling CSV parsing behavior.
   */
  struct Options
  {
    /**
     * @brief Field delimiter.
     */
    char delimiter = ',';

    /**
     * @brief Quote character used for quoted fields.
     */
    char quote = '"';

    /**
     * @brief Whether the first row is a header row.
     */
    bool has_header = false;

    /**
     * @brief Skip completely empty lines.
     */
    bool skip_empty_lines = true;

    /**
     * @brief Trim a trailing carriage return (`\r`) from lines.
     *
     * Useful for Windows-style line endings.
     */
    bool trim_crlf = true;

    /**
     * @brief Enforce the same number of fields for all rows.
     *
     * If enabled, rows with inconsistent field counts will fail.
     */
    bool strict_column_count = false;
  };

  namespace detail
  {
    inline void trim_trailing_cr(std::string &line, const Options &opt)
    {
      if (opt.trim_crlf && !line.empty() && line.back() == '\r')
        line.pop_back();
    }

    inline bool is_effectively_empty_line(const std::string &line) noexcept
    {
      return line.empty();
    }

    inline Result parse_line(std::string_view line,
                             const Options &opt,
                             Row &out_row)
    {
      out_row.fields.clear();

      std::string current;
      current.reserve(line.size());

      bool in_quotes = false;

      for (std::size_t i = 0; i < line.size(); ++i)
      {
        const char ch = line[i];

        if (ch == opt.quote)
        {
          if (in_quotes)
          {
            const bool has_next = (i + 1 < line.size());
            if (has_next && line[i + 1] == opt.quote)
            {
              current.push_back(opt.quote);
              ++i;
            }
            else
            {
              in_quotes = false;
            }
          }
          else
          {
            in_quotes = true;
          }
          continue;
        }

        if (ch == opt.delimiter && !in_quotes)
        {
          out_row.fields.push_back(std::move(current));
          current.clear();
          continue;
        }

        current.push_back(ch);
      }

      if (in_quotes)
      {
        return Result::failure("csv_stream: unmatched quote in CSV row");
      }

      out_row.fields.push_back(std::move(current));
      return Result::success();
    }

  } // namespace detail

  /**
   * @brief Streaming CSV reader.
   *
   * Supports reading from either a file path or an existing input stream.
   */
  class Reader
  {
  public:
    /**
     * @brief Construct a reader from an input stream.
     *
     * The caller retains ownership of the stream.
     */
    explicit Reader(std::istream &input, const Options &opt = {})
        : stream_(&input), options_(opt)
    {
      initialize();
    }

    /**
     * @brief Construct a reader from a file path.
     *
     * @throws std::runtime_error if the file cannot be opened.
     */
    explicit Reader(const std::string &file_path, const Options &opt = {})
        : owned_stream_(std::make_unique<std::ifstream>(file_path)), options_(opt)
    {
      if (!owned_stream_ || !(*owned_stream_))
        throw std::runtime_error("csv_stream: failed to open file: " + file_path);

      stream_ = owned_stream_.get();
      initialize();
    }

    Reader(const Reader &) = delete;
    Reader &operator=(const Reader &) = delete;

    Reader(Reader &&) noexcept = default;
    Reader &operator=(Reader &&) noexcept = default;

    /**
     * @brief Check whether a header row was loaded.
     */
    bool has_header() const noexcept
    {
      return has_header_row_;
    }

    /**
     * @brief Get the parsed header row.
     */
    const Row &header() const noexcept
    {
      return header_row_;
    }

    /**
     * @brief Current 1-based line number in the input.
     *
     * This includes the header line if present.
     */
    std::size_t line_number() const noexcept
    {
      return line_number_;
    }

    /**
     * @brief Number of data rows successfully read.
     *
     * This excludes the header row.
     */
    std::size_t rows_read() const noexcept
    {
      return rows_read_;
    }

    /**
     * @brief Read the next CSV row.
     *
     * @param out_row Destination row.
     * @return true if a row was read, false on end-of-file.
     * @throws std::runtime_error if parsing fails in strict mode cases.
     */
    bool read_row(Row &out_row)
    {
      ensure_ready();

      std::string line;

      while (std::getline(*stream_, line))
      {
        ++line_number_;
        detail::trim_trailing_cr(line, options_);

        if (options_.skip_empty_lines && detail::is_effectively_empty_line(line))
          continue;

        const Result parsed = detail::parse_line(line, options_, out_row);
        if (!parsed.ok)
        {
          throw std::runtime_error(parsed.error + " at line " + std::to_string(line_number_));
        }

        if (expected_columns_ == 0)
        {
          expected_columns_ = out_row.size();
        }
        else if (options_.strict_column_count && out_row.size() != expected_columns_)
        {
          throw std::runtime_error(
              "csv_stream: inconsistent column count at line " +
              std::to_string(line_number_) +
              " (expected " + std::to_string(expected_columns_) +
              ", got " + std::to_string(out_row.size()) + ")");
        }

        ++rows_read_;
        return true;
      }

      return false;
    }

    /**
     * @brief Read the next CSV row using a non-throwing API.
     *
     * @param out_row Destination row.
     * @param has_row Set to true if a row was read, false on end-of-file.
     * @return Result indicating success or failure.
     */
    Result try_read_row(Row &out_row, bool &has_row) noexcept
    {
      has_row = false;

      try
      {
        has_row = read_row(out_row);
        return Result::success();
      }
      catch (const std::exception &e)
      {
        return Result::failure(e.what());
      }
    }

  private:
    void initialize()
    {
      if (options_.has_header)
      {
        Row temp;
        std::string line;

        while (std::getline(*stream_, line))
        {
          ++line_number_;
          detail::trim_trailing_cr(line, options_);

          if (options_.skip_empty_lines && detail::is_effectively_empty_line(line))
            continue;

          const Result parsed = detail::parse_line(line, options_, temp);
          if (!parsed.ok)
          {
            throw std::runtime_error(parsed.error + " at line " + std::to_string(line_number_));
          }

          header_row_ = std::move(temp);
          has_header_row_ = true;
          expected_columns_ = header_row_.size();
          return;
        }
      }
    }

    void ensure_ready() const
    {
      if (stream_ == nullptr)
        throw std::runtime_error("csv_stream: reader is not initialized");
    }

  private:
    std::unique_ptr<std::ifstream> owned_stream_;
    std::istream *stream_ = nullptr;
    Options options_{};

    Row header_row_;
    bool has_header_row_ = false;

    std::size_t line_number_ = 0;
    std::size_t rows_read_ = 0;
    std::size_t expected_columns_ = 0;
  };

  /**
   * @brief Parse a single CSV line into a row.
   *
   * This helper is useful for tests or custom stream pipelines.
   *
   * @param line CSV line.
   * @param out_row Destination row.
   * @param opt Parsing options.
   * @return Result indicating success or failure.
   */
  inline Result parse_row(std::string_view line, Row &out_row, const Options &opt = {})
  {
    return detail::parse_line(line, opt, out_row);
  }

  /**
   * @brief Parse a single CSV line and throw on error.
   *
   * @param line CSV line.
   * @param opt Parsing options.
   * @return Parsed row.
   * @throws std::runtime_error if parsing fails.
   */
  inline Row parse_row_or_throw(std::string_view line, const Options &opt = {})
  {
    Row row;
    const Result result = parse_row(line, row, opt);
    if (!result.ok)
      throw std::runtime_error(result.error);
    return row;
  }

} // namespace csv_stream

#endif // CSV_STREAM_CSV_STREAM_HPP
