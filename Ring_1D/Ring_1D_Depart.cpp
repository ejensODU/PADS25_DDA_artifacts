#include "Ring_1D_Depart.h"
#include "Ring_1D_Arrive.h"
#include "../Dist.h"
#include "../OoO_EventSet.h"
#include <algorithm>

Ring_1D_Depart::Ring_1D_Depart(size_t networkNodeID,
                               std::vector<bool> neighbors,
                               size_t ringSize,
                               const std::string& vertexName,
                               size_t distSeed, std::string traceFolderName,
                               std::string traceFileHeading,
                               double minServiceTime, double modeServiceTime, double maxServiceTime,
                               double minTransitTime, double modeTransitTime, double maxTransitTime,
                               OoO_SV<int>& packetQueueSV,
                               std::queue<std::shared_ptr<Ring_1D_Packet>>& packetQueue)
: Vertex(vertexName, 0, distSeed, traceFolderName, traceFileHeading),
_networkNodeID(networkNodeID),
_ringSize(ringSize),
_neighbors(neighbors),
_packetQueueSV(packetQueueSV), _packetQueue(packetQueue) {

    _serviceDelay = std::make_unique<TriangularDist>(minServiceTime, modeServiceTime, maxServiceTime, distSeed + networkNodeID);
    _transitDelay = std::make_unique<TriangularDist>(minTransitTime, modeTransitTime, maxTransitTime, distSeed + networkNodeID);
}

void Ring_1D_Depart::AddArriveVertices(std::vector<std::shared_ptr<Ring_1D_Arrive>> arriveVertices) {
    _arriveVertices = arriveVertices;
}

void Ring_1D_Depart::IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) {
    std::vector<size_t> input_SV_indices;
    input_SV_indices.push_back(_packetQueueSV.getModelIndex());
    Is.push_back(input_SV_indices);

    std::vector<size_t> output_SV_indices;
    output_SV_indices.push_back(_packetQueueSV.getModelIndex());
    Os.push_back(output_SV_indices);
}

void Ring_1D_Depart::Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) {
    std::shared_ptr<Ring_1D_Packet> packet = std::dynamic_pointer_cast<Ring_1D_Packet>(entity);

    // Evaluate Conditions
    _packetInQueue = (_packetQueueSV.get() > 0);

    // Update SVs
    std::shared_ptr<Ring_1D_Packet> queue_packet;
    _packetQueueSV.dec();
    if (_packetInQueue) {
        queue_packet = std::move(_packetQueue.front());
        _packetQueue.pop();
    }

    // Use packet's stored direction
    int dest_dir = packet->isClockwise() ? 0 : 1;  // 0 for clockwise, 1 for counterclockwise

    // Schedule New Events
    if (_packetInQueue) {
        newEvents.push_back(new OoO_Event(shared_from_this(),
                                          simTime + _serviceDelay->GenRV(),
                                          queue_packet));
    }

    newEvents.push_back(new OoO_Event(_arriveVertices.at(dest_dir),
                                      simTime + _transitDelay->GenRV(),
                                      packet));

    // Trace
    std::string trace_string = std::to_string(simTime) + ", " + std::to_string(_packetQueueSV.get());
    WriteToTrace(trace_string);

    _numExecutions++;
}
