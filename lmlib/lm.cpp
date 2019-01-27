/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif

#include <string.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_statistics_double.h>

typedef void* lmlib_lmsolver;

struct lmlib_lmsolver_internal {
  gsl_vector* y;
  gsl_matrix* x;
  gsl_vector* c;  // the coefficients
  gsl_matrix* cov;
  gsl_multifit_linear_workspace* work;
  size_t n_size;  // number of serial */
  size_t p_size;  // number of parameter */
  double chisq;

  void init(size_t serial_size, size_t x_size) {
    n_size = serial_size;
    p_size = x_size;

    /** input */
    y = gsl_vector_alloc(n_size);
    x = gsl_matrix_alloc(n_size, p_size);

    /** output */
    c = gsl_vector_alloc(p_size);
    cov = gsl_matrix_alloc(p_size, p_size);

    work = gsl_multifit_linear_alloc(n_size, p_size);
  }

  double calc_rsq(const double* y_val, const double* x_val) {
    memcpy(y->data, y_val, sizeof(double) * n_size);
    memcpy(x->data, x_val, sizeof(double) * n_size * p_size);

    // now do the fit
    gsl_multifit_linear(x, y, c, cov, &chisq, work);

    double tss = gsl_stats_tss(y->data, 1, n_size);
    double rsq = 1 - chisq / tss;
    // printf("chisq[%lf] rsquared:%lf \n", chisq, rsq);
    return rsq;
  }
};

extern "C" LMAPI_EXPORT lmlib_lmsolver lmlib_lmsolver_open(size_t n, size_t p) {
  lmlib_lmsolver_internal* solver = new lmlib_lmsolver_internal;
  solver->init(n, p);

  return solver;
}

extern "C" LMAPI_EXPORT double lmlib_lmsolver_rsqured(lmlib_lmsolver lms,
                                                      const double* y_val,
                                                      const double* x_val) {
  auto solver = reinterpret_cast<lmlib_lmsolver_internal*>(lms);
  return solver->calc_rsq(y_val, x_val);
}

extern "C" LMAPI_EXPORT void lmlib_lmsolver_close(lmlib_lmsolver lms) {
  auto solver = reinterpret_cast<lmlib_lmsolver_internal*>(lms);
  delete solver;
}

extern "C" LMAPI_EXPORT int lmlm(const double* y_val, const double* x_val,
                                 int y_size, int x_size, double* rsqured) {
  gsl_vector* y;
  gsl_matrix* x;
  gsl_vector* c;  // the coefficients
  gsl_matrix* cov;

  int p;
  int n = y_size;

  p = x_size / y_size + 1;

  /** input */
  y = gsl_vector_alloc(n);
  memcpy(y->data, y_val, sizeof(double) * n);

  x = gsl_matrix_alloc(n, p);
  // memcpy(x->data, x_val, sizeof(double) * n);
  for (int i = 0; i < n; ++i) {
    gsl_matrix_set(x, i, 0, static_cast<double>(1));
    double* rowx = x->data + p * i + 1;
    const double* valx = x_val + (p - 1) * i;
    memcpy(rowx, valx, sizeof(double) * (p - 1));
  }

  // output
  c = gsl_vector_alloc(p);
  cov = gsl_matrix_alloc(p, p);

  double chisq;
  // allocate temporary work space for gsl
  gsl_multifit_linear_workspace* work;
  work = gsl_multifit_linear_alloc(n, p);

  // now do the fit
  gsl_multifit_linear(x, y, c, cov, &chisq, work);

  //  for (int i = 0; i < p; ++i) {
  //    printf("c[%d]: %lf\n", i, c->data[i]);
  //  }
  //  printf("chisq:%lf\n", chisq);

  //  for (int i = 0; i < n; ++i) {
  //    double r = c->data[0] + c->data[1] * x_val[i * 3 + 0] +
  //               c->data[2] * x_val[i * 3 + 1] + c->data[3] * x_val[i * 3 +
  //               2];

  //    printf("ori %lf  y=%lf\n", y_val[i], r);
  //  }
  double tss = gsl_stats_tss(y_val, 1, n);
  double rsq = 1 - chisq / tss;
  // printf("chisq[%lf] rsquared:%lf \n", chisq, rsq);
  if (rsqured) {
    *rsqured = rsq;
  }

  //  for (int i = 0; i < p; ++i) {
  //    for (int j = 0; j < p; ++j) {
  //      printf("%lf\t", gsl_matrix_get(cov, i, j));
  //    }
  //    printf("\n");
  //  }

  // clean data
  gsl_multifit_linear_free(work);
  gsl_matrix_free(x);
  gsl_matrix_free(cov);
  gsl_vector_free(y);
  gsl_vector_free(c);

  return 0;
}
