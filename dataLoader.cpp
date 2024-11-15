#include "dataLoader.h"

int loadAccData(const std::string &filename, data &data) {
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
