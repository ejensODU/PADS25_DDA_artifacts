#include "Grid_VN2D_Arrive.h"
#include "Grid_VN2D_Depart.h"
#include "../Dist.h"
#include "../OoO_EventSet.h"
#include <thread>

extern void SpinLockData(std::atomic<int>& shared_lock);
extern void UnlockData(std::atomic<int>& shared_lock);

Grid_VN2D_Arrive::Grid_VN2D_Arrive(size_t networkNodeID, size_t gridSizeX, size_t gridSizeY,
                                 const std::string& vertexName, size_t distSeed,
                                 std::string traceFolderName, std::string traceFileHeading,
                                 size_t maxNumArriveEvents,
                                 double minIntraArrivalTime, double modeIntraArrivalTime, double maxIntraArrivalTime,
                                 double minServiceTime, double modeServiceTime, double maxServiceTime,
                                 OoO_SV<int>& packetQueueSV,
                                 std::queue<std::shared_ptr<Grid_VN2D_Packet>>& packetQueue,
                                 std::list<std::shared_ptr<Grid_VN2D_Packet>>& finishedPackets,
                                 std::atomic<int>& finishedPacketListLock)
    : Vertex(vertexName, 0, distSeed, traceFolderName, traceFileHeading),
      _networkNodeID(networkNodeID),
      _gridSizeX(gridSizeX), _gridSizeY(gridSizeY),
      _numIntraArriveEvents(0), _maxNumIntraArriveEvents(maxNumArriveEvents),
      _packetQueueSV(packetQueueSV), _packetQueue(packetQueue),
      _finishedPackets(finishedPackets), _finishedPacketListLock(finishedPacketListLock) {

    _randomNodeID = std::make_unique<UniformIntDist>(0, _gridSizeX*_gridSizeY-1, distSeed + networkNodeID);

    int high_activity_node = UniformIntDist(1, 6, distSeed + networkNodeID).GenRV();
    if (1 == high_activity_node) {
        minIntraArrivalTime /= 2;
        modeIntraArrivalTime /= 2;
        maxIntraArrivalTime /= 2;
    }
    _intraArrivalDelay = std::make_unique<TriangularDist>(minIntraArrivalTime, modeIntraArrivalTime, maxIntraArrivalTime, distSeed + networkNodeID);
    _serviceDelay = std::make_unique<TriangularDist>(minServiceTime, modeServiceTime, maxServiceTime, distSeed + networkNodeID);
}

size_t Grid_VN2D_Arrive::getNetworkNodeID() const {
    return _networkNodeID;
}

OoO_SV<int>& Grid_VN2D_Arrive::getPacketQueueSV() {
    return _packetQueueSV;
}

void Grid_VN2D_Arrive::AddDepartVertex(std::shared_ptr<Grid_VN2D_Depart> departVertex) {
    _departVertex = departVertex;
}

void Grid_VN2D_Arrive::IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) {
    std::vector<size_t> input_SV_indices;
    input_SV_indices.push_back(_packetQueueSV.getModelIndex());
    Is.push_back(input_SV_indices);

    std::vector<size_t> output_SV_indices;
    output_SV_indices.push_back(_packetQueueSV.getModelIndex());
    Os.push_back(output_SV_indices);
}

void Grid_VN2D_Arrive::Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) {
    std::shared_ptr<Grid_VN2D_Packet> packet = std::dynamic_pointer_cast<Grid_VN2D_Packet>(entity);

    // Evaluate Conditions
    _atDestination = false;
    if (nullptr == packet) {
        _intraArrival = true;
    } else {
        _intraArrival = false;
        if (_networkNodeID == packet->getDestNetworkNodeID()) {
            _atDestination = true;
        }
    }

    _serverAvailable = (_packetQueueSV.get() < 0);

    // Update SVs
    if (_intraArrival) {
        size_t dest_network_node_ID = _randomNodeID->GenRV();
        while (_networkNodeID == dest_network_node_ID) {
            dest_network_node_ID = _randomNodeID->GenRV();
        }
        packet = std::make_shared<Grid_VN2D_Packet>(simTime, _networkNodeID, dest_network_node_ID,
                                                dest_network_node_ID % _gridSizeX,
                                                dest_network_node_ID / _gridSizeX,
                                                _gridSizeX, _gridSizeY);
    }

    packet->AddNetworkNodeData(simTime, _networkNodeID);

    if (!_atDestination) {
        _packetQueueSV.inc();
        if (!_serverAvailable) {
            _packetQueue.push(packet);
        }
    } else {
        packet->setExitTime(simTime);
        SpinLockData(_finishedPacketListLock);
        _finishedPackets.push_back(packet);
        UnlockData(_finishedPacketListLock);
    }

    // Schedule New Events
    if (!_atDestination && _serverAvailable) {
        newEvents.push_back(new OoO_Event(_departVertex,
                                        simTime + _serviceDelay->GenRV(),
                                        packet));
    }

    if (_intraArrival && ++_numIntraArriveEvents < _maxNumIntraArriveEvents) {
        newEvents.push_back(new OoO_Event(shared_from_this(),
                                        simTime + _intraArrivalDelay->GenRV(),
                                        nullptr));
    }

    // Trace
    std::string trace_string = std::to_string(simTime) + ", " + std::to_string(_packetQueueSV.get());
    WriteToTrace(trace_string);

    _numExecutions++;
}
