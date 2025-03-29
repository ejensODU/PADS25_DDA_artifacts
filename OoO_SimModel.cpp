#include "OoO_SimModel.h"
#include "Dist.h"

#include <omp.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

// Helper function to get the executable path
std::string getExecutablePath()
{
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string executablePath(result, (count > 0) ? count : 0);
    return executablePath.substr(0, executablePath.find_last_of("/"));
}

OoO_SimModel::OoO_SimModel(double maxSimTime, size_t numThreads, size_t distSeed, int numSerialOoO_Execs, std::string traceFolderName)
: _numVertices(0), _maxSimTime(maxSimTime), _numThreads(numThreads), _distSeed(distSeed), 
  _numSerialOoO_Execs(numSerialOoO_Execs), _traceFolderName(traceFolderName)
{
    std::cout << "OoO_SimModel " << numSerialOoO_Execs << std::endl;
    std::cout << "OpenMP num threads: " << omp_get_max_threads() << std::endl;
}

void OoO_SimModel::setNumVertices(size_t numVertices)
{
    if (0 == _numVertices) {
        _numVertices = numVertices;
        _edges = std::vector<std::vector<Edge>>(_numVertices);
    }
}

size_t OoO_SimModel::getNumVertices() { return _numVertices; }

void OoO_SimModel::Init_OoO(std::string tableFilename)
{
    std::vector<std::vector<float>> ITL_table;
    std::string table_path = getExecutablePath() + "/ITL_tables/" + tableFilename;
    std::cout << table_path << std::endl;
    
    if (!fs::exists(table_path)) {
        ITL_table = MakeITL(tableFilename);
    } else {
        ITL_table = ReadITLTableFromCSV(tableFilename);
    }
    
    _simExec = std::make_unique<OoO_SimExec>(_numThreads, ITL_table, _maxSimTime, _distSeed, _numSerialOoO_Execs);
}

// ITL function in OoO_SimModel Class
std::vector<std::vector<float>> OoO_SimModel::MakeITL(std::string tableFilename)
{
    // ITL, acquire input data
    // get shortest paths, from simulation model
    std::vector<std::vector<float>> shortest_paths = FloydWarshall();

    auto ITL_table_p1_gen_start = std::chrono::high_resolution_clock::now();
    
    // ITL, Phase One
    std::vector<std::vector<float>> ITL(_numVertices, std::vector<float>(_numVertices, 0));
    
    // reachable vertices
    std::vector<std::vector<size_t>> Rs(_numVertices);
    
    // for each vertex l, get reachable vertices
    for (size_t l=0; l<_numVertices; l++) {
        std::vector<size_t> l_m_paths;
        l_m_paths.reserve(_numVertices);
        for (size_t m=0; m<_numVertices; m++) {
            if (shortest_paths.at(l).at(m) < std::numeric_limits<float>::max()) {
                l_m_paths.push_back(m);
            }
        }
        Rs.at(l) = l_m_paths;
    }
    
    // later-event vertex
    for (size_t k=0; k<_numVertices; k++) {
        // create vertex-k SV set Sk
        std::vector<size_t> S_k;
        S_k.reserve(_Is.at(k).size() + _Os.at(k).size());
        std::set_union(_Is.at(k).begin(), _Is.at(k).end(),
                      _Os.at(k).begin(), _Os.at(k).end(),
                      std::back_inserter(S_k));
        std::sort(S_k.begin(), S_k.end());
        
        // set of vertices that can update Sk
        std::vector<size_t> U_Sk;
        U_Sk.reserve(_numVertices);
        for (size_t l=0; l<_numVertices; l++) {
            std::vector<size_t> Ol_Sk;
            Ol_Sk.reserve(std::min(_Os.at(l).size(), S_k.size()));
            std::set_intersection(_Os.at(l).begin(), _Os.at(l).end(),
                                S_k.begin(), S_k.end(),
                                std::back_inserter(Ol_Sk));
            if (Ol_Sk.size() > 0) {
                U_Sk.push_back(l);
            }
        }
        std::sort(U_Sk.begin(), U_Sk.end());
      
        // earlier-event vertex
        for (size_t j=0; j<_numVertices; j++) {
            // vertices reachable from vertex j and can update SVs in Sk
            std::vector<size_t> X_jk;
            X_jk.reserve(std::min(Rs.at(j).size(), U_Sk.size()));
            std::set_intersection(Rs.at(j).begin(), Rs.at(j).end(),
                                U_Sk.begin(), U_Sk.end(),
                                std::back_inserter(X_jk));
            
            // shortest-path j-x times, set minimum to ITL j-k value
            if (X_jk.size() > 0) {
                std::vector<float> T_jx;  
                T_jx.reserve(X_jk.size());
                for (auto& x_jk : X_jk) {
                    T_jx.push_back(shortest_paths.at(j).at(x_jk));
                }
                ITL.at(j).at(k) = *(std::min_element(T_jx.begin(), T_jx.end()));
            }
            // vertex j cannot affect vertex k
            else {
                ITL.at(j).at(k) = std::numeric_limits<float>::max();
            }
        }
    }
    
    auto ITL_table_p1_gen_stop = std::chrono::high_resolution_clock::now();
    auto ITL_table_p1_gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        ITL_table_p1_gen_stop - ITL_table_p1_gen_start);
    printf("ITL table phase 1 generation time %lf seconds\n", 
           ITL_table_p1_gen_duration.count() / 1e6);

    auto ITL_table_p2_gen_start = std::chrono::high_resolution_clock::now();
    
    // ITL, Phase Two
    // earlier event vertex
    for (size_t h=0; h<_numVertices; h++) {
        // later event vertex
        for (size_t i=0; i<_numVertices; i++) {
            // vertices that vertex i can affect immediately
            std::vector<size_t> Z_i;
            Z_i.reserve(_numVertices);
            for (size_t l=0; l<_numVertices; l++) {
                // vertex i can affect vertex l immediately
                if (0 == ITL.at(i).at(l)) {
                    Z_i.push_back(l);
                }
            }
            sort(Z_i.begin(), Z_i.end());
            
            // vertices that vertex h can reach and that vertex i can affect immediately
            std::vector<size_t> X_hi;
            X_hi.reserve(std::min(Rs.at(h).size(), Z_i.size()));
            std::set_intersection(Rs.at(h).begin(), Rs.at(h).end(),
                                Z_i.begin(), Z_i.end(),
                                std::back_inserter(X_hi));
            std::sort(X_hi.begin(), X_hi.end());
            
            // if intersection vertices exist
            if (X_hi.size() > 0) {
                std::vector<float> T_hi;
                T_hi.reserve(X_hi.size());
                
                // get shortest-path times for each intersection vertex
                for (const auto& x_hi : X_hi) {
                    T_hi.push_back(shortest_paths.at(h).at(x_hi));
                }
                std::sort(T_hi.begin(), T_hi.end());
                
                // update ITL if phase-two value is smaller
                if (T_hi.at(0) < ITL.at(h).at(i)) {
                    ITL.at(h).at(i) = T_hi.at(0);
                }
            }
        }
    }
    
    auto ITL_table_p2_gen_stop = std::chrono::high_resolution_clock::now();
    auto ITL_table_p2_gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        ITL_table_p2_gen_stop - ITL_table_p2_gen_start);
    printf("ITL table phase 2 generation time %lf seconds\n", 
           ITL_table_p2_gen_duration.count() / 1e6);

    WriteITLTableToCSV(ITL, tableFilename);
    return ITL;
}

