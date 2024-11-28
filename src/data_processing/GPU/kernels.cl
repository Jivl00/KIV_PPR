__kernel void abs_diff_calc(__global const double* arr, __global double* diff, const int median) {
    int id = get_global_id(0);
    diff[id] = fabs(arr[id] - median);
}