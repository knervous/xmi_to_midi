
#include <string>
#include <vector>
#include <cstdio>
#include "file.h"
#include "xmi.h"

// Expose a C function to the Emscripten runtime
extern "C" {

int xmi_to_midi(const char* file_name, const unsigned char* array_buffer, size_t array_buffer_size) {
    std::vector<char> file_buffer(array_buffer, array_buffer + array_buffer_size);

    printf("Loaded XMI from ArrayBuffer\n");

    std::vector<size_t> midiLengths;
    std::vector<unsigned char*> midiData = TranscodeXmiToMid((const unsigned char*)&file_buffer[0], file_buffer.size(), midiLengths);

    if (midiData.empty()) {
        printf("Could not convert XMI data\n");
        return -1;
    }

    for (size_t j = 0; j < midiData.size(); j++) {
        std::string out_file = std::string(file_name) + "_" + std::to_string(j) + ".mid";

        if (!SaveFile(out_file, midiData[j], midiLengths[j])) {
            printf("Could not save %s\n", out_file.c_str());
            return -1;
        }

        printf("Converted and saved to %s\n", out_file.c_str());

        delete[] midiData[j];
    }

    return 0; 
}

}  // extern "C"
