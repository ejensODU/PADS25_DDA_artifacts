#pragma once

#include <memory>
#include <queue>
#include <list>
#include <atomic>
#include "../Vertex.h"
#include "../OoO_SV.h"
#include "Ring_1D_Packet.h"

class Ring_1D_Depart;

class Ring_1D_Arrive : public Vertex, public std::enable_shared_from_this<Ring_1D_Arrive> {
public:
    Ring_1D_Arrive(size_t networkNodeID,
                 size_t ringSize,
                 const std::string& vertexName, size_t distSeed,
                 std::string traceFolderName, std::string traceFileHeading,
                 size_t maxNumArriveEvents,
                 double minIntraArrivalTime, double modeIntraArrivalTime, double maxIntraArrivalTime,
                 double minServiceTime, double modeServiceTime, double maxServiceTime,
                 OoO_SV<int>& packetQueueSV,
                 std::queue<std::shared_ptr<Ring_1D_Packet>>& packetQueue,
                 std::list<std::shared_ptr<Ring_1D_Packet>>& finishedPackets,
                 std::atomic<int>& finishedPacketListLock);

    size_t getNetworkNodeID() const;
    OoO_SV<int>& getPacketQueueSV();
    void AddDepartVertex(std::shared_ptr<Ring_1D_Depart> departVertex);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) override;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) override;

private:
    const size_t _networkNodeID;
    const size_t _ringSize;
    std::unique_ptr<class UniformIntDist> _randomNodeID;
    int _numIntraArriveEvents;
    const int _maxNumIntraArriveEvents;
    bool _atDestination;
    bool _intraArrival;
    bool _serverAvailable;
    OoO_SV<int>& _packetQueueSV;
    std::queue<std::shared_ptr<Ring_1D_Packet>>& _packetQueue;
    std::shared_ptr<Ring_1D_Depart> _departVertex;
    std::unique_ptr<class TriangularDist> _intraArrivalDelay;
    std::unique_ptr<class TriangularDist> _serviceDelay;
    std::list<std::shared_ptr<Ring_1D_Packet>>& _finishedPackets;
    std::atomic<int>& _finishedPacketListLock;
};
