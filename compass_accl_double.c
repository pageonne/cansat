#include "compass.h"
#include "acclgyro.h"
#include <math.h>
#include <stdio.h>

static const double PI = 3.14159265359;
static const double convert_to_G = 16384.0;
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
    double phi_degree = 0;
    double psi_degree = 0;
    double y1 = 0;
    double y2 = 0;
    double x1 = 0;
    double x2 = 0;
    double x3 = 0;
    double theta_degree = 0;
    while(1)
    {
      acclx_knd = (double)get_acclx()/convert_to_G*0.1 + acclx_knd*0.9;
      accly_knd = (double)get_accly()/convert_to_G*0.1 + accly_knd*0.9;
      acclz_knd = (double)get_acclz()/convert_to_G*0.1 + acclz_knd*0.9;
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
      psi_radian = atan2(-acclx_knd, accly_knd*sin(phi_radian) + acclz_knd*cos(phi_radian));
      phi_degree = phi_radian*180.0/PI;
      psi_degree = psi_radian*180.0/PI;
      printf("theta = %lf", phi_degree);
      printf("phi = %lf", psi_degree);
      y1 = acclz_knd*sin(phi_radian);
      y2 = accly_knd*cos(phi_radian);
      x1 = acclx_knd*cos(psi_radian);
      x2 = accly_knd*sin(psi_radian)*sin(phi_radian);
      x3 = acclz_knd*sin(psi_radian)*cos(phi_radian);
      print("y1の値は%fです。\n", y1);
      print("y1のアドレスは%pです。\n", &y1);
        print("y2の値は%fです。\n", y2);
      print("y2のアドレスは%pです。\n", &y2);
        print("x1の値は%fです。\n", x1);
      print("x1のアドレスは%pです。\n", &x1);
        print("x2の値は%fです。\n", x2);
      print("x2のアドレスは%pです。\n", &x2);
        print("x3の値は%fです。\n", x3);
      print("x3のアドレスは%pです。\n", &x3);
      theta_degree = atan2(y1 - y2,x1 + x2 + x3)*180.0/PI + 180.0;
      printf("theta = %lf", theta_degree);
    }
}
