/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2021 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#ifndef __BITPIT_STRING_UTILS_TPP__
#define __BITPIT_STRING_UTILS_TPP__

namespace bitpit {

namespace utils {

namespace string {

/*!
* \ingroup common_strings
*
* Left-trim operator for std::string.
*
* Remove left trailing spaces from string. For instance, if the input string is
* "  test_string  ", on output this function returns "test_string  "
*
* \param[in,out] s on input contains the input string, on output contains the
* trimmed string
* \result A reference to the input string.
*/
inline std::string & ltrim(std::string &s)
{
    // The argument of the lambda function passed to the erase method is an
    // unsigned char, that's because, like all other functions from <cctype>,
    // the behavior of std::isspace is undefined if the argument's value is
    // neither representable as unsigned char nor equal to EOF. To use these
    // functions safely, they should not be directly used with standard
    // algorithms when the iterator's value type is char or signed char.
    // Instead, the value should be converted to unsigned char first (see
    // https://en.cppreference.com/w/cpp/string/byte/isspace#Notes).
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c){ return !std::isspace(c); }));

    return s;
}

/*!
* \ingroup common_strings
*
* Right-trim operator for std::string.
* Remove right blank spaces from string. For instance, if the input string is
* "  test_string  ", on output this function returns "  test_string"
*
* \param[in,out] s on input contains the input string, on output contains the
* trimmed string
* \result A reference to the input string.
*/
inline std::string &rtrim(std::string &s)
{
    // The argument of the lambda function passed to the erase method is an
    // unsigned char, that's because, like all other functions from <cctype>,
    // the behavior of std::isspace is undefined if the argument's value is
    // neither representable as unsigned char nor equal to EOF. To use these
    // functions safely, they should not be directly used with standard
    // algorithms when the iterator's value type is char or signed char.
    // Instead, the value should be converted to unsigned char first (see
    // https://en.cppreference.com/w/cpp/string/byte/isspace#Notes).
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c){ return !std::isspace(c); }).base(), s.end());

    return s;
}

/*!
* \ingroup common_strings
*
* Trim operator for std::string.
* Remove left/right blank spaces from string. For instance, if the input string is
* "  test_string  ", on output this function returns "test_string"
*
* \param[in,out] s on input contains the input string, on output contains the
* trimmed string
* \result A reference to the input string.
*/
inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}

/*!
* \ingroup common_strings
*
* String left-filler. Create a string composed of the input string left-filled
* with a specified character. E.g.
* given the input string s = "test", lfill(10, s, '_') will return
* "______test".
*
* \param[in] nchars is the final length of the string
* \param[in] s is the input string
* \param[in] c is the char used as filler
*/
inline std::string lfill(int nchars, const std::string &s, char c)
{
    std::stringstream ss;
    ss << std::string(nchars - s.length(), c) << s;

    return ss.str();
}

/*!
* \ingroup common_strings
*
* String right-filler. Create a string composed of the input string right-filled
* with a specified character. E.g.
* given the input string s = "test", rfill(10, s, '_') will return
* "test______".
*
* \param[in] nchars is the final length of the string
* \param[in] s is the input string
* \param[in] c is the char used as filler
*/
inline std::string rfill(int nchars, const std::string &s, char c)
{
    std::stringstream ss;
    ss << s << std::string(nchars - s.length(), c);

    return ss.str();
}

/*!
* \ingroup common_strings
*
* Given an integer, returns a string of length nchars, composed of the input number
* and nchars - ndigits '0' characters (where ndigits is the number of digits of the input integer)
* in the following format "000xxx".
* If ndigits > nchars, the output string will contaiens ndigits characters storing the
* digits of the input number.
* For instance, if nchars = 4 and num = 3, this function returns the string "0003".
* If nchars = 4, and num = 12345, this function returns "12345".
*
* \param[in] nchars is the final length of the string
* \param[in] num is the input integer
* \result A string storing the input number in the format 000xxx.
*/
inline std::string zeroPadNumber(int nchars, int num)
{
    std::ostringstream ss;
    ss << std::setw(nchars) << std::setfill('0') << num;

    return ss.str();
}

/*!
* \ingroup common_strings
*
* Check whether a string contains the kwyword or not.
*
* \param[in] line is the input string
* \param[in] key is the search key
* \result Return true if the keyword has been found, false otherwise.
*/
inline bool keywordInString(const std::string &line, const std::string &key)
{
    return (line.find(key) != std::string::npos);
}

/*!
* \ingroup common_strings
*
* Convertes a string into fundamental data type.
*
* If no data of type T can be extracted from the input string a 0 value,
* will be stored in output.
* If multiple values can be extracted from the input string, only the first
* value will be saved in output.
*
* \param[in] input is the input string
* \param[out] output on output contains the value extracted from the input
* string
*/
template <class T>
void convertString(const std::string &input, T &output)
{
    std::size_t nValues = 0;
    std::stringstream ss(input);
    ss >> std::ws;
    while (ss.good()) {
        ++nValues;
        if (nValues > 1) {
            std::cout << " more than one element in string " << input   << std::endl;
            std::cout << " assigning first element             "  << std::endl;
            return;
        }

        ss >> output;
        ss >> std::ws;
    }

    if (nValues == 0) {
        std::cout << " no useful information in string " << input   << std::endl;
        std::cout << " casting zero                   " <<  std::endl;

        output = T{};
    }
}

/*!
* \ingroup common_strings
*
* Convertes a string into a vector of fundamental data type.
*
* If no data of type T can be extracted from the input string an empty vector is returned.
* Values extracted from string are pushed at the end of the vector.
*
* \param[in] input is the input string
* \param[out] output on output contains the values extracted from the input
* string
*/
template <class T>
void convertString(const std::string &input, std::vector<T> &output)
{
    output.clear();

    std::stringstream ss(input);
    ss >> std::ws;
    while (ss.good()) {
        T x;
        ss >> x;
        ss >> std::ws;
        output.push_back(std::move(x));
    }

    if (output.empty()) {
        std::cout << " no useful information in string " << input   << std::endl;
        std::cout << " returning empty vector          " <<  std::endl;
    };
}

/*!
* \ingroup common_strings
*
* Convertes a string into a arrayof fundamental data type.
*
* If no data of type T can be extracted from the input string a void array with null
* elements is returned.
* If the number of elements which can be extracted from the input string is larger
* than the array size, only the first n elements are saved in the array.
*
* \param[in] input is the input string
* \param[out] output on output contains the values extracted from the input
* string
*/
template <class T, size_t n>
void convertString(const std::string &input, std::array<T,n> &output)
{
    std::size_t nValues = 0;
    std::stringstream ss(input);
    ss >> std::ws;
    while (ss.good()) {
        ++nValues;
        if (nValues > n) {
            std::cout << " more than " << n << " elements in string " << input   << std::endl;
            std::cout << " assigning first " << n << " elements "   << std::endl;
            return;
        }

        ss >> output[nValues - 1];
        ss >> std::ws;
    }

    if (nValues < n) {
        std::cout << " not enough useful information in string " << input   << std::endl;
        std::cout << " casting zero into missing elements      " <<  std::endl;

        for(size_t i=nValues; i<n; i++) {
            output[i] = T{};
        }
    }
}

}

}

}

#endif
