#ifndef RTC_H_
#define RTC_H_
#include <stdint.h>

// Definir um novo tipo de função chamado data_t, que agrupa todos os valores relevantes
// em uma variável só.
typedef struct{
    int segundo;
    int minuto;
    int hora;
    int dia;
    int mes;
    int ano;
} data_t;

extern data_t agora;
extern const data_t calibracao;
extern const double latitude_local_graus; // São Paulo (negativo para Sul)
extern const double longitude_local_graus; // São Paulo (negativo para Oeste)
extern const double longitude_meridiano_padrao_graus; // Para UTC-3 (BRT)

// Funções fornecidas pela biblioteca
void I2C0_Init();
void RTC_Verificar_E_Configurar_Se_Necessario(data_t *valores_calibracao_padrao);
void configurar_RTC(data_t *calibracao);
void ler_RTC(data_t *agora);
void delay_ms(volatile uint32_t ms);

double angulo_horario_solar(void);
double angulo_zenital(void);

#endif /* RTC_H_ */