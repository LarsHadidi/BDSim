#include <samplers/PressureSampler.h>

namespace samplers {

	PressureSampler::PressureSampler(boost::property_tree::iptree* initConf,
									 interfaces::ForceField *const forceField, double samplingrate,
									 double smpCountPerDatapoint) :
	 FORCE_FIELD(forceField),
	 A(initConf->get<double>("dimx") * initConf->get<double>("dimy")),
	 PARTICLE_COUNT(initConf->get<uint32_t>("particles")),
	 SAMPLES_PER_DATAPOINT(smpCountPerDatapoint),
	 CALLS_PER_SAMPLE(1 / samplingrate),
	 temperature(initConf->get<double>("temperature"))

	{
		this->smpCalls = 0;
		this->samples = 0;
		this->cumulativeVirial = 0;
		this->output = (boost::filesystem::path(boost::filesystem::path(initConf->get<std::string>("outputpath"))) / "PRESSURE").string();

		this->outStream.open(output, std::ios::out);
	}

	PressureSampler::~PressureSampler() {
		BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of PRESSURE-Sampler.";
		this->outStream.flush();
		this->outStream.close();
	}

	void PressureSampler::reset(){
		this->smpCalls = 0;
		this->samples = 0;
		this->cumulativeVirial = 0;

		this->outStream.flush();
		this->outStream.close();

		boost::filesystem::remove(this->output);

		this->outStream.open(this->output, std::ios::out);
	}

	void PressureSampler::sample(){
		++this->smpCalls;
		if(this->smpCalls >= this->CALLS_PER_SAMPLE){
			this->smpCalls = 0;

			this->cumulativeVirial += this->FORCE_FIELD->getVirial();

			++this->samples;
		}
		if(this->samples >= this->SAMPLES_PER_DATAPOINT) {
			this->samples = 0;

			double P = 0.5 / this->A * this->cumulativeVirial / this->SAMPLES_PER_DATAPOINT + (double)this->PARTICLE_COUNT / this->A * this->temperature;

			this->outStream << std::setprecision(DIGITS) << P << std::endl;

			this->cumulativeVirial = 0;
		}
	}
}
