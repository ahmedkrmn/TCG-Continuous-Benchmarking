/*
 *  Source file of a benchmark program for libm functions.
 *
 *  This file is a part of the project "TCG Continuous Benchmarking".
 *
 *  Copyright (C) 2020  Ahmed Karaman <ahmedkhaledkaraman@gmail.com>
 *  Copyright (C) 2020  Aleksandar Markovic <aleksandar.qemu.devel@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define RANDOM_DATA_LEN 100

/**
 * Arrays for storing random inputs. For now, only random inputs between
 * 0.0 and 1.0 are supported. There are two separate sets of inputs for each
 * floating point type: 'float', 'double' and 'long double'.
 */
static float f_rnd1_0_to_1[RANDOM_DATA_LEN];
static float f_rnd2_0_to_1[RANDOM_DATA_LEN];
static double d_rnd1_0_to_1[RANDOM_DATA_LEN];
static double d_rnd2_0_to_1[RANDOM_DATA_LEN];
static long double l_rnd1_0_to_1[RANDOM_DATA_LEN];
static long double l_rnd2_0_to_1[RANDOM_DATA_LEN];

/**
 * Arrays for storing results. They can be of type 'int' (the only functions
 * where this is the case are ilogbl(), ilogb(), and ilogbl()), 'float',
 * 'double' and 'long double'.
 */
static int i_result[RANDOM_DATA_LEN];
static float f_result[RANDOM_DATA_LEN];
static double d_result[RANDOM_DATA_LEN];
static long double l_result[RANDOM_DATA_LEN];

/**
 * Utility function for calculating elapsed time (ms) as a difference between
 * times derived from two 'timespec' objects.
 */
static int64_t get_elapsed_time(struct timespec* time_start,
                                struct timespec* time_stop) {
    struct timespec time_diff;
    uint64_t elapsed_time;

    if ((time_stop->tv_nsec - time_start->tv_nsec) < 0) {
        time_diff.tv_sec = time_stop->tv_sec - time_start->tv_sec - 1;
        time_diff.tv_nsec = time_stop->tv_nsec - time_start->tv_nsec +
                            1000000000;
    } else {
        time_diff.tv_sec = time_stop->tv_sec - time_start->tv_sec;
        time_diff.tv_nsec = time_stop->tv_nsec - time_start->tv_nsec;
    }
    elapsed_time = time_diff.tv_sec * 1000 +
                   time_diff.tv_nsec / 1000000;

    return elapsed_time;
}

/**
 * Macros for creating wrappers for measuring elapsed time for libm math
 * functions whose base floating point type is 'float'.
 */
#define TEST_MATH_FUNCTION_F_F(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                f_result[j] = FNAME(f_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            f_result[0] += f_result[i];                          \
        }                                                        \
        printf("Control result: %f\n", f_result[0]);             \
    }

#define TEST_MATH_FUNCTION_I_F(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                i_result[j] = FNAME(f_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            i_result[0] += i_result[i];                          \
        }                                                        \
        printf("Control result: %d\n", i_result[0]);             \
    }

#define TEST_MATH_FUNCTION_F_FF(FNAME)                                   \
    void test_loop_##FNAME(int32_t n) {                                  \
        struct timespec time_start, time_end;                            \
        uint64_t elapsed_time;                                           \
                                                                         \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);                 \
        printf("* Profiling function %s():\t", #FNAME);                  \
        for (size_t i = 0; i < n; ++i) {                                 \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {               \
                f_result[j] = FNAME(f_rnd1_0_to_1[j], f_rnd2_0_to_1[j]); \
            }                                                            \
        }                                                                \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);                   \
                                                                         \
        elapsed_time = get_elapsed_time(&time_start, &time_end);         \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);          \
                                                                         \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {                   \
            f_result[0] += f_result[i];                                  \
        }                                                                \
        printf("Control result: %f\n", f_result[0]);                     \
    }

/* create source code of numerous functions using macros above */

