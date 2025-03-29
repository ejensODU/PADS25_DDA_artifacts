#pragma once

#include <vector>
#include <list>
#include <set>
#include <memory>
#include <atomic>

class Vertex;

struct EventRecord {
    size_t _sequenceNum;
    double _timestamp;
    std::string _eventType;
};

class Entity {
public:
    Entity(double genTime);
    void setExitTime(double exitTime);
    virtual ~Entity() = default;  // Virtual destructor for proper cleanup of derived classes
    virtual void PrintData() const = 0;
protected:
    const size_t _ID;             // Unique identifier
    const double _genTime;        // Generation time
    double _exitTime;             // Exit time from the system
private:
    static std::atomic<size_t> _entityCount; // Counter for generating unique IDs
};

class OoO_Event {
public:
    OoO_Event(std::shared_ptr<Vertex> vertex, double time, std::shared_ptr<Entity> entity);
    OoO_Event(const OoO_Event& other);
    
    // Execute this event
    void Execute();
    
    // Getters and setters
    std::list<OoO_Event*>& getNewEvents() { return _newEvents; }
    double getTime() const { return _time; }
    std::shared_ptr<Vertex> getVertex() { return _vertex; }
    int getVertexIndex() const;
    void setStatus(int status) { _status.store(status); }
    int getStatus() const { return _status.load(); }
    
private:
    std::shared_ptr<Vertex> _vertex;   // Vertex associated with this event
    const double _time;                // Time at which this event occurs
    std::shared_ptr<Entity> _entity;   // Entity associated with this event
    std::atomic<int> _status;          // Status of the event (0=idle, 1=ready, 2=executed)
    std::list<OoO_Event*> _newEvents;  // New events generated during execution
};

// Comparison functor for ordering events in the event set
struct EventPtr_Compare final
{
    bool operator() (const std::shared_ptr<OoO_Event> left, const std::shared_ptr<OoO_Event> right) const
    {
        if (left->getTime() < right->getTime()) return true;
        if (right->getTime() < left->getTime()) return false;

        return left->getVertexIndex() < right->getVertexIndex();
    }
};

class OoO_EventSet {
public:
    OoO_EventSet(std::vector<std::vector<float>> ITL, double maxSimTime);
    
    // Get all ready events from the event set
    void GetReadyEvents(std::list<std::shared_ptr<OoO_Event>>& readyEvents);
    
    // Update the event set after executing events
    bool UpdateEventSet(double& simTime);
    
    // Add a new event to the event set
    void AddEvent(OoO_Event* newEvent);
    
    // Query methods for the event set
    bool GetEmpty() const { return _E.empty(); }
    int GetSize() const { return _E.size(); }
    
    // Execute events serially in timestamp order
    void ExecuteSerial(double& simTime, std::atomic<int>& numEventsExecuted, std::string execOrderFilename);
    
    // Execute events serially but with out-of-order capabilities
    void ExecuteSerial_OoO(double& simTime, std::atomic<int>& numEventsExecuted, int distSeed, 
                         int numSerialOoO_Execs, std::string IO_ExecOrderFilename);
    
    // Get ready events for out-of-order serial execution
    void GetReadyEventsOoO_Serial(std::list<std::shared_ptr<OoO_Event>>& readyEvents, 
                                unsigned short& numReadyEvents, double& meanReadyEventIndex, 
                                double& stdReadyEventIndex, std::string& readyEventNames);
    
    // Count ready events for in-order serial execution
    void CountReadyEventsIO_Serial(unsigned short& numReadyEvents, double& meanReadyEventIndex, 
                                 double& stdReadyEventIndex, std::string& readyEventNames);
    
    // Print the event set
    void PrintE();
    
    // Get statistics about the execution
    double GetReadyEventsMeanSize();
    double GetE_SizesMeanSize();
    double GetE_RangesMeanRange();
    
    // Export statistics to CSV
    void WriteSerialReadyEventsToCSV();
    
private:
    std::multiset<std::shared_ptr<OoO_Event>, EventPtr_Compare> _E;     // Event set
    std::vector<std::vector<float>> _ITL;            // Independence Time Limit table
    std::list<OoO_Event*> _newEvents;                // New events generated during execution
    std::shared_ptr<OoO_Event> _eLater;              // Later event in ITL check
    std::shared_ptr<OoO_Event> _eEarlier;            // Earlier event in ITL check
    int _eeVertInd, _leVertInd;                      // Vertex indices for ITL check
    bool _leIndep;                                   // Flag for event independence
    double _eeLeLimit;                               // ITL limit
    const int _omega;                                // Maximum events to check in GetReadyEvents
    const double _maxSimTime;                        // Maximum simulation time
    
    // Statistics collection
    std::list<unsigned short> _readyEventsSizes;     // Sizes of ready event sets
    std::list<unsigned short> _E_Sizes;              // Sizes of event set
    std::list<unsigned short> _E_Ranges;             // Ranges of event timestamps
    
    // Serial execution statistics
    std::list<unsigned short> _numReadyEventsSerial; // Number of ready events in serial execution
    std::list<double> _readyEventIndexMeans;         // Mean event indices
    std::list<double> _readyEventIndexStds;          // Standard deviation of event indices
    std::list<std::string> _readyEventNames;         // Names of ready events
    std::vector<double> _maxTS_ByEventType;          // Maximum timestamps by event type
};