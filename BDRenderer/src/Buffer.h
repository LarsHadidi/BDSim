#pragma once

#define MAX_BYTES 4096

#include <iostream>
#include <fstream>
#include <string>

class Buffer {
    public:
        Buffer(std::string inputFile, std::streamoff startPosition, size_t bufferSize);
        ~Buffer();
        char* get(const uint64_t position, const size_t size);
    private:
        void load(const uint64_t FRAGMENT);
        char* buffer;
        uint64_t currentFragment;
        std::ifstream*  inputStream;
        const std::streamoff START_POSITION;
        const size_t BUFFER_SIZE;
};