TEST_MATH_FUNCTION_F_F(sqrtf)
TEST_MATH_FUNCTION_F_F(cbrtf)
TEST_MATH_FUNCTION_F_FF(hypotf)
TEST_MATH_FUNCTION_F_FF(powf)

TEST_MATH_FUNCTION_F_F(expf)
TEST_MATH_FUNCTION_F_F(exp2f)
TEST_MATH_FUNCTION_F_F(expm1f)
TEST_MATH_FUNCTION_F_F(logf)
TEST_MATH_FUNCTION_F_F(log2f)
TEST_MATH_FUNCTION_F_F(log10f)
TEST_MATH_FUNCTION_F_F(log1pf)
TEST_MATH_FUNCTION_F_F(ilogbf)
TEST_MATH_FUNCTION_F_F(logbf)

TEST_MATH_FUNCTION_F_F(sinf)
TEST_MATH_FUNCTION_F_F(cosf)
TEST_MATH_FUNCTION_F_F(tanf)
TEST_MATH_FUNCTION_F_F(asinf)
TEST_MATH_FUNCTION_F_F(acosf)
TEST_MATH_FUNCTION_F_F(atanf)
TEST_MATH_FUNCTION_F_FF(atan2f)

TEST_MATH_FUNCTION_F_F(sinhf)
TEST_MATH_FUNCTION_F_F(coshf)
TEST_MATH_FUNCTION_F_F(tanhf)
TEST_MATH_FUNCTION_F_F(asinhf)
TEST_MATH_FUNCTION_F_F(acoshf)
TEST_MATH_FUNCTION_F_F(atanhf)

TEST_MATH_FUNCTION_F_F(erff)
TEST_MATH_FUNCTION_F_F(erfcf)
TEST_MATH_FUNCTION_F_F(lgammaf)
TEST_MATH_FUNCTION_F_F(tgammaf)

/**
 * Macros for creating wrappers for measuring elapsed time for libm math
 * functions whose base floating point type is 'double'.
 */
#define TEST_MATH_FUNCTION_D_D(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                d_result[j] = FNAME(d_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            d_result[0] += d_result[i];                          \
        }                                                        \
        printf("Control result: %f\n", d_result[0]);             \
    }

#define TEST_MATH_FUNCTION_I_D(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                i_result[j] = FNAME(d_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            i_result[0] += i_result[i];                          \
        }                                                        \
        printf("Control result: %d\n", i_result[0]);             \
    }

#define TEST_MATH_FUNCTION_D_DD(FNAME)                                   \
    void test_loop_##FNAME(int32_t n) {                                  \
        struct timespec time_start, time_end;                            \
        uint64_t elapsed_time;                                           \
                                                                         \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);                 \
        printf("* Profiling function %s():\t", #FNAME);                  \
        for (size_t i = 0; i < n; ++i) {                                 \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {               \
                d_result[j] = FNAME(d_rnd1_0_to_1[j], d_rnd2_0_to_1[j]); \
            }                                                            \
        }                                                                \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);                   \
                                                                         \
        elapsed_time = get_elapsed_time(&time_start, &time_end);         \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);          \
                                                                         \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {                   \
            d_result[0] += d_result[i];                                  \
        }                                                                \
        printf("Control result: %f\n", d_result[0]);                     \
    }

/* create source code of numerous functions using macros above */

TEST_MATH_FUNCTION_D_D(sqrt)
TEST_MATH_FUNCTION_D_D(cbrt)
TEST_MATH_FUNCTION_D_DD(hypot)
TEST_MATH_FUNCTION_D_DD(pow)

TEST_MATH_FUNCTION_D_D(exp)
TEST_MATH_FUNCTION_D_D(exp2)
TEST_MATH_FUNCTION_D_D(expm1)
TEST_MATH_FUNCTION_D_D(log)
TEST_MATH_FUNCTION_D_D(log2)
TEST_MATH_FUNCTION_D_D(log10)
TEST_MATH_FUNCTION_D_D(log1p)
TEST_MATH_FUNCTION_D_D(ilogb)
TEST_MATH_FUNCTION_D_D(logb)

