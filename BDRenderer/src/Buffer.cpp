#include "Buffer.h"


Buffer::Buffer(std::string inputFile, std::streamoff startPosition, size_t bufferSize) :
START_POSITION(startPosition), BUFFER_SIZE(bufferSize)
{
    this->inputStream = new std::ifstream(inputFile, std::ios::in | std::ios::binary);
    this->buffer = new char[MAX_BYTES];
}

Buffer::~Buffer()
{
    this->inputStream->close();
    delete this->inputStream;
    delete this->buffer;
}

char* Buffer::get(const uint64_t position, const size_t size) {
    uint64_t min = this->currentFragment * MAX_BYTES;
    uint64_t max = min + MAX_BYTES;
    if (position < min || position + size >= max) {
        uint64_t fragment = position / MAX_BYTES;
        load(fragment);
    }
    min = this->currentFragment * MAX_BYTES;
    return this->buffer + (position - min);
}

void Buffer::load(const uint64_t FRAGMENT) {
    this->currentFragment = FRAGMENT;
    this->inputStream->seekg(this->START_POSITION + FRAGMENT * MAX_BYTES, std::ios::beg);
    this->inputStream->read(this->buffer, MAX_BYTES);
}
