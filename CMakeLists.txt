cmake_minimum_required(VERSION 3.16)
project(VNNLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------

if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc /wd4100 /wd4251 /wd4275 /wd4005 /wd4244 /wd4267)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS YY_NO_UNISTD_H)
else()
    add_compile_options(-Wall -Wextra -g -Wno-unused-parameter -Wno-unused-function)
endif()

# ------------------------------------------------------------
# Directories
# ------------------------------------------------------------

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GENERATED_DIR "${SRC_DIR}/generated" CACHE PATH "Directory with generated files")
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Includes
include_directories(
    ${GENERATED_DIR}
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/util
)

# ------------------------------------------------------------
# BNFC generation
# ------------------------------------------------------------

option(BNFC "Run BNFC to generate parser files" OFF)

if(BNFC)
    message(STATUS "BNFC generation enabled")

    find_program(BNFC_EXECUTABLE bnfc REQUIRED)
    find_package(BISON REQUIRED)
    find_package(FLEX REQUIRED)

    set(BNFC_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/grammar/grammar.cf)

    add_custom_command(
        OUTPUT
            ${GENERATED_DIR}/Absyn.C
            ${GENERATED_DIR}/Absyn.H
            ${GENERATED_DIR}/Printer.C
            ${GENERATED_DIR}/Printer.H
            ${GENERATED_DIR}/Buffer.C
            ${GENERATED_DIR}/Buffer.H
            ${GENERATED_DIR}/grammar.y
            ${GENERATED_DIR}/grammar.l
            ${GENERATED_DIR}/Parser.H
            ${GENERATED_DIR}/ParserError.H
            ${GENERATED_DIR}/Skeleton.C
            ${GENERATED_DIR}/Skeleton.H
            ${GENERATED_DIR}/Test.C
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
        COMMAND ${BNFC_EXECUTABLE} --cpp -o ${GENERATED_DIR} ${BNFC_INPUT}
        DEPENDS ${BNFC_INPUT}
        COMMENT "Running BNFC on ${BNFC_INPUT}"
        VERBATIM
    )

    add_custom_target(BNFCGen
        DEPENDS
            ${GENERATED_DIR}/Absyn.C
            ${GENERATED_DIR}/Absyn.H
            ${GENERATED_DIR}/Printer.C
            ${GENERATED_DIR}/Printer.H
            ${GENERATED_DIR}/Buffer.C
            ${GENERATED_DIR}/Buffer.H
            ${GENERATED_DIR}/grammar.y
            ${GENERATED_DIR}/grammar.l
            ${GENERATED_DIR}/Parser.H
            ${GENERATED_DIR}/ParserError.H
            ${GENERATED_DIR}/Skeleton.C
            ${GENERATED_DIR}/Skeleton.H
            ${GENERATED_DIR}/Test.C
    )

    # ------------------------------------------------------------
    # Bison / Flex
    # ------------------------------------------------------------

    # Custom commands for Bison and Flex
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/Parser.C ${GENERATED_DIR}/Bison.H
        COMMAND ${BISON_EXECUTABLE} -t -pgrammar_ -d ${GENERATED_DIR}/grammar.y -o ${GENERATED_DIR}/Parser.C
        DEPENDS ${GENERATED_DIR}/grammar.y BNFCGen
        WORKING_DIRECTORY ${GENERATED_DIR}
        COMMENT "Running Bison on grammar.y"
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${GENERATED_DIR}/Lexer.C
        COMMAND ${FLEX_EXECUTABLE} -Pgrammar_ -o${GENERATED_DIR}/Lexer.C ${GENERATED_DIR}/grammar.l
        DEPENDS ${GENERATED_DIR}/grammar.l ${GENERATED_DIR}/Bison.H BNFCGen
        WORKING_DIRECTORY ${GENERATED_DIR}
        COMMENT "Running Flex on grammar.l"
        VERBATIM
    )

    # Create targets for parser generation
    add_custom_target(ParserGen
        DEPENDS ${GENERATED_DIR}/Parser.C ${GENERATED_DIR}/Lexer.C ${GENERATED_DIR}/Bison.H
    )

else()
    message(STATUS "Skipping BNFC generation as BNFC is not set")
    add_custom_target(BNFCGen)
    add_custom_target(ParserGen)
endif()

# ------------------------------------------------------------
# Sources
# ------------------------------------------------------------

set(MAIN_SRCS
    ${SRC_DIR}/VNNLib.cpp
    ${SRC_DIR}/TypeChecker.cpp
    ${SRC_DIR}/TypedAST.cpp
    ${SRC_DIR}/TypedBuilder.cpp
    ${SRC_DIR}/LinearArithExpr.cpp
    ${SRC_DIR}/CompatTransformer.cpp
    ${SRC_DIR}/DNFConverter.cpp
)

