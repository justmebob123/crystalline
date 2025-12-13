PHP_ARG_ENABLE(algorithms, whether to enable Algorithms support,
[  --enable-algorithms   Enable Algorithms support])

if test "$PHP_ALGORITHMS" != "no"; then
  PHP_NEW_EXTENSION(algorithms, algorithms_extension.c, $ext_shared)
  
  # Add include paths
  PHP_ADD_INCLUDE(../../algorithms/include)
  PHP_ADD_INCLUDE(../../include)
  PHP_ADD_INCLUDE(../../math/include)
  
  # Add library paths and libraries
  PHP_ADD_LIBRARY_WITH_PATH(algorithms, ../.., ALGORITHMS_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(crystallinemath, ../../math/lib, ALGORITHMS_SHARED_LIBADD)
  PHP_SUBST(ALGORITHMS_SHARED_LIBADD)
fi