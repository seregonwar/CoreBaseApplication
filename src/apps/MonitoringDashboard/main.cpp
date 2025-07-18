#define UNICODE
#define _UNICODE
#include "MonitoringDashboard.h"
#include <iostream>

int main() {
    try {
        CoreNS::Monitoring::MonitoringDashboard dashboard;
        dashboard.initialize();
        dashboard.run();
        dashboard.shutdown();
    } catch (const std::exception& ex) {
        std::cerr << "Errore nell'esecuzione della dashboard: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}