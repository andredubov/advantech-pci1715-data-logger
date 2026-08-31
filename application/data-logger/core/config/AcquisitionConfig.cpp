#include "AcquisitionConfig.hpp"

namespace app {

AcquisitionConfig::AcquisitionConfig()
    : startChannel(0)
    , channelCount(1)
    , samplesPerChannel(10240)
    , samplingRate(500000.0)
    , inputMode("bipolar")
    , inputRange("10V")
{
}

} // namespace app
