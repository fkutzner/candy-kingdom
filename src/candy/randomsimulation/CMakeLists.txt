add_library(randomsimulation
    RandomSimulator.cc
    RandomSimulator.h
    SimulationVector.cc
    SimulationVector.h
    Randomization.cc
    Randomization.h
    Partition.cc
    Partition.h
    ClauseOrder.cc
    ClauseOrder.h
    Propagation.cc
    Propagation.h
    Conjectures.cc
    Conjectures.h)

if (NOT MSVC)
  target_compile_options(randomsimulation PRIVATE -Wextra)
endif()
