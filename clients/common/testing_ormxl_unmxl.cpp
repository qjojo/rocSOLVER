
#include <testing_ormxl_unmxl.hpp>

#define TESTING_ORMXL_UNMXL(...) template void testing_ormxl_unmxl<__VA_ARGS__>(Arguments&);

INSTANTIATE(TESTING_ORMXL_UNMXL, FOREACH_SCALAR_TYPE, FOREACH_BOOLEAN_INT, APPLY_STAMP)
