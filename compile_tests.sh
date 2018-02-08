#!/bin/sh

set -x

# OOPC tests

#./compile.sh -DDEBUG_OBJ -DDEBUG_MEM example.c test_example.c #-o test_example
./compile.sh -DDEBUG_OBJ -DDEBUG_MEM test.c -o main
#./compile.sh -DDEBUG_OBJ -DDEBUG_MEM memBlock.c array.c test_array.c #-o test_array -lm
#./compile.sh test_exception.c -o test_exception -lm
#./compile.sh -DDEBUG_CALL -DDEBUG_MEM -DDEBUG_THROW test_protection.c -o test_protection -lm

# C++ tests

#./compile-cpp.sh test_example.C -o cpp-test_example
#./compile-cpp.sh person.C employee.C education.C manager.C test_manager.C -o cpp-test_manager
