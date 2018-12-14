/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>

#include <getopt.h>

#include <ossie/bitops.h>

class scoped_timer
{
public:
    explicit scoped_timer(std::ostream& stream=std::cout) :
        _stream(stream)
    {
        reset();
    }

    void reset()
    {
        clock_gettime(CLOCK_MONOTONIC, &_start);
    }

    double elapsed()
    {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (now.tv_sec - _start.tv_sec) + 1e-9*(now.tv_nsec - _start.tv_nsec);
    }

    ~scoped_timer()
    {
        _stream << ((uint64_t)(elapsed()*1e6)) << std::endl;
    }

private:
    std::ostream& _stream;
    struct timespec _start;
};

void test_getint(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 1, 5, 8, 11, 16, 20, 24, 32, 64 };

    unsigned char packed[9];
    for (size_t ii = 0; ii < sizeof(packed); ++ii) {
        packed[ii] = random();
    }

    stream << "bits,offset,time(usec)" << std::endl;
    uint64_t value = 0;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        for (size_t offset = 0; offset < 8; ++ offset) {
            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                value = redhawk::bitops::getint(packed, offset, bits);
            }
        }
    }
}

void test_setint(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 1, 5, 8, 11, 16, 20, 24, 32, 64 };

    unsigned char packed[9];
    std::fill(packed, packed + sizeof(packed), 0);

    uint64_t value = random();

    stream << "bits,offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        for (size_t offset = 0; offset < 8; ++ offset) {
            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                redhawk::bitops::setint(packed, offset, value, bits);
            }
        }
    }
}

void test_fill(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };
    std::vector<unsigned char> buffer;

    stream << "bits,offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        buffer.resize((bits + 7) / 8);
        for (size_t offset = 0; offset < 8; ++offset) {

            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                redhawk::bitops::fill(&buffer[0], offset, bits, 1);
            }
        }
    }
}

void test_pack(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };

    std::vector<unsigned char> unpacked;
    std::vector<unsigned char> packed;

    stream << "bits,offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        for (size_t offset = 0; offset < 8; ++offset) {
            // Generate random data for unpacked byte array
            unpacked.resize(bits);
            for (size_t index = 0; index < unpacked.size(); ++ index) {
                // Assume normal distribution of evens/odds
                unpacked[index] = random() & 1;
            }
            // Clear the bit array
            packed.resize((bits + 7) / 8);
            std::fill(packed.begin(), packed.end(), 0);

            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                redhawk::bitops::pack(&packed[0], offset, &unpacked[0], bits);
            }
        }
    }
}

void test_unpack(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };

    std::vector<unsigned char> packed;
    std::vector<unsigned char> unpacked;

    stream << "bits,offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        for (size_t offset = 0; offset < 8; ++offset) {
            // Generate random data for packed bit array
            packed.resize((bits + 7) / 8);
            for (size_t index = 0; index < packed.size(); ++ index) {
                // Assume normal distribution of evens/odds
                packed[index] = random();
            }
            // Clear the unpacked byte array
            unpacked.resize(bits);
            std::fill(unpacked.begin(), unpacked.end(), 0);

            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                redhawk::bitops::unpack(&unpacked[0], &packed[0], offset, bits);
            }
        }
    }
}

void test_popcount(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };

    std::vector<unsigned char> packed;

    stream << "bits,offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];
        for (size_t offset = 0; offset < 8; ++offset) {
            // Generate random bit data
            packed.resize((bits + 7) / 8);
            for (size_t index = 0; index < packed.size(); ++ index) {
                packed[index] = random();
            }

            stream << bits << "," << offset << ",";
            scoped_timer timer(stream);
            for (size_t jj = 0; jj < iterations; ++jj) {
                redhawk::bitops::popcount(&packed[0], offset, bits);
            }
        }
    }
}

void test_copy(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };

    std::vector<unsigned char> source;
    std::vector<unsigned char> dest;

    stream << "bits,src offset,dest offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];

        // Generate random bits for the source; round up to a full byte and add
        // enough excess to accomodate offset of up to a byte.
        source.resize((bits + 15) / 8);
        for (size_t index = 0; index < source.size(); ++ index) {
            source[index] = random();
        }

        // Allocate an equal amount of space for the destination
        dest.resize((bits + 15) / 8);
        std::fill(dest.begin(), dest.end(), 0);

        for (size_t src_offset = 0; src_offset < 8; ++src_offset) {
            for (size_t dest_offset = 0; dest_offset < 8; ++dest_offset) {
                
                stream << bits << "," << src_offset << "," << dest_offset << ",";
                scoped_timer timer(stream);
                for (size_t jj = 0; jj < iterations; ++jj) {
                    redhawk::bitops::copy(&dest[0], dest_offset, &source[0], src_offset, bits);
                }
            }
        }
    }
}

void test_compare(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };

    std::vector<unsigned char> lhs;
    std::vector<unsigned char> rhs;

    stream << "bits,left offset,right offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];

        // Generate random bits for the left hand side; round up to a full byte
        // and add enough excess to accomodate offset of up to a byte.
        lhs.resize((bits + 15) / 8);
        for (size_t index = 0; index < lhs.size(); ++ index) {
            lhs[index] = random();
        }
        rhs.resize(lhs.size());

        for (size_t lhs_offset = 0; lhs_offset < 8; ++lhs_offset) {
            for (size_t rhs_offset = 0; rhs_offset < 8; ++rhs_offset) {
                // Copy to the right hand side
                redhawk::bitops::copy(&rhs[0], rhs_offset, &lhs[0], lhs_offset, bits);
                
                stream << bits << ","
                       << lhs_offset << ","
                       << rhs_offset << ",";
                scoped_timer timer(stream);
                for (size_t jj = 0; jj < iterations; ++jj) {
                    if (redhawk::bitops::compare(&lhs[0], lhs_offset, &rhs[0], rhs_offset, bits)) {
                        std::cerr << "FAIL" << std::endl;
                    }
                }
            }
        }
    }
}

