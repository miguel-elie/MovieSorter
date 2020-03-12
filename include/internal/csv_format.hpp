/** @file
 *  Defines an object used to store CSV format settings
 */

#pragma once
#include <stdexcept>
#include <string>
#include <vector>

#include "compatibility.hpp"

namespace csv {
    class CSVReader;

    /** Stores the inferred format of a CSV file. */
    struct CSVGuessResult {
        char delim;
        int header_row;
    };

    /** Stores information about how to parse a CSV file.
     *  Can be used to construct a csv::CSVReader. 
     */
    class CSVFormat {
    public:
        /** Settings for parsing a RFC 4180 CSV file */
        CSVFormat() = default;

        /** Sets the delimiter of the CSV file
         *
         *  @throws `std::runtime_error` thrown if trim, quote, or possible delimiting characters overlap
         */
        CSVFormat& delimiter(char delim);

        /** Sets a list of potential delimiters
         *  
         *  @throws `std::runtime_error` thrown if trim, quote, or possible delimiting characters overlap
         *  @param[in] delim An array of possible delimiters to try parsing the CSV with
         */
        CSVFormat& delimiter(const std::vector<char> & delim);

        /** Sets the whitespace characters to be trimmed
         *
         *  @throws `std::runtime_error` thrown if trim, quote, or possible delimiting characters overlap
         *  @param[in] ws An array of whitespace characters that should be trimmed
         */
        CSVFormat& trim(const std::vector<char> & ws);

        /** Sets the quote character
         *
         *  @throws `std::runtime_error` thrown if trim, quote, or possible delimiting characters overlap
         */
        CSVFormat& quote(char quote);

        /** Sets the column names.
         *
         *  @note Unsets any values set by header_row()
         */
        CSVFormat& column_names(const std::vector<std::string>& names);

        /** Sets the header row
         *
         *  @note Unsets any values set by column_names()
         */
        CSVFormat& header_row(int row);

        /** Tells the parser to throw an std::runtime_error if an
         *  invalid CSV sequence is found
         */
        CONSTEXPR CSVFormat& strict_parsing(bool is_strict = true) {
            this->strict = is_strict;
            return *this;
        }

        /** Tells the parser to detect and remove UTF-8 byte order marks */
        CONSTEXPR CSVFormat& detect_bom(bool detect = true) {
            this->unicode_detect = detect;
            return *this;
        }

        #ifndef DOXYGEN_SHOULD_SKIP_THIS
        char get_delim() const {
            // This error should never be received by end users.
            if (this->possible_delimiters.size() > 1) {
                throw std::runtime_error("There is more than one possible delimiter.");
            }

            return this->possible_delimiters.at(0);
        }

        std::vector<char> get_possible_delims() {
            return this->possible_delimiters;
        }

        CONSTEXPR int get_header() {
            return this->header;
        }
        #endif
        
        /** CSVFormat for guessing the delimiter */
        CSV_INLINE static CSVFormat guess_csv() {
            CSVFormat format;
            format.delimiter({ ',', '|', '\t', ';', '^' })
                .quote('"')
                .header_row(0)
                .detect_bom(true);

            return format;
        }

        /** CSVFormat for strict RFC 4180 parsing */
        CSV_INLINE static CSVFormat rfc4180_strict() {
            CSVFormat format;
            format.delimiter(',')
                .quote('"')
                .header_row(0)
                .detect_bom(true)
                .strict_parsing(true);

            return format;
        }

        bool guess_delim() {
            return this->possible_delimiters.size() > 1;
        }

        friend CSVReader;
        friend std::vector<std::string> get_col_names(csv::string_view, CSVFormat);
    private:
        /**< Throws an error if delimiters and trim characters overlap */
        void assert_no_char_overlap();

        /**< Set of possible delimiters */
        std::vector<char> possible_delimiters = { ',' };

        /**< Set of whitespace characters to trim */
        std::vector<char> trim_chars = {};

        /**< Quote character */
        char quote_char = '"';

        /**< Row number with columns (ignored if col_names is non-empty) */
        int header = 0;

        /**< Should be left empty unless file doesn't include header */
        std::vector<std::string> col_names = {};

        /**< RFC 4180 non-compliance -> throw an error */
        bool strict = false;

        /**< Detect and strip out Unicode byte order marks */
        bool unicode_detect = true;
    };
}