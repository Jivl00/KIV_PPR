#include "my_utils.h"

void set_num_threads(bool parallel) {
    if (parallel) {
        omp_set_num_threads(omp_get_max_threads());
    } else {
        omp_set_num_threads(1);
    }
}