std::vector<std::vector<float>> OoO_SimModel::ReadITLTableFromCSV(const std::string& tableFilename) const {
    std::vector<std::vector<float>> ITL;
    std::string directory = getExecutablePath() + "/ITL_tables/";
    std::string filename = directory + tableFilename;

    // Open the file
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading.\n";
        return ITL;
    }

    // Read the 2D vector from the file
    std::string line;
    while (getline(file, line)) {
        std::vector<float> row;
        std::stringstream ss(line);
        std::string cell;

        while (getline(ss, cell, ',')) {
            row.push_back(static_cast<float>(std::stoi(cell)));
        }
        ITL.push_back(row);
    }
    file.close();
    std::cout << "Data successfully read from " << filename << "\n";

    return ITL;
}

void OoO_SimModel::WriteITLTableToCSV(std::vector<std::vector<float>>& ITL, std::string tableFilename) const
{
    std::string directory = getExecutablePath() + "/ITL_tables/";
    std::string filename = directory + tableFilename;
    
    // Create and open a file
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing.\n";
        return;
    }
    
    // Write the 2D vector to the file
    for (const auto& row : ITL) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) {
                file << ","; // CSV format
            }
        }
        file << "\n";
    }
    file.close();
    std::cout << "Data successfully written to " << filename << "\n";
}

std::vector<std::vector<float>> OoO_SimModel::FloydWarshall()
{
    auto FW_table_gen_start = std::chrono::high_resolution_clock::now();
    
    // Step 1: Initialize the distance matrix
    std::vector<std::vector<float>> dist(_numVertices, 
                                       std::vector<float>(_numVertices, std::numeric_limits<float>::max()));
    for (size_t i=0; i<_numVertices; i++) {
        dist[i][i] = 0;
    }
    
    // Step 2: Populate initial distances based on edges
    for (size_t i=0; i<_numVertices; i++) {
        for (const auto& edge : _edges.at(i)) {
            size_t sourceVertexIdx = i;
            size_t targetVertexIdx = edge.getTermVertexIndex();
            float minDelayTime = edge.getMinDist();
            if (sourceVertexIdx != targetVertexIdx) { // Skip self-loops
                dist[sourceVertexIdx][targetVertexIdx] = minDelayTime;
            }
        }
    }
    
    // Step 3: Apply Floyd-Warshall algorithm
    for (size_t k=0; k<_numVertices; k++) {
        for (size_t i=0; i<_numVertices; i++) {
            for (size_t j=0; j<_numVertices; j++) {
                if (dist[i][k] != std::numeric_limits<float>::max() && 
                    dist[k][j] != std::numeric_limits<float>::max() &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
    
    auto FW_table_gen_stop = std::chrono::high_resolution_clock::now();
    auto FW_table_gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        FW_table_gen_stop - FW_table_gen_start);
    printf("F-W table generation time %lf seconds\n", FW_table_gen_duration.count() / 1e6);

    return dist;
}

void OoO_SimModel::SimulateModel(std::string execOrderFilename)
{
    // Add initial events and run simulation
    for (auto& event : _initEvents) {
        _simExec->ScheduleInitEvent(event);
    }
    
    // Run the simulation
    _simExec->RunSerialSim(execOrderFilename);
}