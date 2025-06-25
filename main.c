#include "rtc_e_sol.h"
#include "servo.h"
#include <stdio.h>

// Código feito por:
// Philip William -- Código solarimétrico e do RTC
// Anita Cunha, Yara Rodrigues -- Código dos servomotores

// PASSOS LÓGICOS PARA O TRACKER FUNCIONAR
// 1. Inicializar os hardwares relevantes
// 2. Puxar o valor do RTC a partir do rtc.c
// 3. Calcular o ângulo horário solar e ângulo zenital a partir do solarimetria.c
// 4. Girar o motor no ângulo adequado a partir do servo.c
// 5. Esperar 5 minutos para repetir o prodecimento do passo 2-4
// 6. Quando o ângulo zenital passar do horizonte (ou do ângulo de ataque da placa),
// antecipar a posição do dia seguinte e testar em intervalos de 30 minutos até que o sol nasça novamente.

// Valores de entrada
const data_t calibracao = {0, 0, 12, 2, 6, 2025};  // 02/06/2025 12:00:00
const double latitude_local_graus = -23.55;  // São Paulo (negativo para Sul)
const double longitude_local_graus = -46.63;  // São Paulo (negativo para Oeste)
const double longitude_meridiano_padrao_graus = -45.0;  // Para UTC-3 (BRT)

int main(void){
    // 1.
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    RTC_Setup(i2c_dev, &calibracao);  // Liga o relógio, e calibra caso não tenha ocorrido
    servo_inicializar();
    printf("Sistema do tracker solar inicializado.\n");

    while(1){
        // 2.
        RTC_Ler(&agora);
        printf("Data: %02d/%02d/%04d %02d:%02d:%02d\n", agora.dia, agora.mes, agora.ano, agora.hora, agora.minuto, agora.segundo);

        // 3.
        double omega_atual_graus = angulo_horario_solar();
        double zenital_atual_graus = angulo_zenital();
        if(omega_atual_graus < -900.0 || zenital_atual_graus < -900.0){
            printf("Erro ao calcular o ângulo omega e zenital.\n");
            delay_ms(60000);
            continue;
        }
        printf("Calculado: Omega = %.2f graus, Zenital = %.2f graus.\n", omega_atual_graus, zenital_atual_graus);

        // 4.
        double angulo_servo_alvo = 90.0;
        if(zenital_atual_graus < 90.0){
            angulo_servo_alvo = omega_atual_graus + 90.0;
            if(angulo_servo_alvo < 0.0) angulo_servo_alvo = 0;
            if(angulo_servo_alvo > 180.0) angulo_servo_alvo = 180.0;

            servo_definir_angulo(angulo_servo_alvo);
            printf("Servo: %.1f graus (Sol acima do horizonte)\n", angulo_servo_alvo);

            // 5.
            delay_ms(300000);
        }
        else{
            angulo_servo_alvo = 0.0;
            servo_definir_angulo(angulo_servo_alvo);
            printf("Servo: %.1f graus (Sol abaixo do horizonte, posicao de espera)\n", angulo_servo_alvo);

            // 6.
            while(1){
                printf("Modo noturno. Aguardando 30 minutos para nova verificação.\n");
                delay_ms(30*60*1000);

                ler_RTC(&agora);
                printf("Verificação noturna RTC: %02d/%02d %02d:%02d\n", agora.dia, agora.mes, agora.hora, agora.minuto);

                if(agora.hora>=4 && agora.hora<=8){
                    double omega_check = angulo_horario_solar();
                    double zenital_check = angulo_zenital();
                    printf("Check matinal: Omega=%.2f, Zenital=%.2f\n", omega_check, zenital_check);

                    if(zenital_check < 90.0 && zenital_check >= 0.0){
                        printf("Sol detectado. Retornando operações.\n");
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
