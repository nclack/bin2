cmake_minimum_required(VERSION 3.10)
project(Bin2x2Test C)

set(CMAKE_C_STANDARD 11)

# Function to add a test executable
function(add_bin2x2_test NAME SOURCE)
    add_executable(${NAME} test/test.c src/${SOURCE})
    target_include_directories(${NAME} PRIVATE src)
    if(${SOURCE} MATCHES "avx2")
        target_compile_options(${NAME} PRIVATE -mavx2)
    elseif(${SOURCE} MATCHES "neon")
        target_compile_options(${NAME} PRIVATE -mfpu=neon)
    endif()
endfunction()

# Add test executables
add_bin2x2_test(test_avx2_naive bin2.avx2.naive.c)
add_bin2x2_test(test_avx2_v2 bin2.avx2.v2.c)
add_bin2x2_test(test_neon_naive bin2.neon.naive.c)
add_bin2x2_test(test_neon_v2 bin2.neon.v2.c)

# Enable testing
enable_testing()

# Add tests
add_test(NAME AVX2_Naive COMMAND test_avx2_naive)
add_test(NAME AVX2_V2 COMMAND test_avx2_v2)
add_test(NAME NEON_Naive COMMAND test_neon_naive)
add_test(NAME NEON_V2 COMMAND test_neon_v2)
