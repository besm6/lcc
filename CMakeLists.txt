cmake_minimum_required(VERSION 3.16)
project(lcc)

set(CMAKE_C_FLAGS "-Wall -Werror") # -Wshadow

find_package(BISON REQUIRED)
bison_target(grammar "${CMAKE_SOURCE_DIR}/lburg/gram.y" "${CMAKE_CURRENT_BINARY_DIR}/gram.c")

#
# Build lburg
#
add_executable(lburg
    lburg/lburg.c
    gram.c
)
target_include_directories(lburg BEFORE PUBLIC
    lburg
)

#
# Build machine descriptions
#
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/dagcheck.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/dagcheck.md > ${CMAKE_BINARY_DIR}/dagcheck.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/dagcheck.md
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/alpha.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/alpha.md > ${CMAKE_BINARY_DIR}/alpha.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/alpha.md
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/mips.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/mips.md > ${CMAKE_BINARY_DIR}/mips.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/mips.md
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/sparc.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/sparc.md > ${CMAKE_BINARY_DIR}/sparc.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/sparc.md
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/x86.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/x86.md > ${CMAKE_BINARY_DIR}/x86.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/x86.md
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/x86linux.c
    COMMAND ${CMAKE_BINARY_DIR}/lburg ${CMAKE_SOURCE_DIR}/src/x86linux.md > ${CMAKE_BINARY_DIR}/x86linux.c
    DEPENDS ${CMAKE_BINARY_DIR}/lburg
            ${CMAKE_SOURCE_DIR}/src/x86linux.md
)

#
# Build rcc library
#
add_library(librcc STATIC
    src/alloc.c
    src/bind.c
    src/dag.c
    src/decl.c
    src/enode.c
    src/error.c
    src/expr.c
    src/event.c
    src/init.c
    src/inits.c
    src/input.c
    src/lex.c
    src/list.c
    src/output.c
    src/prof.c
    src/profio.c
    src/simp.c
    src/stmt.c
    src/string.c
    src/sym.c
    src/trace.c
    src/tree.c
    src/types.c
    src/null.c
    src/symbolic.c
    src/gen.c
    src/bytecode.c
    src/stab.c
    ${CMAKE_BINARY_DIR}/dagcheck.c
    ${CMAKE_BINARY_DIR}/alpha.c
    ${CMAKE_BINARY_DIR}/mips.c
    ${CMAKE_BINARY_DIR}/sparc.c
    ${CMAKE_BINARY_DIR}/x86.c
    ${CMAKE_BINARY_DIR}/x86linux.c
)
target_include_directories(librcc BEFORE PUBLIC
    src
)

#
# Build rcc
#
add_executable(rcc
    src/main.c
)
target_link_libraries(rcc librcc)

#
# Build cpp
#
add_executable(cpp
    cpp/cpp.c
    cpp/lex.c
    cpp/nlist.c
    cpp/tokens.c
    cpp/macro.c
    cpp/eval.c
    cpp/include.c
    cpp/hideset.c
    cpp/getopt.c
    cpp/unix.c
)

#
# Build lcc
#
add_executable(lcc
    etc/lcc.c
    etc/linux.c
)

#
# Build bprint
#
add_executable(bprint
    etc/bprint.c
)
target_include_directories(bprint BEFORE PUBLIC
    src
)

#
# Build lcc library
#
add_library(liblcc STATIC
    lib/assert.c
    lib/bbexit.c
    lib/yynull.c
)
