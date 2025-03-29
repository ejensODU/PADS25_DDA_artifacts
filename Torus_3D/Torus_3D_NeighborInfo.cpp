#include "Torus_3D_NeighborInfo.h"
#include "Torus_3D_Arrive.h"
#include <queue>
#include <algorithm>
#include <cstdio>

void Torus_3D_Path::PrintPath() {
    printf("%lf: ", totalCost);
    for (auto index: nodeIndices) printf("%lu,", index);
    printf("\n");
}

std::tuple<size_t, size_t, size_t> Torus_3D_NeighborInfo::getCoordinates(size_t nodeID, size_t gridSizeX, size_t gridSizeY) const {
    size_t x = nodeID % gridSizeX;
    size_t y = (nodeID / gridSizeX) % gridSizeY;
    size_t z = nodeID / (gridSizeX * gridSizeY);
    return {x, y, z};
}

size_t Torus_3D_NeighborInfo::getNodeID(size_t x, size_t y, size_t z, size_t gridSizeX, size_t gridSizeY) const {
    return z * gridSizeX * gridSizeY + y * gridSizeX + x;
}

std::vector<size_t> Torus_3D_NeighborInfo::getTorusNeighbors(size_t nodeID, size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ) const {
    std::vector<size_t> neighbors;
    auto [x, y, z] = getCoordinates(nodeID, gridSizeX, gridSizeY);

    // Calculate wrapped coordinates for all six directions
    size_t west_x = wrapCoordinate(x - 1, gridSizeX);
    size_t east_x = wrapCoordinate(x + 1, gridSizeX);
    size_t north_y = wrapCoordinate(y - 1, gridSizeY);
    size_t south_y = wrapCoordinate(y + 1, gridSizeY);
    size_t down_z = wrapCoordinate(z - 1, gridSizeZ);
    size_t up_z = wrapCoordinate(z + 1, gridSizeZ);

    // Add all six neighbors (always valid in torus)
    neighbors.push_back(getNodeID(west_x, y, z, gridSizeX, gridSizeY));
    neighbors.push_back(getNodeID(east_x, y, z, gridSizeX, gridSizeY));
    neighbors.push_back(getNodeID(x, north_y, z, gridSizeX, gridSizeY));
    neighbors.push_back(getNodeID(x, south_y, z, gridSizeX, gridSizeY));
    neighbors.push_back(getNodeID(x, y, down_z, gridSizeX, gridSizeY));
    neighbors.push_back(getNodeID(x, y, up_z, gridSizeX, gridSizeY));

    return neighbors;
}

std::vector<size_t> Torus_3D_NeighborInfo::getValidNeighbors(size_t nodeID, size_t targetNodeID, size_t hopsFromStart,
                                                    size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ) const {
    auto allNeighbors = getTorusNeighbors(nodeID, gridSizeX, gridSizeY, gridSizeZ);
    std::vector<size_t> validNeighbors;
    const auto& hopLevelNeighbors = I.at(hopsFromStart);

    auto [node_x, node_y, node_z] = getCoordinates(nodeID, gridSizeX, gridSizeY);
    auto [target_x, target_y, target_z] = getCoordinates(targetNodeID, gridSizeX, gridSizeY);

    // Calculate current distance with torus wrapping
    size_t current_distance = getWrappedDistance(static_cast<int>(node_x), static_cast<int>(target_x), gridSizeX) +
                             getWrappedDistance(static_cast<int>(node_y), static_cast<int>(target_y), gridSizeY) +
                             getWrappedDistance(static_cast<int>(node_z), static_cast<int>(target_z), gridSizeZ);

    for (size_t neighborID : allNeighbors) {
        auto [neighbor_x, neighbor_y, neighbor_z] = getCoordinates(neighborID, gridSizeX, gridSizeY);

        // Calculate neighbor distance with torus wrapping
        size_t neighbor_distance = getWrappedDistance(static_cast<int>(neighbor_x), static_cast<int>(target_x), gridSizeX) +
                                 getWrappedDistance(static_cast<int>(neighbor_y), static_cast<int>(target_y), gridSizeY) +
                                 getWrappedDistance(static_cast<int>(neighbor_z), static_cast<int>(target_z), gridSizeZ);

        if (neighbor_distance >= current_distance) continue;

        if (std::find(hopLevelNeighbors.begin(), hopLevelNeighbors.end(), neighborID) != hopLevelNeighbors.end()) {
            validNeighbors.push_back(neighborID);
        }
    }

    return validNeighbors;
}

