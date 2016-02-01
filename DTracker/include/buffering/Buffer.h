#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

namespace buffering {

    class Buffer {
    public:
        Buffer(const std::string FILE, std::streampos startPosition, size_t bufferSize, size_t cacheSize);

        ~Buffer();

        // Guarantees that the returned pointer points to a address holding BYTE_COUNT Bytes for current read only
        char *getBytes(const size_t BYTE_COUNT);

        size_t getBufferSize() const;

        void reset();

        bool hasBytes();

    private:
        void load();

        char *cache;
        uint64_t cachePosition;
        size_t bytesRead;
        std::ifstream *inputStream;
        const std::streampos START_POSITION;
        const size_t BUFFER_SIZE;
        const size_t CACHE_SIZE;
    };
}