TEST_MATH_FUNCTION_D_D(sin)
TEST_MATH_FUNCTION_D_D(cos)
TEST_MATH_FUNCTION_D_D(tan)
TEST_MATH_FUNCTION_D_D(asin)
TEST_MATH_FUNCTION_D_D(acos)
TEST_MATH_FUNCTION_D_D(atan)
TEST_MATH_FUNCTION_D_DD(atan2)

TEST_MATH_FUNCTION_D_D(sinh)
TEST_MATH_FUNCTION_D_D(cosh)
TEST_MATH_FUNCTION_D_D(tanh)
TEST_MATH_FUNCTION_D_D(asinh)
TEST_MATH_FUNCTION_D_D(acosh)
TEST_MATH_FUNCTION_D_D(atanh)

TEST_MATH_FUNCTION_D_D(erf)
TEST_MATH_FUNCTION_D_D(erfc)
TEST_MATH_FUNCTION_D_D(lgamma)
TEST_MATH_FUNCTION_D_D(tgamma)

/**
 * Macros for creating wrappers for measuring elapsed time for libm math
 * functions whose base floating point type is 'long double'.
 */
#define TEST_MATH_FUNCTION_L_L(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                l_result[j] = FNAME(l_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            l_result[0] += l_result[i];                          \
        }                                                        \
        printf("Control result: %Lf\n", l_result[0]);            \
    }

#define TEST_MATH_FUNCTION_I_L(FNAME)                            \
    void test_loop_##FNAME(int32_t n) {                          \
        struct timespec time_start, time_end;                    \
        uint64_t elapsed_time;                                   \
                                                                 \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);         \
        printf("* Profiling function %s():\t", #FNAME);          \
        for (size_t i = 0; i < n; ++i) {                         \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {       \
                i_result[j] = FNAME(l_rnd1_0_to_1[j]);           \
            }                                                    \
        }                                                        \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);           \
                                                                 \
        elapsed_time = get_elapsed_time(&time_start, &time_end); \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);  \
                                                                 \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {           \
            i_result[0] += i_result[i];                          \
        }                                                        \
        printf("Control result: %i\n", i_result[0]);             \
    }

#define TEST_MATH_FUNCTION_L_LL(FNAME)                                   \
    void test_loop_##FNAME(int32_t n) {                                  \
        struct timespec time_start, time_end;                            \
        uint64_t elapsed_time;                                           \
                                                                         \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);                 \
        printf("* Profiling function %s():\t", #FNAME);                  \
        for (size_t i = 0; i < n; ++i) {                                 \
            for (size_t j = 0; j < RANDOM_DATA_LEN; ++j) {               \
                l_result[j] = FNAME(l_rnd1_0_to_1[j], l_rnd2_0_to_1[j]); \
            }                                                            \
        }                                                                \
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);                   \
                                                                         \
        elapsed_time = get_elapsed_time(&time_start, &time_end);         \
        printf("Elapsed time: %" PRIu64 " ms\t", elapsed_time);          \
                                                                         \
        for (size_t i = 1; i < RANDOM_DATA_LEN; i++) {                   \
            l_result[0] += l_result[i];                                  \
        }                                                                \
        printf("Control result: %Lf\n", l_result[0]);                    \
    }

/* create source code of numerous functions using macros above */

TEST_MATH_FUNCTION_L_L(sqrtl)
TEST_MATH_FUNCTION_L_L(cbrtl)
TEST_MATH_FUNCTION_L_LL(hypotl)
TEST_MATH_FUNCTION_L_LL(powl)

