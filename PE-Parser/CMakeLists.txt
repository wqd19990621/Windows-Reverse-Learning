cmake_minimum_required(VERSION 3.10)
project(PE-Parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Source files
# ============================================================================
set(SOURCES
    src/main.cpp
    src/pe_common.cpp
    src/pe_parser.cpp
    src/pe_dos.cpp
    src/pe_nt.cpp
    src/pe_section.cpp
    src/pe_import.cpp
    src/pe_export.cpp
    src/pe_relocation.cpp
    src/pe_info.cpp
    src/console.cpp
)

# ============================================================================
# Header files
# ============================================================================
set(HEADERS
    include/pe_common.h
    include/pe_parser.h
    include/console.h
)

# ============================================================================
# Executable target
# ============================================================================
add_executable(pe_parser ${SOURCES} ${HEADERS})

target_include_directories(pe_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Windows-specific settings
if(WIN32)
    # Set character set to Unicode (optional, but good practice)
    # Note: This project uses ANSI API explicitly (CreateFileA, etc.)

    # Link against required Windows libraries
    target_link_libraries(pe_parser PRIVATE)

    # Enable ASLR and DEP for the parser itself
    set_target_properties(pe_parser PROPERTIES
        WIN32_EXECUTABLE FALSE  # Console application
    )
endif()

# ============================================================================
# Install
# ============================================================================
install(TARGETS pe_parser RUNTIME DESTINATION bin)
