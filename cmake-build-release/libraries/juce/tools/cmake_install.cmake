# Install script for directory: /home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/JUCE-6.0.1" TYPE FILE FILES
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/cmake-build-release/libraries/juce/tools/JUCEConfigVersion.cmake"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/cmake-build-release/libraries/juce/tools/JUCEConfig.cmake"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/LaunchScreen.storyboard"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/PIPAudioProcessor.cpp.in"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/PIPComponent.cpp.in"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/PIPConsole.cpp.in"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/RecentFilesMenuTemplate.nib"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/UnityPluginGUIScript.cs.in"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/copyDir.cmake"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/JUCEHelperTargets.cmake"
    "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/libraries/juce/extras/Build/CMake/JUCEUtils.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/cmake-build-release/libraries/juce/tools/modules/cmake_install.cmake")
  include("/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/cmake-build-release/libraries/juce/tools/extras/Build/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/elanda/Documents/Development/Juce/Libraries/VideoExamples/cmake-build-release/libraries/juce/tools/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
