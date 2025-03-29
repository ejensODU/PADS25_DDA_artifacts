#include "Grid_VN2D_Depart.h"
#include "Grid_VN2D_Arrive.h"
#include "../Dist.h"
#include "../OoO_EventSet.h"
#include <algorithm>

Grid_VN2D_Depart::Grid_VN2D_Depart(size_t networkNodeID, size_t x, size_t y,
                                  std::vector<bool> neighbors,
                                  size_t gridSizeX, size_t gridSizeY, size_t hopRadius,
                                  const std::string& vertexName,
                                  size_t distSeed, std::string traceFolderName,
                                  std::string traceFileHeading,
                                  double minServiceTime, double modeServiceTime, double maxServiceTime,
                                  double minTransitTime, double modeTransitTime, double maxTransitTime,
                                  OoO_SV<int>& packetQueueSV,
                                  std::queue<std::shared_ptr<Grid_VN2D_Packet>>& packetQueue)
    : Vertex(vertexName, 0, distSeed, traceFolderName, traceFileHeading),
      _networkNodeID(networkNodeID),
      _x(x), _y(y), _hopRadius(hopRadius),
      _neighbors(neighbors),
      _gridSizeX(gridSizeX), _gridSizeY(gridSizeY),
      _packetQueueSV(packetQueueSV), _packetQueue(packetQueue) {

    _serviceDelay = std::make_unique<TriangularDist>(minServiceTime, modeServiceTime, maxServiceTime, distSeed + networkNodeID);
    _transitDelay = std::make_unique<TriangularDist>(minTransitTime, modeTransitTime, maxTransitTime, distSeed + networkNodeID);
}

void Grid_VN2D_Depart::AddNeighborInfo(const Grid_VN2D_NeighborInfo& info) {
    _neighborInfo = info;
}

void Grid_VN2D_Depart::AddArriveVertices(std::vector<std::shared_ptr<Grid_VN2D_Arrive>> arriveVertices) {
    _arriveVertices = arriveVertices;
}

void Grid_VN2D_Depart::IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) {
    std::vector<size_t> input_SV_indices;
    input_SV_indices.push_back(_packetQueueSV.getModelIndex());

    for (const auto& neighbor : _neighborInfo.N) {
        if (neighbor) {
            input_SV_indices.push_back(neighbor->getPacketQueueSV().getModelIndex());
        }
    }

    Is.push_back(input_SV_indices);

    std::vector<size_t> output_SV_indices;
    output_SV_indices.push_back(_packetQueueSV.getModelIndex());
    Os.push_back(output_SV_indices);
}

void Grid_VN2D_Depart::PrintNeighborInfo() const {
    printf("\n=== Neighbor Info Debug Output ===\n");

    printf("I structure (size: %lu):\n", _neighborInfo.I.size());
    for (size_t i = 0; i < _neighborInfo.I.size(); i++) {
        printf("Hop level %lu (size: %lu): ", i, _neighborInfo.I[i].size());
        for (const auto& idx : _neighborInfo.I[i]) {
            printf("%lu ", idx);
        }
        printf("\n");
    }

    printf("\nN structure (size: %lu):\n", _neighborInfo.N.size());
    for (size_t i = 0; i < _neighborInfo.N.size(); i++) {
        if (_neighborInfo.N[i]) {
            printf("Index %lu: Node ID %lu (Queue size: %d)\n",
                   i, _neighborInfo.N[i]->getNetworkNodeID(),
                   _neighborInfo.N[i]->getPacketQueueSV().get());
        } else {
            printf("Index %lu: nullptr\n", i);
        }
    }

    printf("\nCurrent node position: x=%lu, y=%lu\n", _x, _y);
    printf("Grid size: %lu x %lu\n", _gridSizeX, _gridSizeY);
    printf("Hop radius: %lu\n", _hopRadius);
    printf("==============================\n\n");
}

