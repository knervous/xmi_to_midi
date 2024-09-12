CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# Option for building WebAssembly or native
OPTION(BUILD_WASM "Build for WebAssembly" OFF)

if (BUILD_WASM)
    # Set the toolchain file for Emscripten before the project command
	if(DEFINED ENV{EMSDK})
		set(EMSDK_DIR $ENV{EMSDK})
	else()
		message(FATAL_ERROR "EMSDK environment variable is not set. Did you source emsdk_env.sh?")
	endif()

	set(CMAKE_TOOLCHAIN_FILE "${EMSDK_DIR}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE STRING "")

	message(STATUS "Using Emscripten toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

PROJECT(xmi_to_midi)

# Common source and header files
SET(xmi_sources
    file.cpp
    xmi.cpp
)

SET(xmi_headers
    file.h
    xmi.h
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (BUILD_WASM)
    # WASM specific configuration
    SET(wasm_sources
        ${xmi_sources}
        wasm.cpp
    )

    ADD_EXECUTABLE(xmi_to_midi ${wasm_sources} ${xmi_headers})

    # Emscripten build settings
    set_target_properties(xmi_to_midi PROPERTIES SUFFIX .js)

    target_link_options(xmi_to_midi
        PRIVATE
        -sALLOW_MEMORY_GROWTH
        -sEXPORT_NAME='XMI'
        -fexceptions
        -sMALLOC=mimalloc
        -sMODULARIZE
        -sEXPORT_ES6
        -sNO_EXIT_RUNTIME
        -sEXPORTED_RUNTIME_METHODS=FS,ccall,cwrap,stringToUTF8,UTF8ToString
        -sEXPORTED_FUNCTIONS=_xmi_to_midi,_malloc,_free
        -Os
    )

else()
    IF(NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    ENDIF(NOT CMAKE_BUILD_TYPE)

    # Add native target
    SET(native_sources
        ${xmi_sources}
        main.cpp
    )

    ADD_EXECUTABLE(xmi_to_midi ${native_sources} ${xmi_headers})

    # Installation for native target
    INSTALL(TARGETS xmi_to_midi RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

endif()
