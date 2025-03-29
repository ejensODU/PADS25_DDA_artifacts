#include "Ring_1D_Packet.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <limits>

Ring_1D_Packet::Ring_1D_Packet(double genTime, size_t originNodeID, size_t destNodeID,
                               size_t ringSize, bool clockwise)
: Entity(genTime),
_originNodeID(originNodeID),
_destNodeID(destNodeID),
_ringSize(ringSize),
_clockwise(clockwise) {
}

void Ring_1D_Packet::AddNodeData(double arrivalTime, size_t networkNodeID) {
    if (std::find(_visitedNodes.begin(), _visitedNodes.end(), networkNodeID) != _visitedNodes.end()) {
        printf("WARNING: Packet %lu revisiting node %lu\n", _ID, networkNodeID);
    }

    // For clockwise direction: Check if we've passed our starting point
    bool crossed_zero = false;
    if (!_visitedNodes.empty()) {
        size_t last_node = _visitedNodes.back();
        if (_clockwise && last_node > networkNodeID) {
            crossed_zero = true;  // We've wrapped from high to low
        } else if (!_clockwise && last_node < networkNodeID) {
            crossed_zero = true;  // We've wrapped from low to high
        }
    }

    // Don't do distance checking if we've crossed the zero point
    if (!crossed_zero && !_visitedNodes.empty()) {
        size_t last_node = _visitedNodes.back();
        bool making_progress = false;

        if (_clockwise) {
            // For clockwise: check if we're moving forward without wrapping
            if (networkNodeID > last_node &&
                (networkNodeID <= _destNodeID || last_node >= _destNodeID)) {
                making_progress = true;
                }
        } else {
            // For counterclockwise: check if we're moving backward without wrapping
            if (networkNodeID < last_node &&
                (networkNodeID >= _destNodeID || last_node <= _destNodeID)) {
                making_progress = true;
                }
        }

        if (!making_progress) {
            printf("error: packet not making progress towards destination!\n");
            printf("\tnode ID: %lu, direction: %s\n", networkNodeID,
                   _clockwise ? "clockwise" : "counterclockwise");
            printf("\tlast node: %lu, destination: %lu\n", last_node, _destNodeID);
            PrintData();
            exit(1);
        }
    }

    _nodeArrivalTimes.push_back(arrivalTime);
    _visitedNodes.push_back(networkNodeID);
}

const std::list<size_t>& Ring_1D_Packet::getVisitedNodes() const {
    return _visitedNodes;
}

size_t Ring_1D_Packet::getDestNodeID() const {
    return _destNodeID;
}

double Ring_1D_Packet::GetTimeInNetwork() const {
    return _exitTime - _genTime;
}

void Ring_1D_Packet::PrintData() const {
    std::string arrival_times;
    for (auto arrival_time : _nodeArrivalTimes) {
        arrival_times.append(std::to_string(arrival_time) + ",");
    }

    std::string traversed_nodes;
    for (auto node : _visitedNodes) {
        traversed_nodes.append(std::to_string(node) + ",");
    }

    printf("ID: %lu, gen t: %lf, o: %lu, d: %lu, direction: %s, arrival ts: %s, nodes: %s\n",
           _ID, _genTime, _originNodeID, _destNodeID,
           _clockwise ? "clockwise" : "counterclockwise",
           arrival_times.c_str(), traversed_nodes.c_str());
}
