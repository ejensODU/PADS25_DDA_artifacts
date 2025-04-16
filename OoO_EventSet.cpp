#include "OoO_EventSet.h"
#include "OoO_SimModel.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <chrono>
#include <cmath>
#include <random>
#include <algorithm>
#include <memory>

// Initialize the static member
std::atomic<size_t> Entity::_entityCount{0};

Entity::Entity(double genTime)
: _ID(_entityCount.fetch_add(1)), _genTime(genTime)
{}

void Entity::setExitTime(double exitTime) { _exitTime = exitTime; }

OoO_Event::OoO_Event(std::shared_ptr<Vertex> vertex, double time, std::shared_ptr<Entity> entity)
: _vertex(vertex), _time(time), _entity(entity), _status(0)
{}

// Implementation of the copy constructor
OoO_Event::OoO_Event(const OoO_Event& other)
: _vertex(other._vertex),
  _time(other._time),
  _entity(other._entity),
  _status(other._status.load()),
  _newEvents(other._newEvents)
{}

void OoO_Event::Execute()
{
    _vertex->Run(_newEvents, _time, _entity);
}

int OoO_Event::getVertexIndex() const { return _vertex->getVertexIndex(); }

OoO_EventSet::OoO_EventSet(std::vector<std::vector<float>> ITL, double maxSimTime)
: _ITL(ITL), _maxSimTime(maxSimTime), _omega(32)
{
    _maxTS_ByEventType = std::vector<double>(3, 0);
}

void OoO_EventSet::AddEvent(OoO_Event* newEvent)
{
    std::shared_ptr<OoO_Event> sharedPtr(newEvent);
    _E.insert(std::move(sharedPtr));
}

void OoO_EventSet::GetReadyEvents(std::list<std::shared_ptr<OoO_Event>>& readyEvents)
{
    int i = 0;
    // Iterate through event set, from start to end
    for (auto later_it = _E.begin(); later_it != _E.end(); later_it++) {
        // Stop at omega if event set is too large
        if (i++ == _omega) break;
        
        // Get later event object
        _eLater = (*later_it);
        
        // Non-0 means ready or completed (atomic)
        if (0 != _eLater->getStatus()) {
            continue;
        }

        // Check if later event is independent
        _leIndep = true;
        _leVertInd = _eLater->getVertexIndex();
        
        // Iterate through event set, from beginning to before later event
        for (auto earlier_it = _E.begin(); earlier_it != later_it; earlier_it++) {
            // Get earlier event object
            _eEarlier = (*earlier_it);
            _eeVertInd = _eEarlier->getVertexIndex();
            
            // Get ITL-table limit of event pair
            _eeLeLimit = static_cast<double>(_ITL[_eeVertInd][_leVertInd]);
            
            // If event pair is not independent, later event is not independent in event set
            if (_eLater->getTime() - _eEarlier->getTime() >= _eeLeLimit) {
                _leIndep = false;
                break;
            }
        }
        
        // If later event is independent
        if (_leIndep) {
            // Mark event as ready (atomic)
            _eLater->setStatus(1);
            
            // Add to ready events list
            readyEvents.push_back(_eLater);
        }
    }
    
    // Update statistics if needed
    if (!readyEvents.empty()) {
        _readyEventsSizes.push_back(readyEvents.size());
    }
    
    return;  
}

bool OoO_EventSet::UpdateEventSet(double& simTime)
{
    // Iterate through the event set
    for (auto it = _E.begin(); it != _E.end(); ) {
        // Event executed, ready for removal (atomic)
        if (2 == (*it)->getStatus()) {
            // Get new events from executed event
            _newEvents = (*it)->getNewEvents();
            
            // Schedule new events in event set
            for (auto& eventPtr : _newEvents) {
                std::shared_ptr<OoO_Event> sharedPtr(eventPtr);
                _E.insert(std::move(sharedPtr));
            }
            
            // Empty new events container
            _newEvents.clear();
            
            // Update simulation-clock time
            if ((*it)->getTime() > simTime) {
                simTime = (*it)->getTime();
            }
            
            // Remove executed event
            it = _E.erase(it);
        }
        // If not removing event, increment iterator
        else {
            ++it;
        }
    }

    // Update statistics
    if (!_E.empty()) {
        _E_Sizes.push_back(_E.size());
        _E_Ranges.push_back((*_E.rbegin())->getTime() - (*_E.begin())->getTime());
    }

    // Simulation will terminate if event set is empty or max time reached
    return !_E.empty() && simTime <= _maxSimTime;
}

