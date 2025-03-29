#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>
#include <limits>

class Torus_3D_Arrive;

struct Torus_3D_Path {
    std::vector<size_t> nodeIndices;
    double totalCost;
    Torus_3D_Path() : totalCost(std::numeric_limits<double>::max()) {}
    void PrintPath();
};

struct Torus_3D_NeighborInfo {
    std::vector<std::vector<size_t>> I;  // Hierarchical indices
    std::vector<std::shared_ptr<Torus_3D_Arrive>> N;  // Flat array of neighbor pointers
    std::unordered_map<size_t, size_t> globalToLocalIdx;  // Maps network node IDs to indices in N

    // Coordinate conversion helpers
    std::tuple<size_t, size_t, size_t> getCoordinates(size_t nodeID, size_t gridSizeX, size_t gridSizeY) const;
    size_t getNodeID(size_t x, size_t y, size_t z, size_t gridSizeX, size_t gridSizeY) const;

    // Neighbor finding with torus wrapping
    std::vector<size_t> getTorusNeighbors(size_t nodeID, size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ) const;
    std::vector<size_t> getValidNeighbors(size_t nodeID, size_t targetNodeID, size_t hopsFromStart,
                                         size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ) const;

    // Torus-aware pathfinding
    Torus_3D_Path findShortestPath(size_t finalDestNodeID, size_t intermedTargetNodeID,
                         size_t current_x, size_t current_y, size_t current_z,
                         size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                         size_t hopRadius, const std::list<size_t>& visitedNodes) const;

private:
    // Helper method for coordinate wrapping
    size_t wrapCoordinate(size_t coord, size_t size) const {
        return (coord + size) % size;
    }

    // Helper method for calculating wrapped distance
    size_t getWrappedDistance(int coord1, int coord2, size_t size) const {
        int direct_dist = abs(coord2 - coord1);
        int wrapped_dist = size - direct_dist;
        return std::min(direct_dist, wrapped_dist);
    }
};
