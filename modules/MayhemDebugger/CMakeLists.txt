add_library(MayhemDebugger STATIC
    src/chain.cpp
    src/registry.cpp
)

target_include_directories(MayhemDebugger PUBLIC include)
target_compile_features(MayhemDebugger PUBLIC cxx_std_17)
target_compile_definitions(MayhemDebugger PUBLIC MDBG_ENABLED=$<BOOL:${MDBG_ENABLED}>)

add_subdirectory(samples/enemy_demo)
