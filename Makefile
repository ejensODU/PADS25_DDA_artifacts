CXX = g++
CXXFLAGS = -std=c++20 -fopenmp -O3 -g
VPATH = Grid_VN2D Grid_VN3D Torus_3D Ring_1D

# Base objects
BASE_OBJECTS = Dist.o Vertex.o OoO_SimModel.o OoO_SimExec.o OoO_SV.o OoO_EventSet.o

# Grid objects
RING1D_OBJECTS = Ring_1D_Packet.o Ring_1D_Arrive.o Ring_1D_Depart.o Ring_1D.o
VN2D_OBJECTS = Grid_VN2D_Packet.o Grid_VN2D_NeighborInfo.o Grid_VN2D_Arrive.o Grid_VN2D_Depart.o Grid_VN2D.o
VN3D_OBJECTS = Grid_VN3D_Packet.o Grid_VN3D_NeighborInfo.o Grid_VN3D_Arrive.o Grid_VN3D_Depart.o Grid_VN3D.o
TORUS3D_OBJECTS = Torus_3D_Packet.o Torus_3D_NeighborInfo.o Torus_3D_Arrive.o Torus_3D_Depart.o Torus_3D.o

OBJECTS = $(BASE_OBJECTS) $(RING1D_OBJECTS) $(VN2D_OBJECTS) $(VN3D_OBJECTS) $(TORUS3D_OBJECTS)

all: OoO_Sim

OoO_Sim: OoO_Sim.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Base compilation rules
Dist.o: Dist.cpp Dist.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Vertex.o: Vertex.cpp Vertex.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

OoO_SimModel.o: OoO_SimModel.cpp OoO_SimModel.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

OoO_SimExec.o: OoO_SimExec.cpp OoO_SimExec.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

OoO_SV.o: OoO_SV.cpp OoO_SV.h OoO_SV.tpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

OoO_EventSet.o: OoO_EventSet.cpp OoO_EventSet.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 1D Ring compilation rules
Ring_1D_Packet.o: Ring_1D/Ring_1D_Packet.cpp Ring_1D/Ring_1D_Packet.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Ring_1D_Arrive.o: Ring_1D/Ring_1D_Arrive.cpp Ring_1D/Ring_1D_Arrive.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Ring_1D_Depart.o: Ring_1D/Ring_1D_Depart.cpp Ring_1D/Ring_1D_Depart.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Ring_1D.o: Ring_1D/Ring_1D.cpp Ring_1D/Ring_1D.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 2D Grid compilation rules
Grid_VN2D_Packet.o: Grid_VN2D/Grid_VN2D_Packet.cpp Grid_VN2D/Grid_VN2D_Packet.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN2D_NeighborInfo.o: Grid_VN2D/Grid_VN2D_NeighborInfo.cpp Grid_VN2D/Grid_VN2D_NeighborInfo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN2D_Arrive.o: Grid_VN2D/Grid_VN2D_Arrive.cpp Grid_VN2D/Grid_VN2D_Arrive.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN2D_Depart.o: Grid_VN2D/Grid_VN2D_Depart.cpp Grid_VN2D/Grid_VN2D_Depart.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN2D.o: Grid_VN2D/Grid_VN2D.cpp Grid_VN2D/Grid_VN2D.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 3D Grid compilation rules
Grid_VN3D_Packet.o: Grid_VN3D/Grid_VN3D_Packet.cpp Grid_VN3D/Grid_VN3D_Packet.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN3D_NeighborInfo.o: Grid_VN3D/Grid_VN3D_NeighborInfo.cpp Grid_VN3D/Grid_VN3D_NeighborInfo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN3D_Arrive.o: Grid_VN3D/Grid_VN3D_Arrive.cpp Grid_VN3D/Grid_VN3D_Arrive.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN3D_Depart.o: Grid_VN3D/Grid_VN3D_Depart.cpp Grid_VN3D/Grid_VN3D_Depart.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Grid_VN3D.o: Grid_VN3D/Grid_VN3D.cpp Grid_VN3D/Grid_VN3D.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 3D Torus compilation rules
Torus_3D_Packet.o: Torus_3D/Torus_3D_Packet.cpp Torus_3D/Torus_3D_Packet.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Torus_3D_NeighborInfo.o: Torus_3D/Torus_3D_NeighborInfo.cpp Torus_3D/Torus_3D_NeighborInfo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Torus_3D_Arrive.o: Torus_3D/Torus_3D_Arrive.cpp Torus_3D/Torus_3D_Arrive.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Torus_3D_Depart.o: Torus_3D/Torus_3D_Depart.cpp Torus_3D/Torus_3D_Depart.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Torus_3D.o: Torus_3D/Torus_3D.cpp Torus_3D/Torus_3D.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f OoO_Sim $(OBJECTS)

.PHONY: all clean
