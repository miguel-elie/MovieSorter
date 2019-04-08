#include "data_type.h"
#include "compatibility.hpp"
#include <cassert>
#include <charconv>

/** @file
 *  @brief Provides numeric parsing functionality
 */

namespace csv {
    namespace internals {
        #ifndef DOXYGEN_SHOULD_SKIP_THIS
        std::string type_name(const DataType& dtype) {
            switch (dtype) {
            case CSV_STRING:
                return "string";
            case CSV_INT:
                return "int";
            case CSV_LONG_INT:
                return "long int";
            case CSV_LONG_LONG_INT:
                return "long long int";
            case CSV_DOUBLE:
                return "double";
            default:
                return "null";
            }
        };
        #endif

        constexpr long double _INT_MAX = (long double)std::numeric_limits<int>::max();
        constexpr long double _LONG_MAX = (long double)std::numeric_limits<long int>::max();
        constexpr long double _LONG_LONG_MAX = (long double)std::numeric_limits<long long int>::max();

        DataType data_type(csv::string_view in, long double* const out) {
            /** Distinguishes numeric from other text values. Used by various
             *  type casting functions, like csv_parser::CSVReader::read_row()
             *
             *  #### Rules
             *   - Leading and trailing whitespace ("padding") ignored
             *   - A string of just whitespace is NULL
             *
             *  @param[in] in String value to be examined
             */

            // Empty string --> NULL
            if (in.size() == 0)
                return CSV_NULL;

            bool ws_allowed = true,
                neg_allowed = true,
                dot_allowed = true,
                digit_allowed = true,
                has_digit = false,
                prob_float = false;

            unsigned places_after_decimal = 0;
            long double integral_part = 0,
                decimal_part = 0;

            for (size_t i = 0, ilen = in.size(); i < ilen; i++) {
                const char& current = in[i];

                switch (current) {
                case ' ':
                    if (!ws_allowed) {
                        if (isdigit(in[i - 1])) {
                            digit_allowed = false;
                            ws_allowed = true;
                        }
                        else {
                            // Ex: '510 123 4567'
                            return CSV_STRING;
                        }
                    }
                    break;
                case '-':
                    if (!neg_allowed) {
                        // Ex: '510-123-4567'
                        return CSV_STRING;
                    }

                    neg_allowed = false;
                    break;
                case '.':
                    if (!dot_allowed) {
                        return CSV_STRING;
                    }

                    dot_allowed = false;
                    prob_float = true;
                    break;
                case 'e':
                case 'E':
                    if (!prob_float) {
                        return CSV_STRING;
                    }
                    else if (isdigit(in[i + 1]) || in[i + 1] == '-' || in[i + 1] == '+') {
                        long double exponent;
                        string_view exponential_part = in;

                        if (in[i + 1] == '+') {
                            exponential_part.remove_prefix(i + 2);
                        }
                        else {
                            // Don't strip out minus sign
                            exponential_part.remove_prefix(i + 1);
                        }
                        
                        auto result = data_type(exponential_part, &exponent);

                        if (result >= CSV_INT || result <= CSV_DOUBLE) {
                            long double number = integral_part + decimal_part * pow(10, -(double)places_after_decimal);
                            number *= pow(10, exponent);
                            
                            if (out) *out = neg_allowed ? number : -number;

                            return CSV_DOUBLE;
                        }
                    }
                    break;
                default:
                    if (isdigit(current)) {
                        // Process digit
                        has_digit = true;

                        if (!digit_allowed)
                            return CSV_STRING;
                        else if (ws_allowed) // Ex: '510 456'
                            ws_allowed = false;

                        // Build current number
                        unsigned digit = current - '0';
                        if (prob_float) {
                            places_after_decimal++;
                            decimal_part = (decimal_part * 10) + digit;
                        }
                        else {
                            integral_part = (integral_part * 10) + digit;
                        }
                    }
                    else {
                        return CSV_STRING;
                    }
                }
            }

            // No non-numeric/non-whitespace characters found
            if (has_digit) {
                long double number = integral_part + decimal_part * pow(10, -(double)places_after_decimal);
                if (out) *out = neg_allowed ? number : -number;

                if (prob_float)
                    return CSV_DOUBLE;

                // We can assume number is always non-negative
                assert(out >= 0);

                if (number < _INT_MAX)
                    return CSV_INT;
                else if (number < _LONG_MAX)
                    return CSV_LONG_INT;
                else if (number < _LONG_LONG_MAX)
                    return CSV_LONG_LONG_INT;
                else // Conversion to long long will cause an overflow
                    return CSV_DOUBLE;
            }

            // Just whitespace
            return CSV_NULL;
        }

    }
}