#include "Torus_3D_Depart.h"
#include "Torus_3D_Arrive.h"
#include "../Dist.h"
#include "../OoO_EventSet.h"
#include <algorithm>

Torus_3D_Depart::Torus_3D_Depart(size_t networkNodeID,
                                size_t x, size_t y, size_t z,
                                std::vector<bool> neighbors,
                                size_t gridSizeX, size_t gridSizeY, size_t gridSizeZ,
                                size_t hopRadius, const std::string& vertexName,
                                size_t distSeed, std::string traceFolderName,
                                std::string traceFileHeading,
                                double minServiceTime, double modeServiceTime, double maxServiceTime,
                                double minTransitTime, double modeTransitTime, double maxTransitTime,
                                OoO_SV<int>& packetQueueSV,
                                std::queue<std::shared_ptr<Torus_3D_Packet>>& packetQueue)
    : Vertex(vertexName, 0, distSeed, traceFolderName, traceFileHeading),
      _networkNodeID(networkNodeID),
      _x(x), _y(y), _z(z), _hopRadius(hopRadius),
      _neighbors(neighbors),
      _gridSizeX(gridSizeX), _gridSizeY(gridSizeY), _gridSizeZ(gridSizeZ),
      _packetQueueSV(packetQueueSV), _packetQueue(packetQueue) {

    _serviceDelay = std::make_unique<TriangularDist>(minServiceTime, modeServiceTime, maxServiceTime, distSeed + networkNodeID);
    _transitDelay = std::make_unique<TriangularDist>(minTransitTime, modeTransitTime, maxTransitTime, distSeed + networkNodeID);
}

void Torus_3D_Depart::AddNeighborInfo(const Torus_3D_NeighborInfo& info) {
    _neighborInfo = info;
}

void Torus_3D_Depart::AddArriveVertices(std::vector<std::shared_ptr<Torus_3D_Arrive>> arriveVertices) {
    _arriveVertices = arriveVertices;
}