void Grid_VN2D_Depart::Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) {
    std::shared_ptr<Grid_VN2D_Packet> packet = std::dynamic_pointer_cast<Grid_VN2D_Packet>(entity);

    // Evaluate Conditions
    _packetInQueue = (_packetQueueSV.get() > 0);

    // Update SVs
    std::shared_ptr<Grid_VN2D_Packet> queue_packet;
    _packetQueueSV.dec();
    if (_packetInQueue) {
        queue_packet = std::move(_packetQueue.front());
        _packetQueue.pop();
    }

    // Determine routing
    size_t dest_network_node_ID = packet->getDestNetworkNodeID();
    size_t dest_x = dest_network_node_ID % _gridSizeX;
    size_t dest_y = dest_network_node_ID / _gridSizeX;
    size_t current_network_node_ID = _y * _gridSizeX + _x;

    size_t dest_x_diff = abs(static_cast<int>(dest_x) - static_cast<int>(_x));
    size_t dest_y_diff = abs(static_cast<int>(dest_y) - static_cast<int>(_y));
    size_t current_distance = dest_x_diff + dest_y_diff;

    auto visited_nodes = packet->getVisitedNetworkNodes();

    // Find best path
    Grid_VN2D_Path best_path;

    // If destination is within hop radius
    if (current_distance <= _hopRadius) {
        best_path = _neighborInfo.findShortestPath(dest_network_node_ID, dest_network_node_ID,
                                               _x, _y, _gridSizeX, _gridSizeY,
                                               _hopRadius, visited_nodes);
    }
    // Destination is beyond hop radius
    else {
        const auto& max_hop_neighbors = _neighborInfo.I.back();
        size_t min_path_manhattan_distance = std::numeric_limits<size_t>::max();
        std::vector<Grid_VN2D_Path> max_hop_shortest_paths;

        // Find max-hop neighbors that reduce distance to destination
        for (size_t neighbor_id : max_hop_neighbors) {
            if (std::find(visited_nodes.begin(), visited_nodes.end(), neighbor_id) != visited_nodes.end()) {
                continue;
            }

            size_t neighbor_x = neighbor_id % _gridSizeX;
            size_t neighbor_y = neighbor_id / _gridSizeX;

            size_t neighbor_distance = abs(static_cast<int>(dest_x) - static_cast<int>(neighbor_x)) +
                                    abs(static_cast<int>(dest_y) - static_cast<int>(neighbor_y));

            if (neighbor_distance < current_distance) {
                Grid_VN2D_Path path = _neighborInfo.findShortestPath(dest_network_node_ID, neighbor_id,
                                                                 _x, _y, _gridSizeX, _gridSizeY,
                                                                 _hopRadius, visited_nodes);
                if (!path.nodeIndices.empty()) {
                    max_hop_shortest_paths.push_back(path);
                }
            }
        }

        if (max_hop_shortest_paths.empty()) {
            printf("Error: No valid paths found!\n");
            packet->PrintData();
            printf("Current: (%lu, %lu) %lu, Destination: (%lu, %lu) %lu\n",
                   _x, _y, _networkNodeID, dest_x, dest_y, dest_network_node_ID);
            exit(1);
        }

        // Sort paths by total cost
        std::sort(max_hop_shortest_paths.begin(), max_hop_shortest_paths.end(),
                 [](const Grid_VN2D_Path& a, const Grid_VN2D_Path& b) {
                     return a.totalCost < b.totalCost;
                 });

        // Keep only the cheaper half of paths
        size_t num_paths_to_keep = (max_hop_shortest_paths.size() + 1) / 2;
        max_hop_shortest_paths.resize(num_paths_to_keep);

        // Among remaining paths, find the one ending closest to destination
        for (const Grid_VN2D_Path& path : max_hop_shortest_paths) {
            size_t max_hop_node = path.nodeIndices.back();
            size_t max_hop_x = max_hop_node % _gridSizeX;
            size_t max_hop_y = max_hop_node / _gridSizeX;

            size_t distance = abs(static_cast<int>(dest_x) - static_cast<int>(max_hop_x)) +
                            abs(static_cast<int>(dest_y) - static_cast<int>(max_hop_y));

            if (distance < min_path_manhattan_distance) {
                min_path_manhattan_distance = distance;
                best_path = path;
            }
        }
    }

    // Determine direction from best path
    int dest_dir = -1;
    if (!best_path.nodeIndices.empty() && best_path.nodeIndices.size() > 1) {
        size_t next_node_id = best_path.nodeIndices[1];  // First hop

        // Check if next_node_id is a valid 1-hop neighbor
        const auto& one_hop_neighbors = _neighborInfo.I[0];
        if (std::find(one_hop_neighbors.begin(), one_hop_neighbors.end(), next_node_id)
            == one_hop_neighbors.end()) {
            printf("ERROR: Next hop %lu is not a valid 1-hop neighbor!\n", next_node_id);
        }
        else {
            // Convert to direction
            if (next_node_id == current_network_node_ID - 1) dest_dir = 0;      // West
            else if (next_node_id == current_network_node_ID + 1) dest_dir = 1;  // East
            else if (next_node_id == current_network_node_ID - _gridSizeX) dest_dir = 2;  // North
            else if (next_node_id == current_network_node_ID + _gridSizeX) dest_dir = 3;  // South
        }
    }

    if (dest_dir == -1) {
        printf("Error: No valid direction found!\n");
        printf("1-hop neighbors in I[0]: ");
        for (size_t id : _neighborInfo.I[0]) printf("%lu ", id);
        printf("\n");
        exit(1);
    }

    // Schedule New Events
    if (_packetInQueue) {
        newEvents.push_back(new OoO_Event(shared_from_this(),
                                          simTime + _serviceDelay->GenRV(),
                                          queue_packet));
    }

    newEvents.push_back(new OoO_Event(_arriveVertices.at(dest_dir),
                                      simTime + _transitDelay->GenRV(),
                                      packet));

    // Trace
    std::string trace_string = std::to_string(simTime) + ", ";
    for (const auto& neighbor : _neighborInfo.N) {
        if (neighbor) {
            trace_string.append(std::to_string(neighbor->getPacketQueueSV().get()) + ", ");
        }
    }
    trace_string.append(std::to_string(_packetQueueSV.get()));
    WriteToTrace(trace_string);

    _numExecutions++;
}
