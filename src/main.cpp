#include <chrono>
#include <iostream>
#include <map>
#include <execution>
#include <filesystem>
#include <fstream>

#include "data_loader.h"
#include "execution_policy.h"
#include "device_type.h"
#include "my_utils.h"

arg_parser set_args() {
    arg_parser parser;
    parser.add_argument("--help", "Show help message", false, false);
    parser.add_argument("--input", "Path to input file or directory with data in .csv format", true, true);
    parser.add_argument("--output", "Output directory for results", false, true, "results");
    parser.add_argument("--repetitions", "Number of repetitions for the experiment", false, true, "1");
    parser.add_argument("--num_partitions", "Number of partitions for the data", false, true, "1");
    parser.add_argument("--gpu", "Device type - CPU or GPU", false, false);
    parser.add_argument("--parallel", "Execution policy - parallel or sequential", false, false);
    parser.add_argument("--vectorized", "AVX2 vectorization", false, false);
    parser.add_argument("--all_variants", "Run all variants of the algorithm", false, false);


    auto &group = parser.add_mutually_exclusive_group();
    group.add_argument("--gpu");
    group.add_argument("--vectorized");
    group.add_argument("--all_variants");

    auto &group2 = parser.add_mutually_exclusive_group();
    group2.add_argument("--parallel");
    group2.add_argument("--all_variants");


    parser.set_usage("Example usage: ./main --input data/ACC_001.csv --repetitions 10 --num_partitions 4 --gpu");
    return parser;
}

std::vector<std::string> check_input(const std::string &input) {
    std::vector<std::string> files;
    if (!std::filesystem::exists(input)) { // check if the input file or directory exists
        throw std::runtime_error("Input file or directory does not exist");
    }
    if (std::filesystem::is_directory(input)) {
        for (const auto &entry: std::filesystem::directory_iterator(input)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                files.push_back(entry.path().string());
            } else {
                std::cerr << "Skipping file " << entry.path().string() << std::endl;
            }
        }
    } else if (std::filesystem::is_regular_file(input) && input.substr(input.find_last_of('.') + 1) == "csv") {
        files.push_back(input);
    } else {
        throw std::runtime_error("Input file is not a .csv file");
    }

    if (files.empty()) {
        throw std::runtime_error("No .csv files found in the input directory");
    }

    return files;
}

void check_output(const std::string &out_dir) {
    if (!std::filesystem::exists(out_dir)) { // check if the output directory exists
        std::filesystem::create_directory(out_dir);
    }
    if (!std::filesystem::is_directory(out_dir)) { // check if the output path is a directory
        throw std::runtime_error("Output path is not a directory");
    }
}

size_t check_numeric(const std::string &value, const std::string &name) {
    if (!std::all_of(value.begin(), value.end(), ::isdigit)) {
        throw std::runtime_error(name + " must be a positive integer");
    }
    if (std::stoul(value) == 0) { // check if the value is a positive integer
        throw std::runtime_error(name + " must be a positive integer");
    }
    return std::stoul(value);
}

double do_comp(std::vector<real> &data_vec, real &CV, real &MAD, bool vec, const execution_policy &policy, const device_type &device, size_t repetitions) {
    std::vector<real> times;
    for (size_t i = 0; i < repetitions; ++i) {
        auto data_vec_copy = std::vector<real>(data_vec);
        std::visit([&](auto &&device) {
            auto [stat_time, stat_ret] = measure_time(
                    [&](std::vector<real> &data_vec_copy, real &cv, real &mad, bool is_vectorized,
                        const execution_policy &policy) {
                        return device.compute_CV_MAD(data_vec_copy, cv, mad, is_vectorized, policy);
                    }, data_vec_copy, CV, MAD, vec, std::cref(policy));

            if (stat_ret == EXIT_SUCCESS) {
                std::cout << "Coefficient of variance: " << CV << std::endl;
                std::cout << "Median absolute deviation: " << MAD << std::endl;
                std::cout << "Computed in " << stat_time << " seconds" << std::endl;
                times.push_back(stat_time);
            } else {
                std::cerr << "Failed to compute statistics" << std::endl;
            }

        }, device.get_device());
    }
    // return the median time
    std::sort(times.begin(), times.end());
    return (times[times.size() / 2] + times[(times.size() - 1) / 2]) / 2.0;
}


