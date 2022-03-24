# File: Makefile
# Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

# Tools and options
CXX        = g++
CXXFLAGS   = -g -Wall -Werror --pedantic-errors -std=c++17
MKDIR      = mkdir
MKDIRFLAGS = -p
RM         = rm -rf

# Folders and names
COORDINATOREXE = $(BIN)/mycoordinator
PARTICIPANTEXE = $(BIN)/myparticipant
SRC       = src
INC       = $(SRC)/include
BIN       = bin
OBJ       = obj
SRCS      = $(wildcard $(SRC)/*.cpp)
OBJS      = $(patsubst $(SRC)/%.cpp,$(BIN)/%.o,$(SRCS))

# Build rules
all: $(COORDINATOREXE) $(PARTICIPANTEXE)


$(COORDINATOREXE): $(OBJ)/coordinator.o $(OBJ)/multicast_message.o $(OBJ)/internet_socket.o $(OBJ)/buffer.o $(OBJ)/mycoordinator.o | $(BIN)
	$(CXX) $(CXXFLAGS) -I$(INC) $^ -o $@ -lpthread

$(PARTICIPANTEXE): $(OBJ)/participant.o $(OBJ)/multicast_message.o $(OBJ)/internet_socket.o $(OBJ)/buffer.o $(OBJ)/myparticipant.o | $(BIN)
	$(CXX) $(CXXFLAGS) -I$(INC) $^ -o $@ -lpthread

%: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC) -c $< -o $(OBJ)/$@.o

%: $(SRC)/%.cpp $(INC)/%.hpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC) -c $< -o $(OBJ)/$@.o

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(INC)/%.hpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC) -c $< -o $@

$(BIN) $(OBJ):
	$(MKDIR) $(MKDIRFLAGS) $@

clean:
	$(RM) obj bin