void OoO_EventSet::ExecuteSerial_IO(double& simTime, std::atomic<int>& numEventsExecuted, std::string IO_ExecOrderFilename)
{
	// Serial IO execution order
    std::ofstream IO_exec_order_file;
    if (!IO_ExecOrderFilename.empty()) {
	    IO_exec_order_file.open(IO_ExecOrderFilename);
        IO_exec_order_file << "event_sequence_num, timestamp, event_type" << std::endl;
    }
	
    // Continue until event set is empty or max time is reached
    while (!_E.empty() && (*_E.begin())->getTime() <= _maxSimTime) {
        // Update statistics
        _E_Sizes.push_back(_E.size());
        
        // Get the first event (earliest timestamp)
        std::shared_ptr<OoO_Event> first_event = *_E.begin();
        
        // Execute the event
        first_event->Execute();
        
        // Add new events to the event set
        for (auto& eventPtr : first_event->getNewEvents()) {
            std::shared_ptr<OoO_Event> sharedPtr(eventPtr);
            _E.insert(std::move(sharedPtr));
        }
        
        // Update simulation time and event counter
        simTime = first_event->getTime();
        size_t event_index = numEventsExecuted.fetch_add(1);
		
		// Serial IO execution order
        if (!IO_ExecOrderFilename.empty()) {
		    IO_exec_order_file << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
			<< event_index << ", " << simTime << ", "
			<< first_event->getVertex()->getVertexName() << std::endl;
        }
        
        // Remove the executed event
        _E.erase(_E.begin());
        
        // Update statistics
        if (!_E.empty()) {
            _E_Sizes.push_back(_E.size());
            _E_Ranges.push_back((*_E.rbegin())->getTime() - (*_E.begin())->getTime());
        }
    }
    
    return;
}

void OoO_EventSet::ExecuteSerial_OoO(double& simTime, std::atomic<int>& numEventsExecuted, int distSeed, int numSerialOoO_Execs, std::string IO_ExecOrderFilename)
{
    std::list<std::shared_ptr<OoO_Event>> ready_events;
    unsigned short num_ready_events;
    double mean_ready_event_index;
    double std_ready_event_index;
	
	// Serial OoO execution order
    size_t num_event_matches = 0;
    std::vector<size_t> index_diffs;
    std::vector<EventRecord> IO_events;
    if (!IO_ExecOrderFilename.empty()) {
        std::ifstream IO_exec_order_file(IO_ExecOrderFilename);
        std::string line;
        std::getline(IO_exec_order_file, line); // header
        while (std::getline(IO_exec_order_file, line)) {
            std::stringstream ss(line);
            std::string token;
            // Parse sequence number
            std::getline(ss, token, ',');
            size_t seq = std::stoi(token);
            // Parse timestamp
            std::getline(ss, token, ',');
            double timestamp = std::stod(token);
            // Parse event type
            std::getline(ss, token, ',');
            // Remove leading/trailing whitespace
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            IO_events.emplace_back(seq, timestamp, token);
        }
    }

    // Create file for ready event sets
    //std::ofstream RE_sets("RE_sets.txt");
    
    // Initialize random number generator
    std::mt19937 rng(distSeed);

    // Continue until event set is empty or max time is reached
    while (!_E.empty() && (*_E.begin())->getTime() <= _maxSimTime) {
        // Clear ready events and get new ones
        ready_events.clear();
        std::string ready_event_names;
        GetReadyEventsOoO_Serial(ready_events, num_ready_events, mean_ready_event_index, 
                              std_ready_event_index, ready_event_names);
        
        // Update statistics
        _readyEventsSizes.push_back(num_ready_events);
        _numReadyEventsSerial.push_back(num_ready_events);
        _readyEventIndexMeans.push_back(mean_ready_event_index);
        _readyEventIndexStds.push_back(std_ready_event_index);
        _readyEventNames.push_back(ready_event_names);
        _E_Sizes.push_back(_E.size());

        // Handle different OoO execution modes
        if (numSerialOoO_Execs > 0) {
            // Execute a power-of-2 number of events
            int num_events_exec = 0;
            for (std::shared_ptr<OoO_Event>& event : ready_events) {
                event->Execute();
                event->setStatus(2);
                size_t event_count = numEventsExecuted.fetch_add(1);

                if (++num_events_exec == std::pow(2, numSerialOoO_Execs)) break;
            }
        } else {
            // Execute a percentage of random events
            double percentage = -numSerialOoO_Execs * 10.0;
            int num_random_events = ceil((num_ready_events * percentage) / 100.0);

            // Convert list to vector for random selection
            std::vector<std::shared_ptr<OoO_Event>> ready_events_vector(ready_events.begin(), ready_events.end());
            std::shuffle(ready_events_vector.begin(), ready_events_vector.end(), rng);

            // Execute the selected events
            for (int i = 0; i < num_random_events && i < ready_events_vector.size(); ++i) {
                ready_events_vector[i]->Execute();
                ready_events_vector[i]->setStatus(2);

                // Record execution for analysis
                double timestamp = ready_events_vector[i]->getTime();
                std::string vertex_name = ready_events_vector[i]->getVertex()->getVertexName();
                //RE_sets << "(" << vertex_name << "," << timestamp << ")" << ",";
				
				// Serial OoO execution order
                if (!IO_ExecOrderFilename.empty()) {
                    size_t event_index = numEventsExecuted.fetch_add(1);
                    if (IO_events.at(event_index)._timestamp == timestamp && IO_events.at(event_index)._eventType == vertex_name) num_event_matches++;
                    for (const auto& event_record : IO_events) {
                        if (event_record._timestamp == timestamp && event_record._eventType == vertex_name) index_diffs.push_back(abs(event_index-event_record._sequenceNum));
                    }
                }
				
            }
            //RE_sets << std::endl;
        }
        
        // Update the event set
        UpdateEventSet(simTime);
    }
	
	// Serial OoO execution order
    if (!IO_ExecOrderFilename.empty()) {
        double sum_diffs = std::accumulate(index_diffs.begin(), index_diffs.end(), 0.0);
        double mean_diffs = sum_diffs / index_diffs.size();
        double square_sum_diffs = std::inner_product(
            index_diffs.begin(), index_diffs.end(),
                                                    index_diffs.begin(), 0.0,
                                                    std::plus<>(),
                                                    [mean_diffs](size_t a, size_t b) { return (a - mean_diffs) * (b - mean_diffs); }
        );
        double std_diffs = std::sqrt(square_sum_diffs / index_diffs.size());
        std::ofstream match_count_file(IO_ExecOrderFilename);
        match_count_file << "num_event_matches, mean_diffs, std_diffs" << std::endl;
        match_count_file << num_event_matches << ", " << mean_diffs << ", " << std_diffs << std::endl;
    }
	
    return;
}

