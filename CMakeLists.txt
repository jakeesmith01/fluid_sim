cmake_minimum_required(VERSION 3.10)

# Set the project name
project(FluidSim C)

# Find the SDL2 library
find_package(SDL2 REQUIRED)

# Add the executable
add_executable(FluidSim src/fluid.c)

# Include SDL2 directories
target_include_directories(FluidSim PRIVATE ${SDL2_INCLUDE_DIRS})

# Link SDL2 library
target_link_libraries(FluidSim PRIVATE ${SDL2_LIBRARIES})