void Torus_3D_Depart::IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) {
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

void Torus_3D_Depart::PrintNeighborInfo() const {
    printf("\n=== Neighbor Info Debug Output ===\n");
    printf("Current node position: x=%lu, y=%lu, z=%lu\n", _x, _y, _z);
    printf("Grid size: %lu x %lu x %lu\n", _gridSizeX, _gridSizeY, _gridSizeZ);
    printf("Hop radius: %lu\n", _hopRadius);

    printf("\nI structure (size: %lu):\n", _neighborInfo.I.size());
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
    printf("==============================\n\n");
}

void Torus_3D_Depart::Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) {
    std::shared_ptr<Torus_3D_Packet> packet = std::dynamic_pointer_cast<Torus_3D_Packet>(entity);

    // Evaluate Conditions
    _packetInQueue = (_packetQueueSV.get() > 0);

    // Update SVs
    std::shared_ptr<Torus_3D_Packet> queue_packet;
    _packetQueueSV.dec();
    if (_packetInQueue) {
        queue_packet = std::move(_packetQueue.front());
        _packetQueue.pop();
    }

    // Determine routing with torus wrapping
    size_t dest_network_node_ID = packet->getDestNetworkNodeID();
    size_t dest_x = dest_network_node_ID % _gridSizeX;
    size_t dest_y = (dest_network_node_ID / _gridSizeX) % _gridSizeY;
    size_t dest_z = dest_network_node_ID / (_gridSizeX * _gridSizeY);

    // Calculate wrapped distances
    int dx = static_cast<int>(dest_x) - static_cast<int>(_x);
    int dy = static_cast<int>(dest_y) - static_cast<int>(_y);
    int dz = static_cast<int>(dest_z) - static_cast<int>(_z);

    // Adjust distances for torus wrapping
    if (abs(dx) > static_cast<int>(_gridSizeX/2)) {
        dx = (dx > 0) ? dx - _gridSizeX : dx + _gridSizeX;
    }
    if (abs(dy) > static_cast<int>(_gridSizeY/2)) {
        dy = (dy > 0) ? dy - _gridSizeY : dy + _gridSizeY;
    }
    if (abs(dz) > static_cast<int>(_gridSizeZ/2)) {
        dz = (dz > 0) ? dz - _gridSizeZ : dz + _gridSizeZ;
    }

    size_t current_distance = abs(dx) + abs(dy) + abs(dz);

    auto visited_nodes = packet->getVisitedNetworkNodes();

    // Find best path considering torus topology
    Torus_3D_Path best_path;

    // If destination is within hop radius
    if (current_distance <= _hopRadius) {
        best_path = _neighborInfo.findShortestPath(dest_network_node_ID, dest_network_node_ID,
                                                 _x, _y, _z,
                                                 _gridSizeX, _gridSizeY, _gridSizeZ,
                                                 _hopRadius, visited_nodes);
    }
    // Destination is beyond hop radius
    else {
        const auto& max_hop_neighbors = _neighborInfo.I.back();
        std::vector<Torus_3D_Path> max_hop_shortest_paths;

        // Find max-hop neighbors that reduce distance to destination
        for (size_t neighbor_id : max_hop_neighbors) {
            if (std::find(visited_nodes.begin(), visited_nodes.end(), neighbor_id) != visited_nodes.end()) {
                continue;
            }

            auto [neighbor_x, neighbor_y, neighbor_z] = _neighborInfo.getCoordinates(neighbor_id, _gridSizeX, _gridSizeY);

            // Calculate wrapped distances for neighbor to destination
            int ndx = static_cast<int>(dest_x) - static_cast<int>(neighbor_x);
            int ndy = static_cast<int>(dest_y) - static_cast<int>(neighbor_y);
            int ndz = static_cast<int>(dest_z) - static_cast<int>(neighbor_z);

            // Adjust distances for torus wrapping
            if (abs(ndx) > static_cast<int>(_gridSizeX/2)) {
                ndx = (ndx > 0) ? ndx - _gridSizeX : ndx + _gridSizeX;
            }
            if (abs(ndy) > static_cast<int>(_gridSizeY/2)) {
                ndy = (ndy > 0) ? ndy - _gridSizeY : ndy + _gridSizeY;
            }
            if (abs(ndz) > static_cast<int>(_gridSizeZ/2)) {
                ndz = (ndz > 0) ? ndz - _gridSizeZ : ndz + _gridSizeZ;
            }

            size_t neighbor_distance = abs(ndx) + abs(ndy) + abs(ndz);

            if (neighbor_distance < current_distance) {
                Torus_3D_Path path = _neighborInfo.findShortestPath(dest_network_node_ID, neighbor_id,
                                                         _x, _y, _z,
                                                         _gridSizeX, _gridSizeY, _gridSizeZ,
                                                         _hopRadius, visited_nodes);
                if (!path.nodeIndices.empty()) {
                    max_hop_shortest_paths.push_back(path);
                }
            }
        }

        if (max_hop_shortest_paths.empty()) {
            printf("Error: No valid paths to max-hop neighbors found!\n");
            packet->PrintData();
            printf("Current: (%lu, %lu, %lu) %lu, Destination: (%lu, %lu, %lu) %lu\n",
                   _x, _y, _z, _networkNodeID, dest_x, dest_y, dest_z, dest_network_node_ID);
            exit(1);
        }

        // Sort paths by total cost
        std::sort(max_hop_shortest_paths.begin(), max_hop_shortest_paths.end(),
                 [](const Torus_3D_Path& a, const Torus_3D_Path& b) {
                     return a.totalCost < b.totalCost;
                 });

        // Keep only the cheaper half of paths
        size_t num_paths_to_keep = (max_hop_shortest_paths.size() + 1) / 2;
        max_hop_shortest_paths.resize(num_paths_to_keep);

        // Among remaining paths, find the one ending closest to destination
        size_t min_path_manhattan_distance = std::numeric_limits<size_t>::max();

        for (const Torus_3D_Path& path : max_hop_shortest_paths) {
            size_t max_hop_node = path.nodeIndices.back();
            auto [max_hop_x, max_hop_y, max_hop_z] = _neighborInfo.getCoordinates(max_hop_node, _gridSizeX, _gridSizeY);

            // Calculate wrapped distance for this path endpoint
            int hdx = static_cast<int>(dest_x) - static_cast<int>(max_hop_x);
            int hdy = static_cast<int>(dest_y) - static_cast<int>(max_hop_y);
            int hdz = static_cast<int>(dest_z) - static_cast<int>(max_hop_z);

            // Adjust distances for torus wrapping
            if (abs(hdx) > static_cast<int>(_gridSizeX/2)) {
                hdx = (hdx > 0) ? hdx - _gridSizeX : hdx + _gridSizeX;
            }
            if (abs(hdy) > static_cast<int>(_gridSizeY/2)) {
                hdy = (hdy > 0) ? hdy - _gridSizeY : hdy + _gridSizeY;
            }
            if (abs(hdz) > static_cast<int>(_gridSizeZ/2)) {
                hdz = (hdz > 0) ? hdz - _gridSizeZ : hdz + _gridSizeZ;
            }

            size_t distance = abs(hdx) + abs(hdy) + abs(hdz);

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
        const auto& one_hop_neighbors = _neighborInfo.I[0];

        if (std::find(one_hop_neighbors.begin(), one_hop_neighbors.end(), next_node_id) == one_hop_neighbors.end()) {
            printf("ERROR: Next hop %lu is not a valid 1-hop neighbor!\n", next_node_id);
            exit(1);
        }

        // Calculate current and next node coordinates
        size_t cur_x = _x;
        size_t cur_y = _y;
        size_t cur_z = _z;

        size_t next_x = next_node_id % _gridSizeX;
        size_t next_y = (next_node_id / _gridSizeX) % _gridSizeY;
        size_t next_z = next_node_id / (_gridSizeX * _gridSizeY);

        // Calculate wrapped differences
        int dx = next_x - cur_x;
        if (abs(dx) > static_cast<int>(_gridSizeX/2)) {
            dx = (dx > 0) ? dx - _gridSizeX : dx + _gridSizeX;
        }

        int dy = next_y - cur_y;
        if (abs(dy) > static_cast<int>(_gridSizeY/2)) {
            dy = (dy > 0) ? dy - _gridSizeY : dy + _gridSizeY;
        }

        int dz = next_z - cur_z;
        if (abs(dz) > static_cast<int>(_gridSizeZ/2)) {
            dz = (dz > 0) ? dz - _gridSizeZ : dz + _gridSizeZ;
        }

        // Determine direction based on coordinate differences
        if (dx < 0) dest_dir = 0;      // West
        else if (dx > 0) dest_dir = 1;  // East
        else if (dy < 0) dest_dir = 2;  // North
        else if (dy > 0) dest_dir = 3;  // South
        else if (dz < 0) dest_dir = 4;  // Down
        else if (dz > 0) dest_dir = 5;  // Up
    }

    if (dest_dir == -1) {
        printf("Error: No valid direction found!\n");
        printf("1-hop neighbors in I[0]: ");
        for (size_t id : _neighborInfo.I[0]) printf("%lu ", id);
        printf("\nCurrent pos: (%lu, %lu, %lu)\n", _x, _y, _z);
        if (best_path.nodeIndices.size() > 1) {
            size_t next_node_id = best_path.nodeIndices[1];
            size_t next_x = next_node_id % _gridSizeX;
            size_t next_y = (next_node_id / _gridSizeX) % _gridSizeY;
            size_t next_z = next_node_id / (_gridSizeX * _gridSizeY);
            printf("Next pos: (%lu, %lu, %lu)\n", next_x, next_y, next_z);
        }
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
