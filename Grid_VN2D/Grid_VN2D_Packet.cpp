#include "Grid_VN2D_Packet.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <limits>

Grid_VN2D_Packet::Grid_VN2D_Packet(double genTime, size_t originNetworkNodeID, size_t destNetworkNodeID,
                                  size_t destX, size_t destY, size_t gridSizeX, size_t gridSizeY)
    : Entity(genTime),
      _originNetworkNodeID(originNetworkNodeID),
      _destNetworkNodeID(destNetworkNodeID),
      _destX(destX), _destY(destY),
      _gridSizeX(gridSizeX), _gridSizeY(gridSizeY) {
    _minManhattanDist = std::numeric_limits<size_t>::max();
}

void Grid_VN2D_Packet::AddNetworkNodeData(double arrivalTime, size_t networkNodeID) {
    if (std::find(_visitedNetworkNodes.begin(), _visitedNetworkNodes.end(), networkNodeID) != _visitedNetworkNodes.end()) {
        printf("WARNING: Packet %lu revisiting node %lu\n", _ID, networkNodeID);
    }

    size_t node_x = networkNodeID % _gridSizeX;
    size_t node_y = networkNodeID / _gridSizeX;

    size_t dest_distance = abs(static_cast<int>(_destX) - static_cast<int>(node_x)) +
                          abs(static_cast<int>(_destY) - static_cast<int>(node_y));

    if (dest_distance >= _minManhattanDist) {
        printf("error: packet manhattan distance not decreased!\n");
        printf("\tnode ID: %lu, node x: %lu, node y: %lu\n", networkNodeID, node_x, node_y);
        printf("\tmin MH dist: %lu, dest MH dist: %lu\n", _minManhattanDist, dest_distance);
        PrintData();
        exit(1);
    }
    _minManhattanDist = dest_distance;

    _networkNodeArrivalTimes.push_back(arrivalTime);
    _visitedNetworkNodes.push_back(networkNodeID);
}

const std::list<size_t>& Grid_VN2D_Packet::getVisitedNetworkNodes() const {
    return _visitedNetworkNodes;
}

size_t Grid_VN2D_Packet::getDestNetworkNodeID() const {
    return _destNetworkNodeID;
}

double Grid_VN2D_Packet::GetTimeInNetwork() const {
    return _exitTime - _genTime;
}

void Grid_VN2D_Packet::PrintData() const {
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