void test_compare_large(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 16, 64, 256, 512, 1024, 2048, 4096 };

    std::vector<unsigned char> lhs;
    std::vector<unsigned char> rhs;

    stream << "bits,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];

        lhs.resize((bits + 7) / 8);
        for (size_t index = 0; index < lhs.size(); ++ index) {
            lhs[index] = random();
        }
        rhs = lhs;

        stream << bits << ",";
        scoped_timer timer(stream);
        for (size_t jj = 0; jj < iterations; ++jj) {
            if (redhawk::bitops::compare(&lhs[0], 0, &rhs[0], 0, bits)) {
                std::cerr << "FAIL" << std::endl;
            }
        }
    }
}

void test_hamming_distance(std::ostream& stream, size_t iterations)
{
    size_t bit_counts[] = { 7, 8, 9, 14, 16, 18, 251, 256, 261, 1023, 1024 };
    const size_t max_bits = bit_counts[(sizeof(bit_counts)/sizeof(bit_counts[0])) - 1];

    // Generate random bits for both strings
    std::vector<unsigned char> lhs;
    lhs.resize((max_bits + 15) / 8);
    for (size_t index = 0; index < lhs.size(); ++ index) {
        lhs[index] = random();
    }

    std::vector<unsigned char> rhs;
    rhs.resize(lhs.size());
    for (size_t index = 0; index < rhs.size(); ++ index) {
        rhs[index] = random();
    }

    stream << "bits,left offset,right offset,time(usec)" << std::endl;
    for (size_t ii = 0; ii < (sizeof(bit_counts) / sizeof(bit_counts[0])); ++ii) {
        size_t bits = bit_counts[ii];

        for (size_t lhs_offset = 0; lhs_offset < 8; ++lhs_offset) {
            for (size_t rhs_offset = 0; rhs_offset < 8; ++rhs_offset) {
                stream << bits << ","
                       << lhs_offset << ","
                       << rhs_offset << ",";
                scoped_timer timer(stream);
                for (size_t jj = 0; jj < iterations; ++jj) {
                    redhawk::bitops::hammingDistance(&lhs[0], lhs_offset, &rhs[0], rhs_offset, bits);
                }
            }
        }
    }
}

#define ARRAY_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

void test_find(std::ostream& stream, size_t iterations)
{
    const size_t string_sizes[] = { 64, 251, 512, 1025 };
    const size_t pattern_sizes[] = { 7, 16, 35 };

    stream << "string size,pattern size,time(usec)" << std::endl;

    for (size_t ii = 0; ii < ARRAY_ELEMENTS(string_sizes); ++ii) {
        const size_t string_length = string_sizes[ii];
        std::vector<unsigned char> string;
        string.resize((string_length + 7) / 8);
        std::fill(string.begin(), string.end(), 0);

        for (size_t jj = 0; jj < ARRAY_ELEMENTS(pattern_sizes); ++jj) {
            const size_t pattern_length = pattern_sizes[jj];
            std::vector<unsigned char> pattern;
            pattern.resize((pattern_length + 7) / 8);
            std::fill(pattern.begin(), pattern.end(), 0xFF);

            stream << string_length << ','
                   << pattern_length << ',';
            scoped_timer timer(stream);
            for (size_t kk = 0; kk < iterations; ++kk) {
                redhawk::bitops::find(&string[0], 0, string_length, &pattern[0], 0, pattern_length, pattern_length / 2);
            }
        }
    }
}

typedef void (*benchmark_func)(std::ostream&,size_t);

void run_benchmark(const std::string& name, const std::string& suffix, benchmark_func func, size_t iterations)
{
    std::string filename = name;
    if (!suffix.empty()) {
        filename += "-" + suffix;
    }
    filename += ".csv";

    std::ofstream file(filename.c_str());
    std::cout << name << std::endl;
    func(file, iterations);
}

int main(int argc, char* argv[])
{
    size_t iterations = 100000;

    struct option long_options[] = {
        { "suffix", required_argument, 0, 0 },
        { 0, 0, 0, 0 }
    };

    typedef std::map<std::string,benchmark_func> FuncTable;
    FuncTable functions;
    functions["pack"] = &test_pack;
    functions["fill"] = &test_fill;
    functions["unpack"] = &test_unpack;
    functions["getint"] = &test_getint;
    functions["setint"] = &test_setint;
    functions["popcount"] = &test_popcount;
    functions["copy"] = &test_copy;
    functions["compare"] = &test_compare;
    functions["compare-aligned"] = &test_compare_large;
    functions["hamming"] = &test_hamming_distance;
    functions["find"] = &test_find;

    int option_index;
    std::string suffix;
    while (true) {
        int status = getopt_long(argc, argv, "", long_options, &option_index);
        if (status == '?') {
            // Invalid option
            return -1;
        } else if (status == 0) {
            if (option_index == 0) {
                suffix = optarg;
            }
        } else {
            // End of arguments
            break;
        }
    }

    if (optind < argc) {
        for (int arg = optind; arg < argc; ++arg) {
            const std::string name = argv[arg];
            FuncTable::iterator func = functions.find(name);
            if (func == functions.end()) {
                std::cerr << "unknown test '" << name << "'" << std::endl;
            } else {
                run_benchmark(name, suffix, func->second, iterations);
            }
        }

    } else {
        for (FuncTable::iterator func = functions.begin(); func != functions.end(); ++func) {
            run_benchmark(func->first, suffix, func->second, iterations);
        }
    }

    return 0;
}
