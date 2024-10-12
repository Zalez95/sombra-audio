cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

###############################################################################
# 								SOMBRA_AUDIO
###############################################################################
# Project name and version
project(SombraAudio LANGUAGES CXX)
set(SOMBRA_AUDIO_VERSION_MAJOR "1")
set(SOMBRA_AUDIO_VERSION_MINOR "0")
set(SOMBRA_AUDIO_VERSION_REVISION "0")
set(SOMBRA_AUDIO_VERSION "${SOMBRA_AUDIO_VERSION_MAJOR}.${SOMBRA_AUDIO_VERSION_MINOR}.${SOMBRA_AUDIO_VERSION_REVISION}")

# SombraAudio Options
option(SOMBRA_AUDIO_BUILD_DOC "Generate the SombraAudio documentation" ON)

# Find the dependencies
find_package(glm)
find_package(stdext)
find_package(miniaudio)
find_package(Doxygen)

# Find the library source files
file(GLOB_RECURSE SOMBRA_AUDIO_SOURCES "src/*.cpp")

# Create the library
add_library(SombraAudio ${SOMBRA_AUDIO_SOURCES})

# Add the target properties
target_include_directories(SombraAudio
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE "src"
)
set_target_properties(SombraAudio PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(SombraAudio PRIVATE "-Wall" "-Wextra" "-Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(SombraAudio PRIVATE "/W4" "/bigobj" "-D_CRT_SECURE_NO_WARNINGS")
endif()

# Link the dependencies
target_link_libraries(SombraAudio
	PUBLIC glm::glm stdext::stdext miniaudio::miniaudio
)

# Install the target
set_target_properties(SombraAudio PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)
install(TARGETS SombraAudio EXPORT SombraAudioTargets)
install(EXPORT SombraAudioTargets
	FILE SombraAudioTargets.cmake
	NAMESPACE SombraAudio::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SombraAudio"
)
install(DIRECTORY "include/saudio" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create the documentation
if(SOMBRA_AUDIO_BUILD_DOC AND DOXYGEN_FOUND)
	set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in")
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/doc")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target(SombraAudioDoc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif()
