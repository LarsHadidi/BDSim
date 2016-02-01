#pragma once

#include <vector>
#include <assert.h>
#include <bits/postypes.h>
#include <boost/filesystem/path.hpp>

#include <buffering/Buffer.h>

namespace buffering {
    class BufferBuilder {
        public:
            BufferBuilder(boost::filesystem::path& file, size_t fileSize, unsigned int bufferCount, size_t cacheSize);
            ~BufferBuilder();
            const std::vector<std::streampos>* const dispatchPoints(const std::vector<std::streampos>* const SPLIT_POINTS) const;
            std::vector<Buffer>* const createBuffers(const std::vector<std::streampos>* const DISPATCH_POINTS) const;
        private:
            const std::string STR_FILE;
            const size_t FILE_SIZE;
            const unsigned int BUFFER_COUNT;
            const size_t CACHE_SIZE;
    };
}