#pragma once
#include <stdexcept>
#include <string>
#include <vector>

namespace csv {
    class CSVReader;

    /** Stores information about how to parse a CSV file.
     *  Can be used to construct a csv::CSVReader. 
     */
    class CSVFormat {
    public:
        /** Settings for parsing a RFC 4180 CSV file */
        CSVFormat() = default;

        /** Sets the delimiter of the CSV file */
        CSVFormat& delimiter(char delim);

        /** Sets a list of pootential delimiters
         *  
         *  @param[in] delim An array of possible delimiters to try parsing the CSV with
         */
        CSVFormat& delimiter(const std::vector<char> & delim);

        /** Sets the quote character */
        CSVFormat& quote(char quote);

        /** Sets the column names */
        CSVFormat& column_names(const std::vector<std::string>& col_names);

        /** Sets the header row */
        CSVFormat& header_row(int row);

        /** Tells the parser to throw an std::runtime_error if an
         *  invalid CSV sequence is found
         */
        CSVFormat& strict_parsing(bool strict = true);

        /** Tells the parser to detect and remove UTF-8 byte order marks */
        CSVFormat& detect_bom(bool detect = true);

        #ifndef DOXYGEN_SHOULD_SKIP_THIS
        char get_delim() {
            // This error should never be received by end users.
            if (this->possible_delimiters.size() > 1) {
                throw std::runtime_error("There is more than one possible delimiter.");
            }

            return this->possible_delimiters.at(0);
        }

        int get_header() {
            return this->header;
        }
        #endif
        
        /** CSVFormat for guessing the delimiter */
        static const CSVFormat GUESS_CSV;

        /** CSVFormat for strict RFC 4180 parsing */
        static const CSVFormat RFC4180_STRICT;

        friend CSVReader;
    private:
        bool guess_delim() {
            return this->possible_delimiters.size() > 1;
        }

        /**< Set of possible delimiters */
        std::vector<char> possible_delimiters = { ',' };

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