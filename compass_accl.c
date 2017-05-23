#include "compass.h"
#include "acclgyro.h"
#include <math.h>
#include <stdio.h>

static const double PI = 3.14159265359;

int main()
{
  	acclgyro_initializer();
    compass_initializer_knd();
    double acclx_knd = 0;
    double accly_knd = 0;
    double acclz_knd = 0;
    double xcompass_knd = 0;
    double ycompass_knd = 0;
    double zcompass_knd = 0;
    double phi_radian = 0;
    double psi_radian = 0;
    double theta_radian = 0;
    double phi_degree = 0;
    double psi_degree = 0;
    double theta_degree = 0;
    while(1)
    {
      acclx_knd = (double)get_acclx();
      accly_knd = (double)get_accly();
      acclz_knd = (double)get_acclz();
      xcompass_knd = (double)get_xcompass();
      ycompass_knd = (double)get_ycompass();
      zcompass_knd = (double)get_zcompass();
      printf("acclx = %lf", acclx_knd);
      printf("accly = %lf", accly_knd);
      printf("acclz = %lf", acclz_knd);
      printf("compassx = %lf", xcompass_knd);
      printf("compassy = %lf", ycompass_knd);
      printf("compassz = %lf", zcompass_knd);
      phi_radian = atan2(accly_knd, acclz_knd);
      psi_radian = atan2(-acclx_knd, accly_knd*sin(theta) + acclz_knd*cos(theta));
      phi_degree = atan2(accly_knd, acclz_knd)*180.0/PI;
      psi_degree = atan2(-acclx_knd, accly_knd*sin(theta) + acclz_knd*cos(theta))*180.0/PI;
      printf("theta = %lf", phi_degree);
      printf("phi = %lf", psi_degree);
      theta = atan2((acclz_knd*sin(phi_radian) - accly_knd*cos(phi_radian)) / (acclx_knd*cos(psi_radian) + accly_knd*sin(psi_radian)*sin(phi_radian) + acclz_knd*sin(psi_radian)*cos(phi_radian))) * 180.0/PI + 180.0);
      printf("phi = %lf", psi_degree);
    }
}
