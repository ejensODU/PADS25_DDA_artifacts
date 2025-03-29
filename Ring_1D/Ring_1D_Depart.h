#pragma once

#include <memory>
#include <queue>
#include <vector>
#include "../Vertex.h"
#include "../OoO_SV.h"
#include "Ring_1D_Packet.h"

// Forward declaration
class Ring_1D_Arrive;

class Ring_1D_Depart : public Vertex, public std::enable_shared_from_this<Ring_1D_Depart> {
public:
    Ring_1D_Depart(size_t networkNodeID,
                   std::vector<bool> neighbors,
                   size_t ringSize,
                   const std::string& vertexName,
                   size_t distSeed, std::string traceFolderName,
                   std::string traceFileHeading,
                   double minServiceTime, double modeServiceTime, double maxServiceTime,
                   double minTransitTime, double modeTransitTime, double maxTransitTime,
                   OoO_SV<int>& packetQueueSV,
                   std::queue<std::shared_ptr<Ring_1D_Packet>>& packetQueue);

    void AddArriveVertices(std::vector<std::shared_ptr<Ring_1D_Arrive>> arriveVertices);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) override;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) override;

private:
    const size_t _networkNodeID;
    const size_t _ringSize;
    std::vector<bool> _neighbors;  // Two neighbors: clockwise and counterclockwise
    bool _packetInQueue;
    OoO_SV<int>& _packetQueueSV;
    std::queue<std::shared_ptr<Ring_1D_Packet>>& _packetQueue;
    std::vector<std::shared_ptr<Ring_1D_Arrive>> _arriveVertices;
    std::unique_ptr<class TriangularDist> _serviceDelay;
    std::unique_ptr<class TriangularDist> _transitDelay;

    // Helper method for determining routing direction
    bool shouldRouteClockwise(size_t destNodeID) const {
        size_t clockwise_dist = (destNodeID >= _networkNodeID) ?
        destNodeID - _networkNodeID :
        _ringSize - (_networkNodeID - destNodeID);
        size_t counter_dist = (_networkNodeID >= destNodeID) ?
        _networkNodeID - destNodeID :
        _ringSize - (destNodeID - _networkNodeID);
        return clockwise_dist <= counter_dist;
    }
};
