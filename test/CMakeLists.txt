# Find the source files
file(GLOB_RECURSE TEST_AUDIO_SOURCES "*.cpp")
file(GLOB TEST_AUDIO_FILES "*.mp3")

# Create the executable
add_executable(TestAudio "${TEST_AUDIO_SOURCES}")

# Add the target properties
set_target_properties(TestAudio PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	ON
)
target_include_directories(TestAudio PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(TestAudio PRIVATE "-Wall" "-Wextra" "-Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(TestAudio PRIVATE "/W4" "-D_CRT_SECURE_NO_WARNINGS")
endif()

# Link the dependencies
target_link_libraries(TestAudio PRIVATE SombraAudio)

# Install the target
set_target_properties(TestAudio PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)
file(COPY ${TEST_AUDIO_FILES} DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
install(TARGETS TestAudio RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
