#include <samplers/VisualizationSampler.h>

namespace samplers{

    VisualizationSampler::VisualizationSampler(boost::property_tree::iptree* initConf, datastructures::CellList*const cells, interfaces::ForceField *const forceField, uint32_t szBuffer) :
            CELLS(cells),
            FORCE_FIELD(forceField),
            BUFFER_SIZE(szBuffer),
            CONFS_BUFFER(new std::vector<std::vector<Conf>*>())
    {
        this->output = (boost::filesystem::path(boost::filesystem::path(initConf->get<std::string>("outputpath"))) / "VIZ").string();
        this->outStream.open(output, std::ios::out | std::ios::binary);
        this->writeHeader();
    }

    inline void VisualizationSampler::writeHeader(){
        this->outStream.write((char*)&(this->CELLS->cellSizeX),sizeof(double));
        this->outStream.write((char*)&(this->CELLS->cellSizeY),sizeof(double));
        this->outStream.write((char*)&(this->CELLS->cellCountX),sizeof(uint32_t));
        this->outStream.write((char*)&(this->CELLS->cellCountY),sizeof(uint32_t));
    }

    VisualizationSampler::~VisualizationSampler(){
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of VIZ-Sampler.";
        if(this->CONFS_BUFFER->size() > 0){
            BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data of VIZ-Sampler.";
            this->write();
        }
        this->outStream.flush();
        this->outStream.close();
        delete this->CONFS_BUFFER;
    }

    void VisualizationSampler::reset(){
        this->CONFS_BUFFER->clear();

        this->outStream.flush();
        this->outStream.close();

        boost::filesystem::remove(this->output);

        this->outStream.open(this->output, std::ios::out | std::ios::binary);
        this->writeHeader();
    }

    void VisualizationSampler::sample(){
        std::vector<Conf>* const CONF = new std::vector<Conf>();

        for(uint32_t i = 0; i < this->CELLS->cellCountTotal; ++i) {
            datastructures::llnode<datastructures::Particle>* node = this->CELLS->data[i].pListHead;
            while(node){
                datastructures::Particle p = node->value;

                datastructures::Particle pn = (node->next != 0x0) ? node->next->value : node->value;
                CONF->push_back({p.posX,p.posY,p.forceX,p.forceY,pn.posX,pn.posY,i});

                node = node->next;
            }
        }
        this->CONFS_BUFFER->push_back(CONF);

        if(this->CONFS_BUFFER->size() == this->BUFFER_SIZE) {
            this->write();
        }
    }


    /// Writes current state to numeric output file and flushes buffer.
    void VisualizationSampler::write(){
        for(std::vector<Conf>* CONF : *(this->CONFS_BUFFER)){
            for(Conf c : *CONF) {
                this->outStream.write((char*)&(c.pX),sizeof(double));
                this->outStream.write((char*)&(c.pY),sizeof(double));
                this->outStream.write((char*)&(c.fX),sizeof(double));
                this->outStream.write((char*)&(c.fY),sizeof(double));
                this->outStream.write((char*)&(c.pXn),sizeof(double));
                this->outStream.write((char*)&(c.pYn),sizeof(double));
                this->outStream.write((char*)&(c.cID),sizeof(uint32_t));
            }
            CONF->clear();
        }
        CONFS_BUFFER->clear();
    }

}