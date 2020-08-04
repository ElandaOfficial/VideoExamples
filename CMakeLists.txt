cmake_minimum_required(VERSION 3.15)

# Create Project
project(VideoExamples
    VERSION   0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(PROJECT_COMPANY_NAME "Foleys Finest")

# Import libraries
add_subdirectory(libraries)

# Get assets
add_subdirectory(Resources)

# Get modules
add_subdirectory(modules)

# Importing sub-projects
add_subdirectory(VideoEditor)
# add_subdirectory(VideoPlayer)

