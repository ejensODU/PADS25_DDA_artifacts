#include "OoO_SimExec.h"

#include <chrono>
#include <iostream>
#include <fstream>

OoO_SimExec::OoO_SimExec(int numThreads, std::vector<std::vector<float>> ITL, double maxSimTime, int distSeed, int numSerialOoO_Execs)
: _run(true), _simTime(0), _numEventsExecuted(0), _distSeed(distSeed), _numSerialOoO_Execs(numSerialOoO_Execs)
{
    // Initialize the event set with the ITL table and maximum simulation time
    _ES = std::make_unique<OoO_EventSet>(ITL, maxSimTime);
}

void OoO_SimExec::ScheduleInitEvent(OoO_Event* newEvent)
{
    _ES->AddEvent(newEvent);
}

void OoO_SimExec::RunSerialSim(std::string execOrderFilename)
{
    std::cout << "serial sim: OoO_SimExec " << _numSerialOoO_Execs << std::endl;

    if (0 == _numSerialOoO_Execs) {
        // Regular in-order serial execution
        auto start = std::chrono::high_resolution_clock::now();
        _ES->ExecuteSerial_IO(_simTime, _numEventsExecuted, execOrderFilename);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        printf("in-order SIMULATION FINISHED\n");
        printf("in-order time %lf, in-order events executed %d, event set (%d):\n", 
              _simTime, _numEventsExecuted.load(), _ES->GetSize());
        printf("SERIAL in-order runtime: %lf, num in-order events executed: %d, mean size ready events: %lf, mean E size: %lf, mean E range: %lf\n", 
              duration.count()/1e6, _numEventsExecuted.load(), _ES->GetReadyEventsMeanSize(), 
              _ES->GetE_SizesMeanSize(), _ES->GetE_RangesMeanRange());
    }
    else {
        // Out-of-order serial execution
        auto start_OoO = std::chrono::high_resolution_clock::now();
        _ES->ExecuteSerial_OoO(_simTime, _numEventsExecuted, _distSeed, _numSerialOoO_Execs, execOrderFilename);

        auto stop_OoO = std::chrono::high_resolution_clock::now();
        auto duration_OoO = std::chrono::duration_cast<std::chrono::microseconds>(stop_OoO - start_OoO);

        printf("OoO SIMULATION FINISHED\n");
        printf("OoO time %lf, events executed %d, event set (%d):\n", 
              _simTime, _numEventsExecuted.load(), _ES->GetSize());
        printf("SERIAL OoO runtime: %lf, num OoO events executed: %d, mean size ready events: %lf, mean E size: %lf, mean E range: %lf\n", 
              duration_OoO.count()/1e6, _numEventsExecuted.load(), _ES->GetReadyEventsMeanSize(), 
              _ES->GetE_SizesMeanSize(), _ES->GetE_RangesMeanRange());
    }
}