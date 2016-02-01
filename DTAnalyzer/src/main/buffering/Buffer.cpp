#include <buffering/Buffer.h>

namespace buffering {

    Buffer::Buffer(const std::string FILE, std::streampos startPosition, size_t bufferSize, size_t cacheSize) :
    BUFFER_SIZE(bufferSize), CACHE_SIZE(cacheSize), START_POSITION(startPosition) {
        this->cachePosition = cacheSize;
        this->bytesRead = 0;
        #pragma omp critical
        {
            this->inputStream = new std::ifstream(FILE, std::ios::in | std::ios::binary);
            this->inputStream->seekg(startPosition, std::ios::beg);
        }
        this->cache = new char[cacheSize];
        this->load();
    }

    Buffer::~Buffer() {
        #pragma omp critical
        {
            this->inputStream->close();
            delete this->inputStream;
        }
        delete this->cache;
    }

    bool Buffer::hasBytes() {
        return bytesRead < BUFFER_SIZE;
    }

    char *Buffer::getBytes(const size_t BYTE_COUNT) {
        assert(BYTE_COUNT <= CACHE_SIZE && "Precondition");
        this->bytesRead += BYTE_COUNT;
        assert(bytesRead <= BUFFER_SIZE && "Precondition");

        if (this->cachePosition + BYTE_COUNT > this->CACHE_SIZE) {
            load();
        }
        this->cachePosition += BYTE_COUNT;
        return this->cache + this->cachePosition - BYTE_COUNT;

    }

    void Buffer::load() {
        #pragma omp critical
        {
            this->inputStream->seekg(this->cachePosition - this->CACHE_SIZE, std::ios::cur);
            this->inputStream->read(this->cache, this->CACHE_SIZE);
            this->cachePosition = 0;
        }
    }

    size_t Buffer::getBufferSize() const {
        return this->BUFFER_SIZE;
    }

    void Buffer::reset() {
        #pragma omp critical
        {
            this->inputStream->clear();
            this->inputStream->seekg(this->START_POSITION, std::ios::beg);
            this->inputStream->read(this->cache, this->CACHE_SIZE);
            this->cachePosition = 0;
            this->bytesRead = 0;
        }
    }
}