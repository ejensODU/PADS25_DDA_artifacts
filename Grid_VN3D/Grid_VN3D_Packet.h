#pragma once
#include <list>
#include "../OoO_EventSet.h"

class Grid_VN3D_Packet : public Entity {
public:
    Grid_VN3D_Packet(double genTime, size_t originNetworkNodeID, size_t destNetworkNodeID,
           size_t destX, size_t destY, size_t destZ,
           size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ);

    void AddNetworkNodeData(double arrivalTime, size_t networkNodeID);
    const std::list<size_t>& getVisitedNetworkNodes() const;
    size_t getDestNetworkNodeID() const;
    double GetTimeInNetwork() const;
    void PrintData() const override;

private:
    const size_t _originNetworkNodeID;
    const size_t _destNetworkNodeID;
    std::list<double> _networkNodeArrivalTimes;
    std::list<size_t> _visitedNetworkNodes;
    const size_t _destX;
    const size_t _destY;
    const size_t _destZ;
    const size_t _gridSizeX;
    const size_t _gridSizeY;
    const size_t _gridSizeZ;
    size_t _minManhattanDist;
};
