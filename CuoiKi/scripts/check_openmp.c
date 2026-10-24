// Kiểm tra môi trường OpenMP: phiên bản, số luồng.
// Build: gcc -O2 -fopenmp check_openmp.c -o check_openmp
#include <omp.h>
#include <stdio.h>

int main(void) {
    printf("_OPENMP = %d\n", _OPENMP);
    printf("omp_get_num_procs()   = %d\n", omp_get_num_procs());
    printf("omp_get_max_threads() = %d\n", omp_get_max_threads());
    #pragma omp parallel
    {
        #pragma omp single
        printf("threads in parallel region = %d\n", omp_get_num_threads());
    }
    return 0;
}
