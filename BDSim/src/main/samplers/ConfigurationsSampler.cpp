#include <samplers/ConfigurationsSampler.h>

namespace samplers {
	ConfigurationsSampler::ConfigurationsSampler (boost::property_tree::iptree*initConf, datastructures::linkedParticleList**const particlesAddress, interfaces::ForceField* const forceField, double samplingrate, uint32_t szBuffer) :

			PTR_PTR_PARTICLES(particlesAddress),
			FORCE_FIELD(forceField),
			CALLS_PER_SAMPLE(1 / samplingrate),
			BUFFER_SIZE(szBuffer),
			buffer(new datastructures::Particle*[BUFFER_SIZE]),
			PARTICLE_COUNT((*PTR_PTR_PARTICLES)->count)
	{

		for(uint32_t i = 0; i < BUFFER_SIZE; ++i) {
			datastructures::Particle* pData = (datastructures::Particle*)calloc(PARTICLE_COUNT, sizeof(datastructures::Particle));
			this->buffer[i] = pData;
		}
		this->bufferPosition = 0;
		this->smpCalls = 0;
		this->output = (boost::filesystem::path(boost::filesystem::path(initConf->get<std::string>("outputpath"))) / "CONFS").string();

		this->outStream.open(output, std::ios::out | std::ios::binary);
	}

	ConfigurationsSampler::~ConfigurationsSampler(){
		BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of CONFS-Sampler.";
		if(this->bufferPosition > 0){
			BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data of CONFS-Sampler.";
			this->writeBufferUntilPos();
		}
		this->outStream.flush();
		this->outStream.close();

		for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
			free(this->buffer[i]);
		}
		free(this->buffer);
	}

	void ConfigurationsSampler::reset(){
		for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
			free(this->buffer[i]);
		}
		for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
			datastructures::Particle* pData = (datastructures::Particle*)calloc(this->PARTICLE_COUNT, sizeof(datastructures::Particle));
			this->buffer[i] = pData;
		}
		this->bufferPosition = 0;
		this->smpCalls = 0;

		this->outStream.flush();
		this->outStream.close();

		boost::filesystem::remove(this->output);

		this->outStream.open(this->output, std::ios::out | std::ios::binary);
	}
	void ConfigurationsSampler::sample(){
		++this->smpCalls;
		if(this->smpCalls >= this->CALLS_PER_SAMPLE){
			this->smpCalls = 0;

			for(uint32_t i = 0; i < (*(this->PTR_PTR_PARTICLES))->count; ++i) {
				memcpy(this->buffer[this->bufferPosition] + i, &((*(this->PTR_PTR_PARTICLES))->data[i].value), sizeof(datastructures::Particle));
			}
			this->bufferPosition++;

			if(this->bufferPosition == this->BUFFER_SIZE) {
				this->writeBufferUntilPos();
				this->bufferPosition = 0;
			}
		}
	}


	/// Writes current state to numeric output file and flushes buffer.
	void ConfigurationsSampler::writeBufferUntilPos(){
		for(uint32_t pos = 0; pos < this->bufferPosition; ++pos){
			// Write numeric values as IEEE encoded little endian binary data
			this->outStream.write((char*)(this->buffer[pos]), PARTICLE_COUNT * sizeof(datastructures::Particle));
		}
	}
}