void OoO_EventSet::GetReadyEventsOoO_Serial(std::list<std::shared_ptr<OoO_Event>>& readyEvents, 
                                         unsigned short& numReadyEvents, double& meanReadyEventIndex, 
                                         double& stdReadyEventIndex, std::string& readyEventNames)
{
    // Open file to record event sets
    //std::ofstream E_sets("E_sets.txt", std::ios::app);

    numReadyEvents = 0;
    std::list<int> ready_event_indices;
    int i = 0;
    
    // Iterate through event set
    for (auto later_it = _E.begin(); later_it != _E.end(); later_it++) {
        // Get later event
        _eLater = (*later_it);
        _leIndep = true;
        _leVertInd = _eLater->getVertexIndex();

        // Record event info
        //double timestamp = _eLater->getTime();
        //std::string vertex_name = _eLater->getVertex()->getVertexName();
        //E_sets << "(" << vertex_name << "," << timestamp << ")" << ",";

        // Check event independence against all earlier events
        for (auto earlier_it = _E.begin(); earlier_it != later_it; earlier_it++) {
            _eEarlier = (*earlier_it);
            _eeVertInd = _eEarlier->getVertexIndex();
            _eeLeLimit = static_cast<double>(_ITL[_eeVertInd][_leVertInd]);
            
            // Check if event is dependent
            if (_eLater->getTime() - _eEarlier->getTime() >= _eeLeLimit) {
                _leIndep = false;
                break;
            }
        }
        
        // Process independent events
        if (_leIndep) {
            // Mark as ready
            _eLater->setStatus(1);
            
            // Add to ready events
            readyEvents.push_back(_eLater);
            numReadyEvents++;
            ready_event_indices.push_back(i);
        }
        i++;
    }
    //E_sets << std::endl;

    // Calculate statistics
    if (!ready_event_indices.empty()) {
        meanReadyEventIndex = std::accumulate(ready_event_indices.begin(), ready_event_indices.end(), 0.0) 
                            / ready_event_indices.size();

        double variance = std::accumulate(ready_event_indices.begin(), ready_event_indices.end(), 0.0, 
                                      [meanReadyEventIndex](double sum, unsigned short value) { 
                                          return sum + std::pow(value - meanReadyEventIndex, 2);
                                      }) / ready_event_indices.size();
        stdReadyEventIndex = std::sqrt(variance);
    } else {
        meanReadyEventIndex = 0;
        stdReadyEventIndex = 0;
    }

    readyEventNames.append(" ");

    return;
}

