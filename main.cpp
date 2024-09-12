
#include <string>
#include <vector>
#include <cstdio>
#include "file.h"
#include "xmi.h"

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        std::vector<char> file_buffer;
        std::string in_file = argv[i] + std::string(".xmi");

        if (!LoadFile(in_file, file_buffer)) {
            printf("Could not load %s\n", in_file.c_str());
            continue;
        }

        printf("Loaded %s\n", in_file.c_str());

        // Store the lengths of multiple MIDIs
        std::vector<size_t> midiLengths;
        std::vector<unsigned char*> midiData = TranscodeXmiToMid((const unsigned char*)&file_buffer[0], file_buffer.size(), midiLengths);

        if (midiData.empty()) {
            printf("Could not convert %s\n", in_file.c_str());
            continue;
        }

        // Save each MIDI separately
        for (size_t j = 0; j < midiData.size(); j++) {
            std::string out_file = argv[i] + std::string("_") + std::to_string(j) + std::string(".mid");

            if (!SaveFile(out_file, midiData[j], midiLengths[j])) {
                printf("Could not save %s\n", out_file.c_str());
                continue;
            }

            printf("Converted and saved %s to %s\n", in_file.c_str(), out_file.c_str());

            // Clean up the allocated memory after saving
            delete[] midiData[j];
        }
    }

    return 0;
}