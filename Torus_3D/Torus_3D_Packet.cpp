#include "Torus_3D_Packet.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <limits>

Torus_3D_Packet::Torus_3D_Packet(double genTime, size_t originNetworkNodeID, size_t destNetworkNodeID,
               size_t destX, size_t destY, size_t destZ,
               size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ)
    : Entity(genTime),
      _originNetworkNodeID(originNetworkNodeID),
      _destNetworkNodeID(destNetworkNodeID),
      _destX(destX), _destY(destY), _destZ(destZ),
      _gridSizeX(gridSizeX), _gridSizeY(gridSizeY), _gridSizeZ(gridSizeZ) {
    _minWrappedDist = std::numeric_limits<size_t>::max();
}

void Torus_3D_Packet::AddNetworkNodeData(double arrivalTime, size_t networkNodeID) {
    if (std::find(_visitedNetworkNodes.begin(), _visitedNetworkNodes.end(), networkNodeID) != _visitedNetworkNodes.end()) {
        printf("WARNING: Packet %lu revisiting node %lu\n", _ID, networkNodeID);
    }

    size_t node_x = networkNodeID % _gridSizeX;
    size_t node_y = (networkNodeID / _gridSizeX) % _gridSizeY;
    size_t node_z = networkNodeID / (_gridSizeX * _gridSizeY);

    // Calculate wrapped distance to destination
    size_t dest_distance = getWrappedDistance(static_cast<int>(node_x), static_cast<int>(_destX), _gridSizeX) +
                          getWrappedDistance(static_cast<int>(node_y), static_cast<int>(_destY), _gridSizeY) +
                          getWrappedDistance(static_cast<int>(node_z), static_cast<int>(_destZ), _gridSizeZ);

    if (dest_distance >= _minWrappedDist) {
        printf("error: packet wrapped distance not decreased!\n");
        printf("\tnode ID: %lu, node x: %lu, node y: %lu, node z: %lu\n",
               networkNodeID, node_x, node_y, node_z);
        printf("\tmin wrapped dist: %lu, dest wrapped dist: %lu\n", _minWrappedDist, dest_distance);
        PrintData();
        exit(1);
    }
    _minWrappedDist = dest_distance;

    _networkNodeArrivalTimes.push_back(arrivalTime);
    _visitedNetworkNodes.push_back(networkNodeID);
}

const std::list<size_t>& Torus_3D_Packet::getVisitedNetworkNodes() const {
    return _visitedNetworkNodes;
}

size_t Torus_3D_Packet::getDestNetworkNodeID() const {
    return _destNetworkNodeID;
}

double Torus_3D_Packet::GetTimeInNetwork() const {
    return _exitTime - _genTime;
}

void Torus_3D_Packet::PrintData() const {
    std::string arrival_times;
    for (auto arrival_time : _networkNodeArrivalTimes) {
        arrival_times.append(std::to_string(arrival_time) + ",");
    }

    std::string traversed_nodes;
    for (auto node : _visitedNetworkNodes) {
        traversed_nodes.append(std::to_string(node) + ",");
    }

    printf("ID: %lu, gen t: %lf, o: %lu, d: %lu, arrival ts: %s, nodes: %s\n",
           _ID, _genTime, _originNetworkNodeID, _destNetworkNodeID,
           arrival_times.c_str(), traversed_nodes.c_str());
}
