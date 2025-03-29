#pragma once

#include <list>
#include "../OoO_EventSet.h"

class Ring_1D_Packet : public Entity {
public:
    Ring_1D_Packet(double genTime, size_t originNodeID, size_t destNodeID,
                   size_t ringSize, bool clockwise);

    void AddNodeData(double arrivalTime, size_t nodeID);
    const std::list<size_t>& getVisitedNodes() const;
    size_t getDestNodeID() const;
    bool isClockwise() const { return _clockwise; }  // New method
    double GetTimeInNetwork() const;
    void PrintData() const override;

private:
    const size_t _originNodeID;
    const size_t _destNodeID;
    const size_t _ringSize;
    const bool _clockwise;  // New member to store direction
    std::list<double> _nodeArrivalTimes;
    std::list<size_t> _visitedNodes;
    size_t _minWrappedDist;  // Minimum wrapped distance seen so far

    // Helper method for wrapped distance calculation
    size_t getWrappedDistance(size_t pos1, size_t pos2) const {
        size_t clockwise_dist = (pos2 >= pos1) ?
        pos2 - pos1 :
        _ringSize - (pos1 - pos2);
        size_t counter_dist = (pos1 >= pos2) ?
        pos1 - pos2 :
        _ringSize - (pos2 - pos1);
        return std::min(clockwise_dist, counter_dist);
    }
};

// #pragma once
// #include <list>
// #include "../OoO_EventSet.h"
//
// class Ring_1D_Packet : public Entity {
// public:
//     Ring_1D_Packet(double genTime, size_t originNodeID, size_t destNodeID,
//                    size_t ringSize);
//
//     void AddNodeData(double arrivalTime, size_t nodeID);
//     const std::list<size_t>& getVisitedNodes() const;
//     size_t getDestNodeID() const;
//     double GetTimeInNetwork() const;
//     void PrintData() const override;
//
// private:
//     const size_t _originNodeID;
//     const size_t _destNodeID;
//     std::list<double> _nodeArrivalTimes;
//     std::list<size_t> _visitedNodes;
//     const size_t _ringSize;
//     size_t _minWrappedDist;  // Minimum wrapped distance seen so far
//
//     // Helper method for wrapped distance calculation
//     size_t getWrappedDistance(size_t pos1, size_t pos2) const {
//         size_t clockwise_dist = (pos2 >= pos1) ?
//                                pos2 - pos1 :
//                                _ringSize - (pos1 - pos2);
//         size_t counter_dist = (pos1 >= pos2) ?
//                              pos1 - pos2 :
//                              _ringSize - (pos2 - pos1);
//         return std::min(clockwise_dist, counter_dist);
//     }
// };