TEST_MATH_FUNCTION_L_L(expl)
TEST_MATH_FUNCTION_L_L(exp2l)
TEST_MATH_FUNCTION_L_L(expm1l)
TEST_MATH_FUNCTION_L_L(logl)
TEST_MATH_FUNCTION_L_L(log2l)
TEST_MATH_FUNCTION_L_L(log10l)
TEST_MATH_FUNCTION_L_L(log1pl)
TEST_MATH_FUNCTION_L_L(ilogbl)
TEST_MATH_FUNCTION_L_L(logbl)

TEST_MATH_FUNCTION_L_L(sinl)
TEST_MATH_FUNCTION_L_L(cosl)
TEST_MATH_FUNCTION_L_L(tanl)
TEST_MATH_FUNCTION_L_L(asinl)
TEST_MATH_FUNCTION_L_L(acosl)
TEST_MATH_FUNCTION_L_L(atanl)
TEST_MATH_FUNCTION_L_LL(atan2l)

TEST_MATH_FUNCTION_L_L(sinhl)
TEST_MATH_FUNCTION_L_L(coshl)
TEST_MATH_FUNCTION_L_L(tanhl)
TEST_MATH_FUNCTION_L_L(asinhl)
TEST_MATH_FUNCTION_L_L(acoshl)
TEST_MATH_FUNCTION_L_L(atanhl)

TEST_MATH_FUNCTION_L_L(erfl)
TEST_MATH_FUNCTION_L_L(erfcl)
TEST_MATH_FUNCTION_L_L(lgammal)
TEST_MATH_FUNCTION_L_L(tgammal)

