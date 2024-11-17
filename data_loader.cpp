#include "data_loader.h"

int load_data_ser(const std::string &filename, data &data) {
    // open the file in binary mode
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the entire file into a buffer
    char *buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    fclose(file);


    // parse the buffer into lines
    std::vector<std::string_view> lines;
    size_t startIdx = 0;
    for (size_t i = 0; i < fileSize; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  // move past the newline character
        }
    }

    // skip the header
    lines.erase(lines.begin());


    // clean the data
    data.x.clear();
    data.y.clear();
    data.z.clear();

    // reserve memory for the data - number of lines
    size_t numLines = lines.size();
    data.x.resize(numLines);
    data.y.resize(numLines);
    data.z.resize(numLines);

    for (size_t i = 0; i < lines.size(); ++i) {
        const char* line_ptr = lines[i].data();

        // Skip the timestamp
        while (*line_ptr && *line_ptr != ',') // move past the timestamp
            line_ptr++;
        line_ptr++; // move past the comma

        // Parse x
        double x = std::strtod(line_ptr, const_cast<char**>(&line_ptr));
        line_ptr++; // move past the comma

        // Parse y
        double y = std::strtod(line_ptr, const_cast<char**>(&line_ptr));
        line_ptr++; // move past the comma

        // Parse z
        double z = std::strtod(line_ptr, const_cast<char**>(&line_ptr));

        // Add x, y, z to the data
        data.x[i] = x;
        data.y[i] = y;
        data.z[i] = z;
    }


    // Clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}


int load_data_par(const std::string &filename, data &data) {
    // open the file in binary mode
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the entire file into a buffer
    char *buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // parse the buffer into lines
    std::vector<std::string_view> lines;
    size_t startIdx = 0;
    for (size_t i = 0; i < fileSize; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  // move past the newline character
        }
    }

    // skip the header
    lines.erase(lines.begin());

    // clean the data
    data.x.clear();
    data.y.clear();
    data.z.clear();

    // reserve memory for the data - number of lines
    size_t numLines = lines.size();
    data.x.resize(numLines);
    data.y.resize(numLines);
    data.z.resize(numLines);

#pragma omp parallel for default(none) shared(lines, data)

//    for (size_t i = 0; i < lines.size(); ++i) {
//        std::istringstream stream((std::string(lines[i]))); // Convert to std::string to work with istringstream
//        std::string value1, value2, value3, value4;
//
//        if (std::getline(stream, value1, ',') &&
//            std::getline(stream, value2, ',') &&
//            std::getline(stream, value3, ',') &&
//            std::getline(stream, value4, ',')) {
//            data.x[i] = atof(value2.c_str());
//            data.y[i] = atof(value3.c_str());
//            data.z[i] = atof(value4.c_str());
//        }
//    }

    for (size_t i = 0; i < lines.size(); ++i) {
        char line[256];
        strncpy(line, lines[i].data(), lines[i].size());
        line[lines[i].size()] = '\0';

        char *token = strtok(line, ",");
        token = strtok(nullptr, ",");
        data.x[i] = atof(token);
        token = strtok(nullptr, ",");
        data.y[i] = atof(token);
        token = strtok(nullptr, ",");
        data.z[i] = atof(token);
    }

    // Clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}