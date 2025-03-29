#pragma once

#include <memory>
#include <queue>
#include <vector>
#include "../Vertex.h"
#include "../OoO_SV.h"
#include "Grid_VN3D_Packet.h"
#include "Grid_VN3D_NeighborInfo.h"

class Grid_VN3D_Depart : public Vertex, public std::enable_shared_from_this<Grid_VN3D_Depart> {
public:
    Grid_VN3D_Depart(size_t networkNodeID,
                    size_t x, size_t y, size_t z,
                    std::vector<bool> neighbors,
                    size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                    size_t hopRadius, const std::string& vertexName,
                    size_t distSeed, std::string traceFolderName,
                    std::string traceFileHeading,
                    double minServiceTime, double modeServiceTime, double maxServiceTime,
                    double minTransitTime, double modeTransitTime, double maxTransitTime,
                    OoO_SV<int>& packetQueueSV,
                    std::queue<std::shared_ptr<Grid_VN3D_Packet>>& packetQueue);

    void AddNeighborInfo(const Grid_VN3D_NeighborInfo& info);
    void AddArriveVertices(std::vector<std::shared_ptr<Grid_VN3D_Arrive>> arriveVertices);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) override;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) override;
    void PrintNeighborInfo() const;

private:
    const size_t _networkNodeID;
    const size_t _x;
    const size_t _y;
    const size_t _z;
    const size_t _hopRadius;
    std::vector<bool> _neighbors;  // Six neighbors in 3D: West, East, North, South, Up, Down
    const size_t _gridSizeX;
    const size_t _gridSizeY;
    const size_t _gridSizeZ;
    Grid_VN3D_NeighborInfo _neighborInfo;
    bool _packetInQueue;
    OoO_SV<int>& _packetQueueSV;
    std::queue<std::shared_ptr<Grid_VN3D_Packet>>& _packetQueue;
    std::vector<std::shared_ptr<Grid_VN3D_Arrive>> _arriveVertices;
    std::unique_ptr<class TriangularDist> _serviceDelay;
    std::unique_ptr<class TriangularDist> _transitDelay;
};
