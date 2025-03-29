#pragma once

#include <memory>
#include <queue>
#include <vector>
#include "../Vertex.h"
#include "../OoO_SV.h"
#include "Torus_3D_Packet.h"
#include "Torus_3D_NeighborInfo.h"

class Torus_3D_Depart : public Vertex, public std::enable_shared_from_this<Torus_3D_Depart> {
public:
    Torus_3D_Depart(size_t networkNodeID,
                    size_t x, size_t y, size_t z,
                    std::vector<bool> neighbors,
                    size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                    size_t hopRadius, const std::string& vertexName,
                    size_t distSeed, std::string traceFolderName,
                    std::string traceFileHeading,
                    double minServiceTime, double modeServiceTime, double maxServiceTime,
                    double minTransitTime, double modeTransitTime, double maxTransitTime,
                    OoO_SV<int>& packetQueueSV,
                    std::queue<std::shared_ptr<Torus_3D_Packet>>& packetQueue);

    void AddNeighborInfo(const Torus_3D_NeighborInfo& info);
    void AddArriveVertices(std::vector<std::shared_ptr<Torus_3D_Arrive>> arriveVertices);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) override;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) override;
    void PrintNeighborInfo() const;

private:
    const size_t _networkNodeID;
    const size_t _x;
    const size_t _y;
    const size_t _z;
    const size_t _hopRadius;
    std::vector<bool> _neighbors;  // Six neighbors in 3D: West, East, North, South, Down, Up
    const size_t _gridSizeX;
    const size_t _gridSizeY;
    const size_t _gridSizeZ;
    Torus_3D_NeighborInfo _neighborInfo;
    bool _packetInQueue;
    OoO_SV<int>& _packetQueueSV;
    std::queue<std::shared_ptr<Torus_3D_Packet>>& _packetQueue;
    std::vector<std::shared_ptr<Torus_3D_Arrive>> _arriveVertices;
    std::unique_ptr<class TriangularDist> _serviceDelay;
    std::unique_ptr<class TriangularDist> _transitDelay;

    // Helper method for coordinate wrapping
    size_t WrapCoordinate(size_t coord, size_t size) const {
        return (coord + size) % size;
    }
};
