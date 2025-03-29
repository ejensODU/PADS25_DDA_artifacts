#pragma once

#include "OoO_EventSet.h"

class OoO_SimExec {
public:
    // Constructor takes ITL table, simulation time limit, and OoO execution parameters
    OoO_SimExec(int numThreads, std::vector<std::vector<float>> ITL, double maxSimTime, int distSeed, int numSerialOoO_Execs);
    
    // Add an initial event to the event set
    void ScheduleInitEvent(OoO_Event* newEvent);
    
    // Run the serial simulation
    void RunSerialSim(std::string execOrderFilename);

private:
    bool _run;                                  // Flag to control simulation execution
    double _simTime;                            // Current simulation time
    std::atomic<int> _numEventsExecuted;        // Counter for executed events
    std::unique_ptr<OoO_EventSet> _ES;          // Event set containing all events
    int _distSeed;                              // Seed for random distributions
    int _numSerialOoO_Execs;                    // Controls OoO execution behavior
};