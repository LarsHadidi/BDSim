#include <blackBoxFactories/ForceFieldFactory.h>

namespace blackBoxFactories {

    boost::property_tree::iptree *ForceFieldFactory::initConf   = nullptr;
    datastructures::CellList  *ForceFieldFactory::cells       = nullptr;


    void ForceFieldFactory::initialize(boost::property_tree::iptree* initConf, datastructures::CellList *const cells) {
        ForceFieldFactory::initConf = initConf;
        ForceFieldFactory::cells = cells;
    }


    interfaces::ForceField* ForceFieldFactory::createWeeksChandlerAndersonForceField(){
        return new forcefields::WeeksChandlerAndersen(ForceFieldFactory::initConf, ForceFieldFactory::cells);
    }
}