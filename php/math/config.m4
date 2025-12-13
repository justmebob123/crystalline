PHP_ARG_ENABLE(crystalline_math, whether to enable Crystalline Math support,
[  --enable-crystalline-math   Enable Crystalline Math support])

if test "$PHP_CRYSTALLINE_MATH" != "no"; then
  PHP_NEW_EXTENSION(crystalline_math, crystalline_math_extension.c, $ext_shared)
  
  # Add include paths
  PHP_ADD_INCLUDE(../../math/include)
  PHP_ADD_INCLUDE(../../include)
  
  # Add library paths and libraries
  PHP_ADD_LIBRARY_WITH_PATH(crystallinemath, ../../math/lib, CRYSTALLINE_MATH_SHARED_LIBADD)
  PHP_SUBST(CRYSTALLINE_MATH_SHARED_LIBADD)
fi