void OoO_EventSet::CountReadyEventsIO_Serial(unsigned short& numReadyEvents, double& meanReadyEventIndex, 
                                          double& stdReadyEventIndex, std::string& readyEventNames)
{
    numReadyEvents = 0;
    std::list<int> ready_event_indices;
    int i = 0;
    
    // Iterate through event set to find independent events
    for (auto later_it = _E.begin(); later_it != _E.end(); later_it++) {
        _eLater = (*later_it);
        _leIndep = true;
        _leVertInd = _eLater->getVertexIndex();

        // Update event type statistics
        if (_eLater->getTime() > _maxTS_ByEventType.at(_leVertInd % 3)) {
            _maxTS_ByEventType.at(_leVertInd % 3) = _eLater->getTime();
        }

        // Check independence against all earlier events
        for (auto earlier_it = _E.begin(); earlier_it != later_it; earlier_it++) {
            _eEarlier = (*earlier_it);
            _eeVertInd = _eEarlier->getVertexIndex();
            _eeLeLimit = static_cast<double>(_ITL[_eeVertInd][_leVertInd]);
            
            if (_eLater->getTime() - _eEarlier->getTime() >= _eeLeLimit) {
                _leIndep = false;
                break;
            }
        }
        
        // Count independent events
        if (_leIndep) {
            numReadyEvents++;
            ready_event_indices.push_back(i);
        }
        i++;
    }
    
    // Calculate statistics
    if (!ready_event_indices.empty()) {
        meanReadyEventIndex = std::accumulate(ready_event_indices.begin(), ready_event_indices.end(), 0.0) 
                            / ready_event_indices.size();

        double variance = std::accumulate(ready_event_indices.begin(), ready_event_indices.end(), 0.0, 
                                      [meanReadyEventIndex](double sum, unsigned short value) { 
                                          return sum + std::pow(value - meanReadyEventIndex, 2);
                                      }) / ready_event_indices.size();
        stdReadyEventIndex = std::sqrt(variance);
    } else {
        meanReadyEventIndex = 0;
        stdReadyEventIndex = 0;
    }

    readyEventNames.append(" ");

    return;
}

void OoO_EventSet::PrintE()
{
    int i = 1;
    for (auto E_event : _E) {
        int vertex_index = E_event->getVertexIndex();
        std::string vertex_name = E_event->getVertex()->getVertexName();
        double event_time = E_event->getTime();

        printf("\tE (%d) vertex name: %s, vertex index: %d, time: %lf\n", i++,
              vertex_name.c_str(), vertex_index, event_time);
    }
}

double OoO_EventSet::GetReadyEventsMeanSize()
{
    return _readyEventsSizes.empty() ? 0 : 
           std::accumulate(_readyEventsSizes.begin(), _readyEventsSizes.end(), 0.0) / _readyEventsSizes.size();
}

double OoO_EventSet::GetE_SizesMeanSize()
{
    return _E_Sizes.empty() ? 0 :
           std::accumulate(_E_Sizes.begin(), _E_Sizes.end(), 0.0) / _E_Sizes.size();
}

double OoO_EventSet::GetE_RangesMeanRange()
{
    return _E_Ranges.empty() ? 0 :
           std::accumulate(_E_Ranges.begin(), _E_Ranges.end(), 0.0) / _E_Ranges.size();
}

void OoO_EventSet::WriteSerialReadyEventsToCSV()
{
    // Open the output file stream
    std::string filename = "serial_ready_events.csv";
    std::ofstream outFile(filename);

    // Check if the file is open
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open file " + filename);
    }

    // Write the header
    outFile << "NumReadyEventsSerial,ReadyEventIndexMeans,ReadyEventIndexStds,ReadyEventNames,E_Sizes\n";

    // Create iterators for each list
    auto itNumReadyEvents = _numReadyEventsSerial.begin();
    auto itReadyEventIndexMeans = _readyEventIndexMeans.begin();
    auto itReadyEventIndexStds = _readyEventIndexStds.begin();
    auto itReadyEventNames = _readyEventNames.begin();
    auto itESizes = _E_Sizes.begin();

    // Iterate through the lists and write each row to the CSV file
    while (itNumReadyEvents != _numReadyEventsSerial.end() &&
           itReadyEventIndexMeans != _readyEventIndexMeans.end() &&
           itReadyEventIndexStds != _readyEventIndexStds.end() &&
           itReadyEventNames != _readyEventNames.end() &&
           itESizes != _E_Sizes.end()) {
        
        outFile << *itNumReadyEvents << ',' << *itReadyEventIndexMeans << ',' 
              << *itReadyEventIndexStds << ',' << *itReadyEventNames << ',' 
              << *itESizes << '\n';

        // Increment the iterators
        ++itNumReadyEvents;
        ++itReadyEventIndexMeans;
        ++itReadyEventIndexStds;
        ++itReadyEventNames;
        ++itESizes;
    }

    // Close the output file stream
    outFile.close();
}