Torus_3D_Path Torus_3D_NeighborInfo::findShortestPath(size_t finalDestNodeID, size_t intermedTargetNodeID,
                                     size_t current_x, size_t current_y, size_t current_z,
                                     size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                                     size_t hopRadius, const std::list<size_t>& visitedNodes) const {
    struct QueueEntry {
        size_t nodeID;
        std::vector<size_t> path;
        double costSoFar;
        size_t hopsFromStart;

        bool operator>(const QueueEntry& other) const {
            return costSoFar > other.costSoFar;
        }
    };

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;
    std::unordered_map<size_t, double> bestCosts;
    size_t currentNodeID = getNodeID(current_x, current_y, current_z, gridSizeX, gridSizeY);

    auto [FD_x, FD_y, FD_z] = getCoordinates(finalDestNodeID, gridSizeX, gridSizeY);

    // Calculate initial distance with torus wrapping
    size_t current_distance_FD_node = getWrappedDistance(static_cast<int>(current_x), static_cast<int>(FD_x), gridSizeX) +
                                    getWrappedDistance(static_cast<int>(current_y), static_cast<int>(FD_y), gridSizeY) +
                                    getWrappedDistance(static_cast<int>(current_z), static_cast<int>(FD_z), gridSizeZ);

    queue.push({currentNodeID, {currentNodeID}, 0.0, 0});
    bestCosts[currentNodeID] = 0.0;

    while (!queue.empty()) {
        auto current = queue.top();
        queue.pop();

        if (current.nodeID == intermedTargetNodeID) {
            Torus_3D_Path result;
            result.nodeIndices = current.path;
            result.totalCost = current.costSoFar;
            return result;
        }

        if (current.hopsFromStart == hopRadius) {
            continue;
        }

        auto neighbors = getValidNeighbors(current.nodeID, intermedTargetNodeID,
                                         current.hopsFromStart, gridSizeX, gridSizeY, gridSizeZ);

        for (size_t neighborID : neighbors) {
            if (std::find(current.path.begin(), current.path.end(), neighborID) != current.path.end()) {
                continue;
            }
            if (std::find(visitedNodes.begin(), visitedNodes.end(), neighborID) != visitedNodes.end()) {
                continue;
            }

            auto [neighbor_x, neighbor_y, neighbor_z] = getCoordinates(neighborID, gridSizeX, gridSizeY);

            // Calculate neighbor distance with torus wrapping
            size_t neighbor_distance_FD_node = getWrappedDistance(static_cast<int>(neighbor_x), static_cast<int>(FD_x), gridSizeX) +
                                             getWrappedDistance(static_cast<int>(neighbor_y), static_cast<int>(FD_y), gridSizeY) +
                                             getWrappedDistance(static_cast<int>(neighbor_z), static_cast<int>(FD_z), gridSizeZ);

            if (neighbor_distance_FD_node >= current_distance_FD_node) {
                continue;
            }

            auto local_idx = globalToLocalIdx.at(neighborID);
            double queueSize = std::max(0.0, static_cast<double>(N[local_idx]->getPacketQueueSV().get()));
            double newCost = current.costSoFar + queueSize;

            if (bestCosts.find(neighborID) == bestCosts.end() || newCost < bestCosts[neighborID]) {
                bestCosts[neighborID] = newCost;
                std::vector<size_t> newPath = current.path;
                newPath.push_back(neighborID);
                queue.push({neighborID, newPath, newCost, current.hopsFromStart + 1});
            }
        }
    }

    return Torus_3D_Path();
}
