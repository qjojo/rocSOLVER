
#include <testing_getf2_getrf.hpp>

#define TESTING_GETF2_GETRF(...) template void testing_getf2_getrf<__VA_ARGS__>(Arguments&);

INSTANTIATE(TESTING_GETF2_GETRF,
            FOREACH_BOOLEAN_0,
            FOREACH_BOOLEAN_1,
            FOREACH_BOOLEAN_INT,
            FOREACH_SCALAR_TYPE,
            APPLY_STAMP)
