#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <tuple>
#include <utility>
#include <variant>
#include <execution>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <vector>
#include "execution_policy.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef _FLOAT
using real = float;
#define LOAD _mm256_loadu_ps
#define STORE _mm256_storeu_ps
#define SETZERO _mm256_setzero_ps
#define ADD _mm256_add_ps
#define SUB _mm256_sub_ps
#define MUL _mm256_mul_ps
#define ANDNOT _mm256_andnot_ps
#define STRIDE __m256
#define SET1 _mm256_set1_ps
#define str_to_real std::strtof
#else
using real = double;
#define LOAD _mm256_loadu_pd
#define STORE _mm256_storeu_pd
#define SETZERO _mm256_setzero_pd
#define ADD _mm256_add_pd
#define SUB _mm256_sub_pd
#define MUL _mm256_mul_pd
#define ANDNOT _mm256_andnot_pd
#define STRIDE __m256d
#define SET1 _mm256_set1_pd
#define str_to_real std::strtod
#endif

/**
 * @brief Measure the time taken by a function to execute
 * Using variadic templates to accept any function and its arguments
 *
 * @tparam Func Function type
 * @tparam Args Argument types
 * @param f Function to measure
 * @param args Arguments to pass to the function
 * @return std::pair<double, decltype(f(args...))> Time taken and return value of the function
 */
template<typename Func, typename... Args>
auto measure_time(Func &&func, Args &&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    return std::make_tuple(duration.count(), result);
}

/**
 * @brief Argument parser class to parse command line arguments
 */
class arg_parser {
public:
    std::string usage;

    /**
     * @brief Set the usage string for the argument parser
     * @param default_usage Usage string
     */
    void set_usage(const std::string &default_usage) {
        this->usage = default_usage;
    }

    /**
     * @brief Argument class to store argument details
     */
    class argument {
    public:
        std::string name;
        std::string help;
        bool required;
        bool has_value;
        std::string default_value;
        std::string value;

        argument() : required(false), has_value(true) {}

        argument(std::string name, std::string help, bool required, bool has_value,
                 const std::string &default_value)
                : name(std::move(name)), help(std::move(help)), required(required), has_value(has_value),
                  default_value(default_value),
                  value(default_value) {}
    };

    /**
     * @brief Mutually exclusive group class to store mutually exclusive arguments
     */
    class mutually_exclusive_group {
    public:
        std::vector<std::string> arguments;

        /**
         * @brief Add an argument to the mutually exclusive group
         * @param name Argument name
         */
        void add_argument(const std::string &name) {
            arguments.push_back(name);
        }
    };

    /**
     * @brief Add an argument to the argument parser
     * @param name Argument name
     * @param help Argument help message
     * @param required Is the argument required
     * @param has_value Does the argument have a value
     * @param default_value Default value for the argument
     */
    void add_argument(const std::string &name, const std::string &help = "", bool required = false,
                      bool has_value = true, const std::string &default_value = "") {
        arguments[name] = argument(name, help, required, has_value, default_value);
    }

    /**
     * @brief Add a mutually exclusive group to the argument parser
     * @return Reference to the mutually exclusive group
     */
    mutually_exclusive_group &add_mutually_exclusive_group() {
        groups.emplace_back();
        return groups.back();
    }

    /**
     * @brief Parse the command line arguments
     * @param argc Number of arguments
     * @param argv Argument values
     */
    void parse_args(int argc, char *argv[]) {
        std::unordered_map<std::string, bool> parsed_arguments;

        for (int i = 1; i < argc; ++i) { // skip the first argument which is the program name
            std::string arg = argv[i];
            // check if help is requested
            if (arg == "--help") {
                print_help();
            }
            if (arguments.find(arg) != arguments.end()) { // check if the argument is valid
                argument &arg_obj = arguments[arg];
                parsed_arguments[arg] = true;
                if (arg_obj.has_value) {
                    if (i + 1 < argc) {
                        arg_obj.value = argv[++i];
                    } else {
                        throw std::runtime_error("Argument " + arg + " requires a value.");
                    }
                } else {
                    arg_obj.value = "true";
                }
            } else {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        for (const auto &[name, arg_obj]: arguments) {
            if (arg_obj.required && parsed_arguments.find(name) == parsed_arguments.end()) {
                throw std::runtime_error("Missing required argument: " + name);
            }
        }

        for (const auto &group: groups) { // check if mutually exclusive arguments are used together
            int count = 0;
            for (const auto &name: group.arguments) {
                if (parsed_arguments.find(name) != parsed_arguments.end()) {
                    ++count;
                }
            }
            if (count > 1) {
                throw std::runtime_error("Mutually exclusive arguments " + std::accumulate(
                        std::next(group.arguments.begin()), group.arguments.end(), group.arguments.front(),
                        [](std::string& a, const std::string& b) { return a.append(", ").append(b); }) + " cannot be used together.");
            }
        }
    }

    /**
     * @brief Get the value of an argument
     * @param name Argument name
     * @return Argument value
     */
    [[nodiscard]] std::string get(const std::string &name) const {
        if (arguments.find(name) != arguments.end()) {
            return arguments.at(name).value;
        }
        throw std::runtime_error("Argument " + name + " not found.");
    }

    /**
     * @brief Print the help message
     */
    void print_help() const {
        std::cerr << "Usage: " << usage << std::endl;
        std::cerr << "List of arguments:" << std::endl;
        for (const auto &[name, arg_obj]: arguments) {
            std::cerr << name << ": " << arg_obj.help
                      << (arg_obj.default_value.empty() ? "" : " (default: " + arg_obj.default_value + ")")
                      << (arg_obj.required ? " [required]" : "") << std::endl;
        }
    }

    std::unordered_map<std::string, argument> arguments;
private:
    std::vector<mutually_exclusive_group> groups;
};