set(PARSER_MAIN
    ${SRC_DIR}/VNNLibParser.cpp
)

set(GENERATED_SRCS
    ${GENERATED_DIR}/Absyn.C
    ${GENERATED_DIR}/Printer.C
    ${GENERATED_DIR}/Buffer.C
    ${GENERATED_DIR}/Parser.C
    ${GENERATED_DIR}/Lexer.C
)

# Ensure uppercase .C files are treated as C++
set_source_files_properties(${GENERATED_SRCS} PROPERTIES LANGUAGE CXX)

# Add specific compiler flags for generated files (suppress warnings)
if(NOT MSVC)
    set_source_files_properties(${GENERATED_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion")
endif()

# ------------------------------------------------------------
# Library
# ------------------------------------------------------------

add_library(VNNLib SHARED
    ${MAIN_SRCS}
    ${GENERATED_SRCS}
)

# Define VNNLIB_BUILD_DLL when building the shared library on Windows (MSVC and MinGW)
if(WIN32)
    target_compile_definitions(VNNLib PRIVATE VNNLIB_BUILD_DLL)
endif()

# Ensure parser generation before building
add_dependencies(VNNLib BNFCGen ParserGen)

target_include_directories(VNNLib
    PUBLIC
        ${INCLUDE_DIR}       
    PRIVATE
        ${INCLUDE_DIR}/util  
        ${GENERATED_DIR}     
)

if(UNIX)
    target_link_libraries(VNNLib m)
    target_compile_options(VNNLib PRIVATE -fvisibility=hidden)
endif()

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(VNNLibParser
    ${MAIN_SRCS}
    ${PARSER_MAIN}
    ${GENERATED_SRCS}
)

# Define VNNLIB_BUILD_DLL for the executable on Windows since it compiles the sources directly
if(WIN32)
    target_compile_definitions(VNNLibParser PRIVATE VNNLIB_BUILD_DLL)
endif()

# Ensure parser generation happens before building executable
add_dependencies(VNNLibParser BNFCGen ParserGen)

if(UNIX)
    target_link_libraries(VNNLibParser m)
endif()

# Add clean target for generated files
add_custom_target(clean-generated
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${GENERATED_DIR}
    COMMENT "Cleaning generated parser files"
)

# ------------------------------------------------------------
# Optional Julia wrapper
# ------------------------------------------------------------

option(MAKE_JULIA "Build Julia wrapper library" OFF)

if(MAKE_JULIA)
    message(STATUS "Building Julia wrapper library")
    
    # Find JlCxx (from CxxWrap.jl)
    find_package(JlCxx REQUIRED)
    
    # Define the Julia wrapper sources
    # Note: Adjust this path if your VNNLibJulia.cpp is located elsewhere
    set(JULIA_WRAPPER_SOURCE "${SRC_DIR}/VNNLibJulia.cpp")

    add_library(VNNLibJulia SHARED ${JULIA_WRAPPER_SOURCE})
    
    # VNNLibJulia should NOT define VNNLIB_BUILD_DLL (it imports from VNNLib)
    # The wrapper itself doesn't export VNNLib symbols, it wraps them
    
    # Ensure that the wrapper can see VNNLib headers and JlCxx headers
    target_include_directories(VNNLibJulia
        PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${JlCxx_INCLUDE_DIRS}
            ${INCLUDE_DIR}
            ${INCLUDE_DIR}/util
            ${GENERATED_DIR}
    )
    
    # Link against JlCxx and the VNNLib library
    target_link_libraries(VNNLibJulia 
        PRIVATE
            JlCxx::cxxwrap_julia
            JlCxx::cxxwrap_julia_stl
            VNNLib
    )
    
    # On Windows/MinGW: enable big object files to handle many template instantiations
    if(WIN32)
        target_compile_options(VNNLibJulia PRIVATE -Wa,-mbig-obj)
    endif()
    
    # On *nix: avoid the lib prefix, so Julia can load the shared library directly
    set_target_properties(VNNLibJulia PROPERTIES
        PREFIX ""  # yields "VNNLibJulia.so" instead of "libVNNLibJulia.so" 
    )
    
    # Ensure parser generation happens before building Julia wrapper
    add_dependencies(VNNLibJulia BNFCGen ParserGen VNNLib)
    
    message(STATUS "Julia wrapper library will be built as `VNNLibJulia`")
endif()