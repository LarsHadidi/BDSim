#pragma once

#include <interfaces/Interfaces.h>
#include <datastructures/Datastructures.h>
#include <tools/Logging.h>
#include <vector>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>

namespace samplers {
    /// \brief Special sampler class for dabugging purposes.
    /// \author Lars Hadidi
    /// Writes a file for visualizing the internal simulation mechanisms.
    class VisualizationSampler : public interfaces::Sampler {
        public:
            /// \brief Struct holding all particle's information.
            typedef struct {
                double pX;
                double pY;
                double fX;
                double fY;
                double pXn;
                double pYn;
                uint32_t cID;
            } Conf;
            /// \brief Creates a configurations sampler object.
            /// The sampling data is stored into an internal buffer which will be flushed when full to the output stream which itself has its own buffer which is flushed to the operating system's disk management which writes th e data to disk.
            /// \param initConf Pointer to the initialization object.
            /// \param cells Pointer to the cell list pointing into the linked list holding the particles.
            /// \param forceField Pointer to the forcefield object.
            /// \param szBuffer Number of samples to be held in the internal buffer. As soon as the buffer is full, its contents will be flushed to disk. The output folder is given in the initialization object. The filename is predefined by the class.
            VisualizationSampler(boost::property_tree::iptree* initConf, datastructures::CellList*const cells, interfaces::ForceField *const forceField, uint32_t szBuffer);
            /// \brief Destroys the object and flushes its internal buffer as well as the output stream's buffer.
            /// This is important as the buffers may not be completely full after the simulation's termination.
            ~VisualizationSampler();
            /// \brief Records the visualization data for debugging purposes.
            void sample();
            /// \brief Clears the internal buffer and resets the object's internal state. Also removes all content from previously flushed data on disk.
            void reset();
        private:
            datastructures::CellList*const CELLS; ///< Pointer to the cell list.
            interfaces::ForceField* const FORCE_FIELD; ///< Pointer to the forcefield.
            std::string output; ///< Path to output file, including filename.
            std::ofstream outStream; ///< Output stream object.
            const uint32_t BUFFER_SIZE; ///< Size of the internal buffer
            std::vector<std::vector<Conf>*>* const CONFS_BUFFER; ///< Pointer to vector of pointers to vectors holding the data, fair enough for debug purposes.
            /// \brief Writes a frame.
            void write();
            /// \brief Writes the file header.
            /// Contains information about the cells from the cell list.
            void writeHeader();
    };


}
