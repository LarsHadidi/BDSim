#include <samplers/BufferedBinarySampler.h>

namespace samplers {
   /* BufferedBinarySampler::BufferedBinarySampler(double rate, uint32_t szBuffer, std::string outputPath) :

            CALLS_PER_SAMPLE(1 / rate),
            BUFFER_SIZE(szBuffer),
            buffer(new T*[BUFFER_SIZE])

    {
        this->bufferPosition = 0;
        this->smpCalls = 0;
        this->outStream.open(outputPath, std::ios::out | std::ios::binary);
    }

    BufferedBinarySampler::~BufferedBinarySampler()
    {
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of" << typeid(this).name();
        if(this->bufferPosition > 0){
            BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data";
            this->writeBufferUntilPos();
        }
        this->outStream.flush();
        this->outStream.close();

        for(uint32_t i = 1; i < BUFFER_SIZE; ++i) {
            free(this->buffer[i]);
        }
        free(this->buffer);
    }*/
}