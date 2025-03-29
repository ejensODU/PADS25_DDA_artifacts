#pragma once

#include <memory>
#include <queue>
#include <list>
#include <atomic>
#include "../Vertex.h"
#include "../OoO_SV.h"
#include "Torus_3D_Packet.h"

class Torus_3D_Depart;

class Torus_3D_Arrive : public Vertex, public std::enable_shared_from_this<Torus_3D_Arrive> {
public:
    Torus_3D_Arrive(size_t networkNodeID,
                    size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                    const std::string& vertexName, size_t distSeed,
                    std::string traceFolderName, std::string traceFileHeading,
                    size_t maxNumArriveEvents,
                    double minIntraArrivalTime, double modeIntraArrivalTime, double maxIntraArrivalTime,
                    double minServiceTime, double modeServiceTime, double maxServiceTime,
                    OoO_SV<int>& packetQueueSV,
                    std::queue<std::shared_ptr<Torus_3D_Packet>>& packetQueue,
                    std::list<std::shared_ptr<Torus_3D_Packet>>& finishedPackets,
                    std::atomic<int>& finishedPacketListLock);

    size_t getNetworkNodeID() const;
    OoO_SV<int>& getPacketQueueSV();
    void AddDepartVertex(std::shared_ptr<Torus_3D_Depart> departVertex);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) override;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) override;

private:
    const size_t _networkNodeID;
    const size_t _gridSizeX;
    const size_t _gridSizeY;
    const size_t _gridSizeZ;
    std::unique_ptr<class UniformIntDist> _randomNodeID;
    int _numIntraArriveEvents;
    const int _maxNumIntraArriveEvents;
    bool _atDestination;
    bool _intraArrival;
    bool _serverAvailable;
    OoO_SV<int>& _packetQueueSV;
    std::queue<std::shared_ptr<Torus_3D_Packet>>& _packetQueue;
    std::shared_ptr<Torus_3D_Depart> _departVertex;
    std::unique_ptr<class TriangularDist> _intraArrivalDelay;
    std::unique_ptr<class TriangularDist> _serviceDelay;
    std::list<std::shared_ptr<Torus_3D_Packet>>& _finishedPackets;
    std::atomic<int>& _finishedPacketListLock;

    // Helper method for coordinate wrapping
    size_t WrapCoordinate(size_t coord, size_t size) const {
        return (coord + size) % size;
    }
};
