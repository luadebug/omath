# cmake/Coverage.cmake
include_guard(GLOBAL)

function(omath_setup_coverage TARGET_NAME)
    if(ANDROID OR IOS OR EMSCRIPTEN)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        target_compile_options(${TARGET_NAME} PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
            -g
            -O0
        )
        target_link_options(${TARGET_NAME} PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
        )

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE
            --coverage
            -g
            -O0
        )
        target_link_options(${TARGET_NAME} PRIVATE
            --coverage
        )

    elseif(MSVC)
        # MSVC requires debug info for coverage
        target_compile_options(${TARGET_NAME} PRIVATE
            /clang:-fprofile-instr-generate
            /clang:-fcoverage-mapping
            /Zi          # Debug information
            /Od          # Disable optimization
            /Ob0         # Disable inlining
            /PROFILE     # Enable profiling (for VS coverage)
            /guard:cf    # Enable control flow guard (helps with branch coverage)
            /JMC         # Just My Code debugging (improves coverage data)
        )
        target_link_options(${TARGET_NAME} PRIVATE
            /clang:-fprofile-instr-generate
            /DEBUG:FULL
            /INCREMENTAL:NO
            /PROFILE
        )
        
        # Add source-level debug info
        if(MSVC_VERSION GREATER_EQUAL 1920)
            target_compile_options(${TARGET_NAME} PRIVATE
                /debug:fastlink
                /ZH:SHA_256
            )
        endif()
        
        # Ensure debug symbols are generated
        set_target_properties(${TARGET_NAME} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "COR_ENABLE_PROFILING=1"
        )
    endif()

    # Create coverage target only once
    if(TARGET coverage)
        return()
    endif()

    if(MSVC)
        # Windows: VS Code Coverage (no custom target needed - run from CI)
        message(STATUS "MSVC detected: Use VS Code Coverage from CI workflow")
        
        # Create a simple target that just runs the tests
        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND $<TARGET_FILE:unit_tests>
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running tests for coverage (use VS Code Coverage from CI)"
        )

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND bash "${CMAKE_SOURCE_DIR}/scripts/coverage-llvm.sh"
                "${CMAKE_SOURCE_DIR}"
                "${CMAKE_BINARY_DIR}"
                "$<TARGET_FILE:unit_tests>"
                "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running LLVM coverage"
        )

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND $<TARGET_FILE:unit_tests> || true
            COMMAND lcov --capture --directory "${CMAKE_BINARY_DIR}"
                --output-file "${CMAKE_BINARY_DIR}/coverage.info"
                --ignore-errors mismatch,gcov
            COMMAND lcov --remove "${CMAKE_BINARY_DIR}/coverage.info"
                "*/tests/*" "*/gtest/*" "*/googletest/*" "*/_deps/*" "/usr/*"
                --output-file "${CMAKE_BINARY_DIR}/coverage.info"
                --ignore-errors unused
            COMMAND genhtml "${CMAKE_BINARY_DIR}/coverage.info"
                --output-directory "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Running lcov/genhtml"
        )
    endif()
endfunction()
