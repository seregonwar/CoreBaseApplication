#pragma once

#include <string>

namespace CoreNS {

struct SystemResources {
    double cpuUsagePercent;
    double availableMemoryBytes;
    double totalMemoryBytes;
    double availableDiskBytes;
    double totalDiskBytes;
    double networkUsagePercent;
    double gpuUsagePercent;
};

} // namespace CoreNS 