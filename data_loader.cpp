#include "data_loader.h"

int load_data(const std::string &filename, data &data) {
    // open the file in binary mode
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the entire file into a buffer
    char *buffer = new char[file_size];
    fread(buffer, 1, file_size, file);
    fclose(file);

    // parse the buffer into lines
    std::vector<std::string_view> lines;
    size_t start_idx = 0;
    for (size_t i = 0; i < file_size; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + start_idx, i - start_idx);
            start_idx = i + 1;  // move past the newline character
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
    for (size_t i = 0; i < lines.size(); ++i) {
        char line[256];
        strncpy(line, lines[i].data(), lines[i].size()); // copy the line
        line[lines[i].size()] = '\0'; // null-terminate the string

        // note: atof way faster than std::stod for this case
        strtok(line, ","); // skip the timestamp
        char *token = strtok(nullptr, ",");
        data.x[i] = atof(token); // parse x
        token = strtok(nullptr, ",");
        data.y[i] = atof(token); // parse y
        token = strtok(nullptr, ",");
        data.z[i] = atof(token); // parse z
    }

    // clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}