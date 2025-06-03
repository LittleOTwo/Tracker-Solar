#include "rtc_e_sol.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int cor[12]={0,1,-1,0,0,1,1,2,3,3,4,4};

int eh_bissexto(int ano){
    if ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0)){
        return 1;
    }
    else return 0;
}

int calcular_n(int dia, int mes, int ano){
    return dia + (mes - 1) * 30 + cor[mes - 1] + eh_bissexto(ano);
}

double para_radianos(double graus){
    return graus * (M_PI / 180.0);
}

double para_graus(double radianos){
    return radianos * (180.0 / M_PI);
}

double angulo_horario_solar(){
    int n = calcular_n(agora.dia, agora.mes, agora.ano);

    double B_rad = (2.0 * M_PI * (n - 81.0)) / 364.0;  // B em radianos

    double EoT_minutos = 9.87 * sin(2.0 * B_rad) - 7.53 * cos(B_rad) - 1.5 * sin(B_rad);  

    double hcl_decimal = agora.hora + (agora.minuto / 60.0) + (agora.segundo / 3600.0);

    double delta_t_longitude_minutos = 4.0 * (longitude_meridiano_padrao_graus - longitude_local_graus);

    double tsl_horas = hcl_decimal + (EoT_minutos + delta_t_longitude_minutos) / 60.0;

    double omega_ahs_graus = 15.0 * (tsl_horas - 12.0);  // Ângulo horário solar
    return omega_ahs_graus;
}

double angulo_zenital(){
    int n = calcular_n(agora.dia, agora.mes, agora.ano);
    double omega_ahs_graus = angulo_horario_solar();
    double argumento_declinacao_graus = (360.0 / 365.0) * (284.0 + n);
    double argumento_declinacao_rad = para_radianos(argumento_declinacao_graus);
    double delta_ds_graus = 23.45 * sin(argumento_declinacao_rad);  // Declinação solar

    double latitude_rad = para_radianos(latitude_local_graus);
    double delta_ds_rad = para_radianos(delta_ds_graus);
    double omega_ah_rad = para_radianos(omega_ahs_graus);

    double cos_theta_z = sin(latitude_rad) * sin(delta_ds_rad) + cos(latitude_rad) * cos(delta_ds_rad) * cos(omega_ah_rad);

    // Garantir que o argumento de acos está no intervalo [-1.0, 1.0]
    if (cos_theta_z > 1.0) cos_theta_z = 1.0;
    if (cos_theta_z < -1.0) cos_theta_z = -1.0;

    double angulo_zenital_rad = acos(cos_theta_z);
    double angulo_zenital_graus = para_graus(angulo_zenital_rad);
    return angulo_zenital_graus;
}