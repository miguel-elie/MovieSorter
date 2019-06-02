/** @file
 *  Defines the data type used for storing information about a CSV row
 */

#include <cassert>
#include <functional>
#include "csv_row.hpp"

namespace csv {
    /** Return a string view of the nth field
     *
     *  @complexity
     *  Constant
     *
     *  @throws
     *  std::runtime_error If n is out of bounds
     */
    csv::string_view CSVRow::get_string_view(size_t n) const {
        csv::string_view ret(this->row_str);
        size_t beg = 0,
            end = 0,
            r_size = this->size();

        if (n >= r_size)
            throw std::runtime_error("Index out of bounds.");

        if (r_size > 1) {
            if (n == 0) {
                end = this->split_at(0);
            }
            else if (r_size == 2) {
                beg = this->split_at(0);
            }
            else {
                beg = this->split_at(n - 1);
                if (n != r_size - 1) end = this->split_at(n);
            }
        }

        // Performance optimization
        if (end == 0) {
            end = row_str.size();
        }
        
        return ret.substr(
            beg,
            end - beg // Number of characters
        );
    }

    /** Return a CSVField object corrsponding to the nth value in the row.
     *
     *  @note This method performs bounds checking, and will throw an
     *        `std::runtime_error` if n is invalid.
     *
     *  @complexity
     *  Constant, by calling csv::CSVRow::get_csv::string_view()
     *
     */
    CSVField CSVRow::operator[](size_t n) const {
        return CSVField(this->get_string_view(n));
    }

    /** Retrieve a value by its associated column name. If the column
     *  specified can't be round, a runtime error is thrown.
     *
     *  @complexity
     *  Constant. This calls the other CSVRow::operator[]() after
     *  converting column names into indices using a hash table.
     *
     *  @param[in] col_name The column to look for
     */
    CSVField CSVRow::operator[](const std::string& col_name) const {
        auto & col_names = this->buffer->col_names;
        auto col_pos = col_names->col_pos.find(col_name);
        if (col_pos != col_names->col_pos.end())
            return this->operator[](col_pos->second);

        throw std::runtime_error("Can't find a column named " + col_name);
    }

    CSVRow::operator std::vector<std::string>() const {

        std::vector<std::string> ret;
        for (size_t i = 0; i < size(); i++)
            ret.push_back(std::string(this->get_string_view(i)));

        return ret;
    }

#pragma region CSVRow Iterator
    /** Return an iterator pointing to the first field. */
    CSVRow::iterator CSVRow::begin() const {
        return CSVRow::iterator(this, 0);
    }

    /** Return an iterator pointing to just after the end of the CSVRow.
     *
     *  @warning Attempting to dereference the end iterator results
     *           in dereferencing a null pointer.
     */
    CSVRow::iterator CSVRow::end() const {
        return CSVRow::iterator(this, (int)this->size());
    }

    CSVRow::reverse_iterator CSVRow::rbegin() const {
        return std::reverse_iterator<CSVRow::iterator>(this->end());
    }

    CSVRow::reverse_iterator CSVRow::rend() const {
        return std::reverse_iterator<CSVRow::iterator>(this->begin());
    }

    unsigned short CSVRow::split_at(size_t n) const
    {
        return this->buffer->split_buffer[this->start + n];
    }

    HEDLEY_NON_NULL(1)
    CSVRow::iterator::iterator(const CSVRow* _reader, int _i)
        : daddy(_reader), i(_i) {
        if (_i < (int)this->daddy->size())
            this->field = std::make_shared<CSVField>(
                this->daddy->operator[](_i));
        else
            this->field = nullptr;
    }

    CSVRow::iterator::reference CSVRow::iterator::operator*() const {
        return *(this->field.get());
    }

    CSVRow::iterator::pointer CSVRow::iterator::operator->() const {
        // Using CSVField * as pointer type causes segfaults in MSVC debug builds
        #ifdef _MSC_BUILD
        return this->field;
        #else
        return this->field.get();
        #endif
    }

    CSVRow::iterator& CSVRow::iterator::operator++() {
        // Pre-increment operator
        this->i++;
        if (this->i < (int)this->daddy->size())
            this->field = std::make_shared<CSVField>(
                this->daddy->operator[](i));
        else // Reached the end of row
            this->field = nullptr;
        return *this;
    }

    CSVRow::iterator CSVRow::iterator::operator++(int) {
        // Post-increment operator
        auto temp = *this;
        this->operator++();
        return temp;
    }

    CSVRow::iterator& CSVRow::iterator::operator--() {
        // Pre-decrement operator
        this->i--;
        this->field = std::make_shared<CSVField>(
            this->daddy->operator[](this->i));
        return *this;
    }

    CSVRow::iterator CSVRow::iterator::operator--(int) {
        // Post-decrement operator
        auto temp = *this;
        this->operator--();
        return temp;
    }
    
    CSVRow::iterator CSVRow::iterator::operator+(difference_type n) const {
        // Allows for iterator arithmetic
        return CSVRow::iterator(this->daddy, i + (int)n);
    }

    CSVRow::iterator CSVRow::iterator::operator-(difference_type n) const {
        // Allows for iterator arithmetic
        return CSVRow::iterator::operator+(-n);
    }

    /** Two iterators are equal if they point to the same field */
    bool CSVRow::iterator::operator==(const iterator& other) const {
        return this->i == other.i;
    }
#pragma endregion CSVRow Iterator
}