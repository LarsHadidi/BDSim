#include <blackBoxFactories/PRNGEngineFactory.h>

namespace blackBoxFactories {

	PRNGEngineFactory::MTEngine PRNGEngineFactory::createMersenneTwister(){
		BOOST_LOG_TRIVIAL(info) << "\tInitializing MT19937 Engine";
		MTEngine engine = MTEngine();
		BOOST_LOG_TRIVIAL(info) << "\t\tGetting seed sequence.";
		const uint32_t SEED_BUFFER_SZ = 32;
		char* buffer = new char[SEED_BUFFER_SZ];

		std::ifstream inStream("/dev/urandom", std::ios_base::in | std::ios_base::binary);

		if(inStream){
			inStream.read(buffer, SEED_BUFFER_SZ);
			std::string strBuffer(buffer);
			boost::random::seed_seq seedSeq(strBuffer.begin(), strBuffer.end());
			engine.seed(seedSeq);
			BOOST_LOG_TRIVIAL(info) << "\t\tSeeded " << SEED_BUFFER_SZ << " bytes from /dev/urandom.";
		}
		else{
			BOOST_LOG_TRIVIAL(info) << "\t\tNo access to /dev/urandom, using time based seed.";
			engine.seed((double)std::chrono::high_resolution_clock::now().time_since_epoch().count());
		}
		delete[] buffer;
		inStream.close();
		BOOST_LOG_TRIVIAL(info) << "\tMT19937 Engine initialization successful.";

#if defined _DEBUG || defined _TEST
BOOST_LOG_TRIVIAL(info) << "SEED RESET FOR DEBUG AND TESTING PURPOSES.";
engine.seed(1);
#endif

		return engine;
	}

}
