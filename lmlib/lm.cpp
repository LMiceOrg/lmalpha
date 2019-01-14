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

extern "C" LMAPI_EXPORT int lm(const double* y_val, const double* x_val,
                               int y_size, int x_size) {
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
  printf("chisq[%lf] rsquared:%lf \n", chisq, 1 - chisq / tss);

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
