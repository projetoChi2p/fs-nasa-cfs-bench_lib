project(CFE_BENCH_LIB C)

GET_FILENAME_COMPONENT(BENCH_LIB_DIR   "${CMAKE_CURRENT_LIST_FILE}"  DIRECTORY)
GET_FILENAME_COMPONENT(TOP_PROJECT_DIR "${BENCH_LIB_DIR}/../../"     REALPATH )

message("+++ Using BENCH_LIB_DIR '${BENCH_LIB_DIR}'.")
message("+++ Using TOP_PROJECT_DIR '${TOP_PROJECT_DIR}'.")

find_path (EXTERNAL_BENCH_SOURCE_DIR
  fix16.c
  ${TOP_PROJECT_DIR}/../../fs_sofia/development/benchmark_applications/fixed_point
)

GET_FILENAME_COMPONENT(EXTERNAL_BENCH_SOURCE_DIR     "${EXTERNAL_BENCH_SOURCE_DIR}/../"  REALPATH )
message("+++ Using EXTERNAL_BENCH_SOURCE_DIR '${EXTERNAL_BENCH_SOURCE_DIR}'.")

# Create the app module
add_cfe_app(bench_lib 
  fsw/src/bench_lib.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/fixed_point/fix16.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/fixed_point_prng_mt/prng.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/mxm_fixed_point_mt/qmtx.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/mxm_fixed_point_mt/mxm_task.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt/huffman.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt/huff_check.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt/huff_bench.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt/huff_test_dataset.c
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt/huff_test.c
)

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.  Using "target_include_directories" is the 
# preferred method of indicating this (vs. directory-scope "include_directories").
target_include_directories(bench_lib PUBLIC 
  fsw/public_inc
  ${EXTERNAL_BENCH_SOURCE_DIR}/fixed_point
  ${EXTERNAL_BENCH_SOURCE_DIR}/fixed_point_prng_mt
  ${EXTERNAL_BENCH_SOURCE_DIR}/mxm_fixed_point_mt
  ${EXTERNAL_BENCH_SOURCE_DIR}/huffman_mt
)

if (ENABLE_UNIT_TESTS)
  add_subdirectory(ut-stubs)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)


