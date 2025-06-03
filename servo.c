#include "servo.h" 
#include "pwm.h"   // Sua biblioteca PWM customizada
#include <stdio.h> // Para printf de ERRO apenas

// NOTA: Constantes como TPM0_BASE, TPM0, TPM_PLLFLL, TPM_CLK, PS_128, EDGE_PWM, TPM_PWM_H
// DEVEM ser definidas pela sua biblioteca "pwm.h" ou pelos includes que ela faz.

void servo_inicializar(void) {
    // Inicializa o módulo TPM (ex: TPM0) para operar em 50Hz
    if (pwm_tpm_Init(TPM0_BASE, TPM_PLLFLL, SERVO_PERIODO_TPM_MODULO, TPM_CLK, PS_128, EDGE_PWM) != 0) {
        printf("ERRO: Falha ao inicializar TPM0 para Servo!\n");
        return;
    }

    // Inicializa o Canal do TPM (ex: TPM0, Canal 0) para o pino do servo
    if (pwm_tpm_Ch_Init(TPM0, 0, TPM_PWM_H, SERVO_GPIO_PORTA, SERVO_GPIO_PINO) != 0) {
        printf("ERRO: Falha ao inicializar Canal 0 do TPM0 para Servo!\n");
        return;
    }
}

void servo_definir_angulo(double angulo) {
    uint16_t valor_cnv;

    if (angulo < 0.0) angulo = 0.0;
    if (angulo > 180.0) angulo = 180.0;

    // Converte ângulo (0-180 graus) para valor de contagem do TPM (para pulsos de 1ms-2ms)
    // 1ms de pulso = 5% do período de 20ms. 2ms de pulso = 10% do período de 20ms.
    double cnv_min = 0.05 * SERVO_PERIODO_TPM_MODULO; 
    double cnv_max = 0.10 * SERVO_PERIODO_TPM_MODULO; 

    valor_cnv = (uint16_t)(cnv_min + (angulo / 180.0) * (cnv_max - cnv_min));

    // Define o valor do duty cycle (largura do pulso) no canal do TPM
    if (pwm_tpm_CnV(TPM0, 0, valor_cnv) != 0) {
        printf("ERRO: Falha ao definir CnV do Servo!\n");
    }
}