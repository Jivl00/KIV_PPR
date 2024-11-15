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
    char *buffer = new char[fileSize + 1];  // +1 for null-terminator
    fread(buffer, 1, fileSize, file);
    fclose(file);

    buffer[fileSize] = '\0';  // Null-terminate the string

    // parse the buffer into lines
    std::vector<std::string_view> lines;
    size_t startIdx = 0;
    for (size_t i = 0; i < fileSize; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  // move past the newline character
        }
    }

    // handle the last line (if any)
    if (startIdx < fileSize) {
        lines.emplace_back(buffer + startIdx);
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
    char *buffer = new char[fileSize + 1];  // +1 for null-terminator
    fread(buffer, 1, fileSize, file);
    fclose(file);

    buffer[fileSize] = '\0';  // Null-terminate the string

    // parse the buffer into lines
    std::vector<std::string_view> lines;
    size_t startIdx = 0;
    for (size_t i = 0; i < fileSize; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  // move past the newline character
        }
    }

    // handle the last line (if any)
    if (startIdx < fileSize) {
        lines.emplace_back(buffer + startIdx);
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
#pragma omp parallel for schedule(static)
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string_view line = lines[i];
        size_t pos = 0;

        // Skip the timestamp
        pos = line.find(',') + 1;

        // Parse x
        size_t next_pos = line.find(',', pos);
        double x = std::stod(std::string(line.substr(pos, next_pos - pos)));
        pos = next_pos + 1;

        // Parse y
        next_pos = line.find(',', pos);
        double y = std::stod(std::string(line.substr(pos, next_pos - pos)));
        pos = next_pos + 1;

        // Parse z
        double z = std::stod(std::string(line.substr(pos)));

        // Add x, y, z to the data
        data.x[i] = x;
        data.y[i] = y;
        data.z[i] = z;
    }


    // Clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}

//int load_data_par(const std::string &filename, data &data) {
//    std::vector<std::string_view> lines;
//
//    // Open the file for reading
//    std::ifstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::cerr << "File " << filename << " not found!" << std::endl;
//        return EXIT_FAILURE;
//    }
//
//    // Get the file size
//    file.seekg(0, std::ios::end);
//    size_t fileSize = file.tellg();
//    file.seekg(0, std::ios::beg);
//
//    // Read the whole file into a string buffer
//    std::string fileContent(fileSize, '\0');
//    file.read(&fileContent[0], fileSize);
//
//    // Now, parse the file content into lines
//    size_t startIdx = 0;
//    for (size_t i = 0; i < fileSize; ++i) {
//        if (fileContent[i] == '\n') {
//            lines.push_back(fileContent.substr(startIdx, i - startIdx));
//            startIdx = i + 1; // Move past the newline
//        }
//    }
//
//    // Don't forget the last line if it's not empty
//    if (startIdx < fileSize) {
//        lines.push_back(fileContent.substr(startIdx));
//    }
//
//    return EXIT_SUCCESS;
//}