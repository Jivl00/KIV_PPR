#include "data_loader.h"

int load_data_ser(const std::string &filename, data &data) {
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        std::cerr << "File " << filename << " not found" << std::endl;
        return EXIT_FAILURE;
    }
    // Clean the data
    data.x.clear();
    data.y.clear();
    data.z.clear();

    // Set buffer size to 8MB
    const size_t bufferSize = 8 * MB;
    setvbuf(file, nullptr, _IOFBF, bufferSize);


    char line[256];
    fgets(line, sizeof(line), file); // Skip header

    data.x.reserve(20 * MB);
    data.y.reserve(20 * MB);
    data.z.reserve(20 * MB);

    while (fgets(line, sizeof(line), file)) {
        char *line_ptr = line;
        while (*line_ptr && *line_ptr != ',') // Skip timestamp
            line_ptr++;
        line_ptr++; // Skip comma

        double accX = std::strtod(line_ptr, &line_ptr); // Read X
        line_ptr++; // Skip comma

        double accY = std::strtod(line_ptr, &line_ptr); // Read Y
        line_ptr++; // Skip comma

        double accZ = std::strtod(line_ptr, &line_ptr); // Read Z


        data.x.emplace_back(accX);
        data.y.emplace_back(accY);
        data.z.emplace_back(accZ);
    }

    fclose(file);
    return EXIT_SUCCESS;
}


int load_data_par(const std::string &filename, data &data) {

    // Open the file in binary mode
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the entire file into a buffer
    char *buffer = new char[fileSize + 1];  // +1 for null-terminator
    fread(buffer, 1, fileSize, file);
    fclose(file);

    buffer[fileSize] = '\0';  // Null-terminate the string

    // Parse the buffer into lines
    std::vector<std::string> lines;
    size_t startIdx = 0;
    for (size_t i = 0; i < fileSize; ++i) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  // Move past the newline character
        }
    }

    // Handle the last line (if any)
    if (startIdx < fileSize) {
        lines.emplace_back(buffer + startIdx);
    }

    // Clean up
    delete[] buffer;

    return EXIT_SUCCESS;
}

//int load_data_par(const std::string &filename, data &data) {
//    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
//    if (hFile == INVALID_HANDLE_VALUE) {
//        std::cerr << "File " << filename << " not found" << std::endl;
//        return EXIT_FAILURE;
//    }
//
//    LARGE_INTEGER fileSize;
//    if (!GetFileSizeEx(hFile, &fileSize)) {
//        std::cerr << "Could not get file size" << std::endl;
//        CloseHandle(hFile);
//        return EXIT_FAILURE;
//    }
//
//    HANDLE hMapFile = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
//    if (hMapFile == nullptr) {
//        std::cerr << "Could not create file mapping" << std::endl;
//        CloseHandle(hFile);
//        return EXIT_FAILURE;
//    }
//
//    char *fileData = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0));
//    if (fileData == nullptr) {
//        std::cerr << "Could not map view of file" << std::endl;
//        CloseHandle(hMapFile);
//        CloseHandle(hFile);
//        return EXIT_FAILURE;
//    }
//
//    CloseHandle(hMapFile);
//    CloseHandle(hFile);
//
//    // Clean the data
//    data.x.clear();
//    data.y.clear();
//    data.z.clear();
//
//    // Read all lines into memory
//    std::vector<std::string> lines;
//    char *lineStart = fileData;
//    for (size_t i = 0; i < fileSize.QuadPart; ++i) {
//        if (fileData[i] == '\n') {
//            lines.emplace_back(lineStart, &fileData[i]);
//            lineStart = &fileData[i + 1];
//        }
//    }
//
//    UnmapViewOfFile(fileData);
//
//    // Skip header
//    if (!lines.empty()) {
//        lines.erase(lines.begin());
//    }
//
//    std::cout << "Read " << lines.size() << " lines" << std::endl;
//
//    return EXIT_SUCCESS;
//}

//int load_data_par(const std::string &filename, data &data) {
//    std::vector<std::string> lines;
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