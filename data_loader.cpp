#include "data_loader.h"

int load_data(const std::string &filename, data &data) {
    // open the file in binary mode
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filename.c_str(), "rb");
    if (err != 0) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    const auto file_size = static_cast<size_t>(ftell(file));
    fseek(file, 0, SEEK_SET);

    // read the entire file into a buffer
    char *buffer = new char[file_size];
    fread(buffer, 1, file_size, file);

    // close the file
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

    std::for_each(std::execution::par, lines.begin(), lines.end(), [&](const std::string_view& line) {
        auto i =static_cast <size_t> (&line - &lines[0]);  // get the index of the line

        char line_cstr[256];
//        strncpy(line_cstr, line.data(), line.size());
        strncpy_s(line_cstr, sizeof(line_cstr), line.data(), line.size());  // copy the line
        line_cstr[sizeof(line_cstr) - 1] = '\0';

        char* saveptr = nullptr;

        strtok_s(line_cstr, ",", &saveptr); // skip the timestamp

        // Parse x, y, z
        char* token = strtok_s(nullptr, ",", &saveptr);
        data.x[i] = std::strtod(token, nullptr); // parse x

        token = strtok_s(nullptr, ",", &saveptr);
        data.y[i] = std::strtod(token, nullptr); // parse y

        token = strtok_s(nullptr, ",", &saveptr);
        data.z[i] = std::strtod(token, nullptr); // parse z
    });

    // clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}