void main(int argc, char* argv[]) {
    /******************************** Parse the command line flags ********************************/
    int c;
    long selected_iterations = -1;
    char* selected_type = NULL;
    char* selected_function = NULL;
    while ((c = getopt(argc, argv, "i:f:t:")) != -1) {
        switch (c) {
            case 'i':
                selected_iterations = atoi(optarg);
                /* Check for valid numbers (not strings and greater than 0) */
                if (selected_iterations < 1) {
                    fprintf(stderr, "Error ... Number of iterations must be greater than 0.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                selected_function = optarg;
                break;
            case 't':
                /* Check for valid data type */
                if (strcmp(optarg, "float") && strcmp(optarg, "double") && strcmp(optarg, "longdouble")) {
                    fprintf(stderr, "Error ... Data type must either be 'float', 'double' or 'longdouble'.\n");
                    exit(EXIT_FAILURE);
                }
                selected_type = optarg;
                break;
            default:
                break;
        }
    }
    /* Validated that the number of iterations was selected */
    if (selected_iterations == -1) {
        fprintf(stderr, "Error ... Please provide number of iterations using '-i n' before benchmarking.\n");
        exit(EXIT_FAILURE);
    }
    /* Validated that only one of the '-f' and '-t' flags was used*/
    if (selected_type && selected_function) {
        fprintf(stderr, "Error ... Flags '-f' and '-t' can't be used together.\n");
        exit(EXIT_FAILURE);
    }
    /**********************************************************************************************/

    /*
     * flags indicating testing underlying data type
     */
    bool d_float = true;
    bool d_double = true;
    bool d_longdouble = true;

    /* Modify the flags accroding to the selected data type */
    if (selected_type != NULL) {
        if (!strcmp(selected_type, "float")) {
            d_double = false;
            d_longdouble = false;
        } else if (!strcmp(selected_type, "double")) {
            d_float = false;
            d_longdouble = false;
        } else if (!strcmp(selected_type, "longdouble")) {
            d_float = false;
            d_double = false;
        }
    }

    /*
     * flags indicating testing a single libm function
     */
    bool default_flag = selected_function == NULL;

    /* group 'power' */
    /* data type 'float'*/
    bool f_sqrtf = default_flag || !strcmp(selected_function, "sqrtf");
    bool f_cbrtf = default_flag || !strcmp(selected_function, "cbrtf");
    bool f_hypotf = default_flag || !strcmp(selected_function, "hypotf");
    bool f_powf = default_flag || !strcmp(selected_function, "powf");
    /* data type 'double'*/
    bool f_sqrt = default_flag || !strcmp(selected_function, "sqrt");
    bool f_cbrt = default_flag || !strcmp(selected_function, "cbrt");
    bool f_hypot = default_flag || !strcmp(selected_function, "hypot");
    bool f_pow = default_flag || !strcmp(selected_function, "pow");
    /* data type 'long double'*/
    bool f_sqrtl = default_flag || !strcmp(selected_function, "sqrtl");
    bool f_cbrtl = default_flag || !strcmp(selected_function, "cbrtl");
    bool f_hypotl = default_flag || !strcmp(selected_function, "hypotl");
    bool f_powl = default_flag || !strcmp(selected_function, "powl");

    /* group 'exponential' */
    /* data type 'float'*/
    bool f_expf = default_flag || !strcmp(selected_function, "expf");
    bool f_exp2f = default_flag || !strcmp(selected_function, "exp2f");
    bool f_expm1f = default_flag || !strcmp(selected_function, "expm1f");
    bool f_logf = default_flag || !strcmp(selected_function, "logf");
    bool f_log2f = default_flag || !strcmp(selected_function, "log2f");
    bool f_log10f = default_flag || !strcmp(selected_function, "log10f");
    bool f_log1pf = default_flag || !strcmp(selected_function, "log1pf");
    bool f_ilogbf = default_flag || !strcmp(selected_function, "ilogbf");
    bool f_logbf = default_flag || !strcmp(selected_function, "logbf");
    /* data type 'double'*/
    bool f_exp = default_flag || !strcmp(selected_function, "exp");
    bool f_exp2 = default_flag || !strcmp(selected_function, "exp2");
    bool f_expm1 = default_flag || !strcmp(selected_function, "expm1");
    bool f_log = default_flag || !strcmp(selected_function, "log");
    bool f_log2 = default_flag || !strcmp(selected_function, "log2");
    bool f_log10 = default_flag || !strcmp(selected_function, "log10");
    bool f_log1p = default_flag || !strcmp(selected_function, "log1p");
    bool f_ilogb = default_flag || !strcmp(selected_function, "ilogb");
    bool f_logb = default_flag || !strcmp(selected_function, "logb");
    /* data type 'long double'*/
    bool f_expl = default_flag || !strcmp(selected_function, "expl");
    bool f_exp2l = default_flag || !strcmp(selected_function, "exp2l");
    bool f_expm1l = default_flag || !strcmp(selected_function, "expm1l");
    bool f_logl = default_flag || !strcmp(selected_function, "logl");
    bool f_log2l = default_flag || !strcmp(selected_function, "log2l");
    bool f_log10l = default_flag || !strcmp(selected_function, "log10l");
    bool f_log1pl = default_flag || !strcmp(selected_function, "log1pl");
    bool f_ilogbl = default_flag || !strcmp(selected_function, "ilogbl");
    bool f_logbl = default_flag || !strcmp(selected_function, "logbl");

    /* group 'trigonometric' */
    /* data type 'float' */
    bool f_sinf = default_flag || !strcmp(selected_function, "sinf");
    bool f_cosf = default_flag || !strcmp(selected_function, "cosf");
    bool f_tanf = default_flag || !strcmp(selected_function, "tanf");
    bool f_asinf = default_flag || !strcmp(selected_function, "asinf");
    bool f_acosf = default_flag || !strcmp(selected_function, "acosf");
    bool f_atanf = default_flag || !strcmp(selected_function, "atanf");
    bool f_atan2f = default_flag || !strcmp(selected_function, "atan2f");
    /* data type 'double' */
    bool f_sin = default_flag || !strcmp(selected_function, "sin");
    bool f_cos = default_flag || !strcmp(selected_function, "cos");
    bool f_tan = default_flag || !strcmp(selected_function, "tan");
    bool f_asin = default_flag || !strcmp(selected_function, "asin");
    bool f_acos = default_flag || !strcmp(selected_function, "acos");
    bool f_atan = default_flag || !strcmp(selected_function, "atan");
    bool f_atan2 = default_flag || !strcmp(selected_function, "atan2");
    /* data type 'long double' */
    bool f_sinl = default_flag || !strcmp(selected_function, "sinl");
    bool f_cosl = default_flag || !strcmp(selected_function, "cosl");
    bool f_tanl = default_flag || !strcmp(selected_function, "tanl");
    bool f_asinl = default_flag || !strcmp(selected_function, "asinl");
    bool f_acosl = default_flag || !strcmp(selected_function, "acosl");
    bool f_atanl = default_flag || !strcmp(selected_function, "atanl");
    bool f_atan2l = default_flag || !strcmp(selected_function, "atan2l");

    /* group 'hyperbolic' */
    /* data type 'float' */
    bool f_sinhf = default_flag || !strcmp(selected_function, "sinhf");
    bool f_coshf = default_flag || !strcmp(selected_function, "coshf");
    bool f_tanhf = default_flag || !strcmp(selected_function, "tanhf");
    bool f_asinhf = default_flag || !strcmp(selected_function, "asinhf");
    bool f_acoshf = default_flag || !strcmp(selected_function, "acoshf");
    bool f_atanhf = default_flag || !strcmp(selected_function, "atanhf");
    /* data type 'double' */
    bool f_sinh = default_flag || !strcmp(selected_function, "sinh");
    bool f_cosh = default_flag || !strcmp(selected_function, "cosh");
    bool f_tanh = default_flag || !strcmp(selected_function, "tanh");
    bool f_asinh = default_flag || !strcmp(selected_function, "asinh");
    bool f_acosh = default_flag || !strcmp(selected_function, "acosh");
    bool f_atanh = default_flag || !strcmp(selected_function, "atanh");
    /* data type 'long double' */
    bool f_sinhl = default_flag || !strcmp(selected_function, "sinhl");
    bool f_coshl = default_flag || !strcmp(selected_function, "coshl");
    bool f_tanhl = default_flag || !strcmp(selected_function, "tanhl");
    bool f_asinhl = default_flag || !strcmp(selected_function, "asinhl");
    bool f_acoshl = default_flag || !strcmp(selected_function, "acoshl");
    bool f_atanhl = default_flag || !strcmp(selected_function, "atanhl");

    /* group 'error and gamma' */
    /* data type 'float' */
    bool f_erff = default_flag || !strcmp(selected_function, "erff");
    bool f_erfcf = default_flag || !strcmp(selected_function, "erfcf");
    bool f_lgammaf = default_flag || !strcmp(selected_function, "lgammaf");
    bool f_tgammaf = default_flag || !strcmp(selected_function, "tgammaf");
    /* data type 'double' */
    bool f_erf = default_flag || !strcmp(selected_function, "erf");
    bool f_erfc = default_flag || !strcmp(selected_function, "erfc");
    bool f_lgamma = default_flag || !strcmp(selected_function, "lgamma");
    bool f_tgamma = default_flag || !strcmp(selected_function, "tgamma");
    /* data type 'long double' */
    bool f_erfl = default_flag || !strcmp(selected_function, "erfl");
    bool f_erfcl = default_flag || !strcmp(selected_function, "erfcl");
    bool f_lgammal = default_flag || !strcmp(selected_function, "lgammal");
    bool f_tgammal = default_flag || !strcmp(selected_function, "tgammal");

    /* initialize arrays for random inputs */
    srand(1);
    for (size_t i = 0; i < RANDOM_DATA_LEN; ++i) {
        f_rnd1_0_to_1[i] = (float)rand() / (float)RAND_MAX;
        f_rnd2_0_to_1[i] = (float)rand() / (float)RAND_MAX;
        d_rnd1_0_to_1[i] = (double)rand() / (double)RAND_MAX;
        d_rnd2_0_to_1[i] = (double)rand() / (double)RAND_MAX;
        l_rnd1_0_to_1[i] = (long double)rand() / (long double)RAND_MAX;
        l_rnd2_0_to_1[i] = (long double)rand() / (long double)RAND_MAX;
    }

    /* execute measurements */
    if (d_float) {
        if (f_sqrtf) {
            test_loop_sqrtf(selected_iterations);
        }
        if (f_cbrtf) {
            test_loop_cbrtf(selected_iterations);
        }
        if (f_hypotf) {
            test_loop_hypotf(selected_iterations);
        }
        if (f_powf) {
            test_loop_powf(selected_iterations);
        }
        if (f_expf) {
            test_loop_expf(selected_iterations);
        }
        if (f_exp2f) {
            test_loop_exp2f(selected_iterations);
        }
        if (f_expm1f) {
            test_loop_expm1f(selected_iterations);
        }
        if (f_logf) {
            test_loop_logf(selected_iterations);
        }
        if (f_log2f) {
            test_loop_log2f(selected_iterations);
        }
        if (f_log10f) {
            test_loop_log10f(selected_iterations);
        }
        if (f_log1pf) {
            test_loop_log1pf(selected_iterations);
        }
        if (f_ilogbf) {
            test_loop_ilogbf(selected_iterations);
        }
        if (f_logbf) {
            test_loop_logbf(selected_iterations);
        }
        if (f_sinf) {
            test_loop_sinf(selected_iterations);
        }
        if (f_cosf) {
            test_loop_cosf(selected_iterations);
        }
        if (f_tanf) {
            test_loop_tanf(selected_iterations);
        }
        if (f_asinf) {
            test_loop_asinf(selected_iterations);
        }
        if (f_acosf) {
            test_loop_acosf(selected_iterations);
        }
        if (f_atanf) {
            test_loop_atanf(selected_iterations);
        }
        if (f_atan2f) {
            test_loop_atan2f(selected_iterations);
        }
        if (f_sinhf) {
            test_loop_sinhf(selected_iterations);
        }
        if (f_coshf) {
            test_loop_coshf(selected_iterations);
        }
        if (f_tanhf) {
            test_loop_tanhf(selected_iterations);
        }
        if (f_asinhf) {
            test_loop_asinhf(selected_iterations);
        }
        if (f_acoshf) {
            test_loop_acoshf(selected_iterations);
        }
        if (f_atanhf) {
            test_loop_atanhf(selected_iterations);
        }
        if (f_erff) {
            test_loop_erff(selected_iterations);
        }
        if (f_erfcf) {
            test_loop_erfcf(selected_iterations);
        }
        if (f_lgammaf) {
            test_loop_lgammaf(selected_iterations);
        }
        if (f_tgammaf) {
            test_loop_tgammaf(selected_iterations);
        }
    }

    if (d_double) {
        if (f_sqrt) {
            test_loop_sqrt(selected_iterations);
        }
        if (f_cbrt) {
            test_loop_cbrt(selected_iterations);
        }
        if (f_hypot) {
            test_loop_hypot(selected_iterations);
        }
        if (f_pow) {
            test_loop_pow(selected_iterations);
        }
        if (f_exp) {
            test_loop_exp(selected_iterations);
        }
        if (f_exp2) {
            test_loop_exp2(selected_iterations);
        }
        if (f_expm1) {
            test_loop_expm1(selected_iterations);
        }
        if (f_log) {
            test_loop_log(selected_iterations);
        }
        if (f_log2) {
            test_loop_log2(selected_iterations);
        }
        if (f_log10) {
            test_loop_log10(selected_iterations);
        }
        if (f_log1p) {
            test_loop_log1p(selected_iterations);
        }
        if (f_ilogb) {
            test_loop_ilogb(selected_iterations);
        }
        if (f_logb) {
            test_loop_logb(selected_iterations);
        }
        if (f_sin) {
            test_loop_sin(selected_iterations);
        }
        if (f_cos) {
            test_loop_cos(selected_iterations);
        }
        if (f_tan) {
            test_loop_tan(selected_iterations);
        }
        if (f_asin) {
            test_loop_asin(selected_iterations);
        }
        if (f_acos) {
            test_loop_acos(selected_iterations);
        }
        if (f_atan) {
            test_loop_atan(selected_iterations);
        }
        if (f_atan2) {
            test_loop_atan2(selected_iterations);
        }
        if (f_sinh) {
            test_loop_sinh(selected_iterations);
        }
        if (f_cosh) {
            test_loop_cosh(selected_iterations);
        }
        if (f_tanh) {
            test_loop_tanh(selected_iterations);
        }
        if (f_asinh) {
            test_loop_asinh(selected_iterations);
        }
        if (f_acosh) {
            test_loop_acosh(selected_iterations);
        }
        if (f_atanh) {
            test_loop_atanh(selected_iterations);
        }
        if (f_erf) {
            test_loop_erf(selected_iterations);
        }
        if (f_erfc) {
            test_loop_erfc(selected_iterations);
        }
        if (f_lgamma) {
            test_loop_lgamma(selected_iterations);
        }
        if (f_tgamma) {
            test_loop_tgamma(selected_iterations);
        }
    }

    if (d_longdouble) {
        if (f_sqrtl) {
            test_loop_sqrtl(selected_iterations);
        }
        if (f_cbrtl) {
            test_loop_cbrtl(selected_iterations);
        }
        if (f_hypotl) {
            test_loop_hypotl(selected_iterations);
        }
        if (f_powl) {
            test_loop_powl(selected_iterations);
        }
        if (f_expl) {
            test_loop_expl(selected_iterations);
        }
        if (f_exp2l) {
            test_loop_exp2l(selected_iterations);
        }
        if (f_expm1l) {
            test_loop_expm1l(selected_iterations);
        }
        if (f_logl) {
            test_loop_logl(selected_iterations);
        }
        if (f_log2l) {
            test_loop_log2l(selected_iterations);
        }
        if (f_log10l) {
            test_loop_log10l(selected_iterations);
        }
        if (f_log1pl) {
            test_loop_log1pl(selected_iterations);
        }
        if (f_ilogbl) {
            test_loop_ilogbl(selected_iterations);
        }
        if (f_logbl) {
            test_loop_logbl(selected_iterations);
        }
        if (f_sinl) {
            test_loop_sinl(selected_iterations);
        }
        if (f_cosl) {
            test_loop_cosl(selected_iterations);
        }
        if (f_tanl) {
            test_loop_tanl(selected_iterations);
        }
        if (f_asinl) {
            test_loop_asinl(selected_iterations);
        }
        if (f_acosl) {
            test_loop_acosl(selected_iterations);
        }
        if (f_atanl) {
            test_loop_atanl(selected_iterations);
        }
        if (f_atan2l) {
            test_loop_atan2l(selected_iterations);
        }
        if (f_sinhl) {
            test_loop_sinhl(selected_iterations);
        }
        if (f_coshl) {
            test_loop_coshl(selected_iterations);
        }
        if (f_tanhl) {
            test_loop_tanhl(selected_iterations);
        }
        if (f_asinhl) {
            test_loop_asinhl(selected_iterations);
        }
        if (f_acoshl) {
            test_loop_acoshl(selected_iterations);
        }
        if (f_atanhl) {
            test_loop_atanhl(selected_iterations);
        }
        if (f_erfl) {
            test_loop_erfl(selected_iterations);
        }
        if (f_erfcl) {
            test_loop_erfcl(selected_iterations);
        }
        if (f_lgammal) {
            test_loop_lgammal(selected_iterations);
        }
        if (f_tgammal) {
            test_loop_tgammal(selected_iterations);
        }
    }

    return;
}
