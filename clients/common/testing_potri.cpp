
#include <testing_potri.hpp>

#define TESTING_POTRI(...) template void testing_potri<__VA_ARGS__>(Arguments&);

INSTANTIATE(TESTING_POTRI, FOREACH_BOOLEAN_0, FOREACH_BOOLEAN_1, FOREACH_SCALAR_TYPE, APPLY_STAMP)
