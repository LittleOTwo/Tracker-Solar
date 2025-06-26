#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>

// Define o período do PWM para 50Hz (20ms) com Prescaler de 128 e clock de ~24MHz
#define SERVO_PERIODO_TPM_MODULO 7500 

// !!! IMPORTANTE: AJUSTE ESTES PINOS PARA SEU HARDWARE !!!
// Defina a PORTA e o PINO onde o fio de SINAL do seu servo está conectado.
// Exemplo para PTD0:
#define SERVO_GPIO_PORTA GPIOD 
#define SERVO_GPIO_PINO  0     
// Verifique como sua biblioteca "pwm.h" espera que estes sejam definidos.

/**
 * @brief Inicializa o PWM para o servomotor.
 */
void servo_inicializar(void);

/**
 * @brief Define o ângulo do servomotor.
 * @param angulo Ângulo desejado em graus (0.0 a 180.0).
 */
void servo_definir_angulo(double angulo);

#endif /* SERVO_H_ */
