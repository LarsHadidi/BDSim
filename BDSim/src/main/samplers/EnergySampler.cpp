#include <samplers/EnergySampler.h>
#include <iostream>
#include <iomanip>

namespace samplers {

	EnergySampler::EnergySampler (boost::property_tree::iptree* initConf, interfaces::ForceField* const forceField, double samplingrate, double smpCountPerDatapoint) :
	    FORCE_FIELD(forceField),
	    SAMPLES_PER_DATAPOINT(smpCountPerDatapoint),
	    CALLS_PER_SAMPLE(1 / samplingrate)
	{
		this->smpCalls = 0;
		this->samples = 0;
		this->cumulativeEnergy = 0;
		this->output = (boost::filesystem::path(boost::filesystem::path(initConf->get<std::string>("outputpath"))) / "ENERGY").string();

		this->outStream.open(output, std::ios::out);
	}
	EnergySampler::~EnergySampler(){
		BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of ENERGY-Sampler.";
		this->outStream.flush();
		this->outStream.close();
	}

	void EnergySampler::reset(){
		this->smpCalls = 0;
		this->samples = 0;
		this->cumulativeEnergy = 0;

		this->outStream.flush();
		this->outStream.close();

		boost::filesystem::remove(this->output);

		this->outStream.open(this->output, std::ios::out);
	}
	void EnergySampler::sample(){
			++this->smpCalls;
			if(this->smpCalls >= this->CALLS_PER_SAMPLE){
				this->cumulativeEnergy += this->FORCE_FIELD->getEnergy();

				this->smpCalls = 0;
				++this->samples;
			}
			if(this->samples >= this->SAMPLES_PER_DATAPOINT) {
				double E = this->cumulativeEnergy / this->SAMPLES_PER_DATAPOINT;

				this->outStream << std::setprecision(DIGITS) << E << std::endl;

				this->samples = 0;
				this->cumulativeEnergy = 0;
			}
		}
}