int main(int argc, char *argv[]) {
    arg_parser parser = set_args();

    try {
        parser.parse_args(argc, argv);
        const std::string &input = parser.arguments.at("--input").value;
        auto files = check_input(input);
        const std::string &output = parser.arguments.at("--output").value;
        check_output(output);
        const size_t repetitions = check_numeric(parser.arguments.at("--repetitions").value, "--repetitions");
        size_t num_partitions = check_numeric(parser.arguments.at("--num_partitions").value, "--num_partitions");
        bool gpu = parser.arguments.at("--gpu").value == "true";
        bool par = parser.arguments.at("--parallel").value == "true";
        bool vec = parser.arguments.at("--vectorized").value == "true";
        bool all_variants = parser.arguments.at("--all_variants").value == "true";


        std::cout << "Running computations on " << files.size() << " files"
                  << " with " << repetitions << " repetitions"
                  << " and " << num_partitions << " partitions" << std::endl;
        execution_policy policy(
                (par || all_variants) ? execution_policy::e_type::Parallel : execution_policy::e_type::Sequential);
        for (const auto &file: files) {
            std::cout << "File " << file;
            // create output file for results
            std::string out_file = output + "/" + std::filesystem::path(file).stem().string() + "_results.csv";
            std::ofstream results_file(out_file);
            if (!results_file.is_open()) {
                throw std::runtime_error("Failed to open output file");
            }
            results_file << "column,num_elements,comp_type,CV,MAD,time\n";

            // load data from file
            struct data data;
            auto [load_time, load_ret] = measure_time(
                    [&](const std::string &filename, struct data &data, const execution_policy &policy) {
                        return load_data(filename, data, policy);
                    }, file, data, std::cref(policy));

            if (load_ret == EXIT_SUCCESS) {
                std::cout << " loaded in " << load_time << " seconds" << std::endl;
            } else {
                std::cerr << "Failed to load data" << std::endl;
            }
            size_t data_size = data.x.size();
            size_t partition_size = data_size / num_partitions;
            size_t partition_end = partition_size;
            for (size_t i = 0; i < num_partitions; ++i) {
                auto copy_x = std::vector<real>(data.x.begin(),
                                                data.x.begin() + static_cast<int>(partition_end));
                auto copy_y = std::vector<real>(data.y.begin(),
                                                data.y.begin() + static_cast<int>(partition_end));
                auto copy_z = std::vector<real>(data.z.begin(),
                                                data.z.begin() + static_cast<int>(partition_end));
                partition_end = (i == num_partitions - 2) ? data_size : partition_end + partition_size;
                std::map<std::string, std::reference_wrapper<std::vector<real>>> data_map = {
                        {"x", copy_x},
                        {"y", copy_y},
                        {"z", copy_z}
                };
                for (const auto &pair: data_map) {
                    const std::string &name = pair.first;
                    std::vector<real> &data_vec = pair.second;

                    std::cout << "\nColumn " << name << " :";

                    size_t n = data_vec.size();

                    std::cout << n << " elements" << std::endl;
                    std::cout << "=============================" << std::endl;

                    if (all_variants) {
                        std::cout << "Running all variants" << std::endl;
                        //cpu
                        device_type device(device_type::d_type::CPU);
                        auto policies = {execution_policy::e_type::Sequential, execution_policy::e_type::Parallel};
                        auto vectorizations = {true, false};
                        for (auto ex_policy: policies) {
                            for (auto vectorized: vectorizations) {
                                real CV = 0;
                                real MAD = 0;
                                std::cout << "Running on CPU in " << (ex_policy == execution_policy::e_type::Parallel ? "parallel" : "sequential")
                                          << " with " << (vectorized ? "vectorization" : "no vectorization") << std::endl;
                                auto med_time = do_comp(data_vec, CV, MAD, vectorized, execution_policy(ex_policy), device, repetitions);
                                results_file << name << "," << n << ",CPU_" << (ex_policy == execution_policy::e_type::Parallel ? "parallel" : "sequential") << "_"
                                             << (vectorized ? "vectorized" : "no_vectorized") << "," << CV << "," << MAD << "," << med_time << "\n";
                            }
                        }
                        //gpu
                        real CV = 0;
                        real MAD = 0;
                        device_type device_gpu(device_type::d_type::GPU);
                        std::cout << "Running on GPU" << std::endl;
                        auto med_time = do_comp(data_vec, CV, MAD, vec, policy, device_gpu, repetitions);
                        results_file << name << "," << n << ",GPU," << CV << "," << MAD << "," << med_time << "\n";
                    } else {
                        real CV = 0;
                        real MAD = 0;
                        device_type device(gpu ? device_type::d_type::GPU : device_type::d_type::CPU);
                        std::cout << "Running on " << (gpu ? "GPU" : "CPU") << std::endl;
                        if (!gpu){
                            std::cout << "Running in " << (par ? "parallel" : "sequential") << " mode with " << (vec ? "vectorization" : "no vectorization") << std::endl;
                        }
                        auto med_time = do_comp(data_vec, CV, MAD, vec, policy, device, repetitions);
                        std::string comp_type = gpu ? "GPU" : "CPU_" + std::string(par ? "parallel" : "sequential") + "_" + std::string(vec ? "vectorized" : "no_vectorized");
                        results_file << name << "," << n << "," << comp_type << "," << CV << "," << MAD << "," << med_time << "\n";
                    }
                }
            }
            results_file.close();
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    //    // set device type - CPU or GPU
    //    device_type device(gpu ? device_type::d_type::GPU : device_type::d_type::CPU);
    //    std::cout << "Running on " << (gpu ? "GPU" : "CPU") << std::endl;
    //    // set execution policy - parallel or sequential
    //    execution_policy policy(par ? execution_policy::e_type::Parallel : execution_policy::e_type::Sequential);
    //    std::cout << "Running in " << (par ? "parallel" : "sequential") << " mode with " << (vec ? "vectorization" : "no vectorization") << std::endl;
    //
    //    // load data from file
    //    struct data data;
    //    auto [load_time, load_ret] = measure_time([&](const std::string& filename, struct data& data, const execution_policy& policy) {
    //        return load_data(filename, data, policy);
    //    }, DATA_FILE, data, std::cref(policy));
    //
    //    if (load_ret == EXIT_SUCCESS) {
    //        std::cout << "Data loaded in " << load_time << " seconds" << std::endl;
    //    } else {
    //        std::cerr << "Failed to load data" << std::endl;
    //    }
    //
    //    std::map<std::string, std::reference_wrapper<std::vector<real>>> data_map = {
    //            {"x", data.x},
    //            {"y", data.y},
    //            {"z", data.z}
    //    };
    //
    //    for (const auto& pair : data_map) {
    //        const std::string& name = pair.first;
    //        std::vector<real>& data_vec = pair.second;
    //
    //        std::cout << "\nColumn " << name << " :" << std::endl;
    //
    //        size_t n = data_vec.size();
    //
    //        std::cout << n << " elements" << std::endl;
    //        std::cout << "=============================" << std::endl;
    //
    //        real CV = 0;
    //        real MAD = 0;
    //
    //        std::visit([&](auto &&device) {
    //            auto [stat_time, stat_ret] = measure_time([&](std::vector<real> &data_vec, real &cv, real &mad, bool is_vectorized, const execution_policy &policy) {
    //                return device.compute_CV_MAD(data_vec, cv, mad, is_vectorized, policy);
    //            }, data_vec, CV, MAD, vec, std::cref(policy));
    //
    //            if (stat_ret == EXIT_SUCCESS) {
    //                std::cout << "Coefficient of variance: " << CV << std::endl;
    //                std::cout << "Median absolute deviation: " << MAD << std::endl;
    //                std::cout << "Computed in " << stat_time << " seconds" << std::endl;
    //            } else {
    //                std::cerr << "Failed to compute statistics" << std::endl;
    //            }
    //
    //        }, device.get_device());
    //
    //    }

    return 0;
}




