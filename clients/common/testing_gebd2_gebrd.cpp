
#include <testing_gebd2_gebrd.hpp>

#define TESTING_GEBD2_GEBRD(...) template void testing_gebd2_gebrd<__VA_ARGS__>(Arguments&);

INSTANTIATE(TESTING_GEBD2_GEBRD,
            FOREACH_BOOLEAN_0,
            FOREACH_BOOLEAN_1,
            FOREACH_BOOLEAN_INT,
            FOREACH_SCALAR_TYPE,
            APPLY_STAMP)
