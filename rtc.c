#include "MKL25Z4.h"
#include "rtc_e_sol.h"
#include <stdio.h>
#include <stdint.h>
#define RTC_ADDR 0x68
#define ENDERECO_FLAG_RTC_RAM 0x08 // Ex: Primeiro byte (posição) da RAM do usuário
#define VALOR_FLAG_RTC_CONFIGURADO 0xA5 // "Número mágico"

// Horário do relógio
data_t agora;

// Funções de conversão
uint8_t bcd_to_dec(uint8_t val){
    return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t dec_to_bcd(uint8_t val){
    return ((val / 10) << 4) | (val % 10);
}

// Funções do I2C
void I2C0_Init(void){
    SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    PORTE->PCR[24] |= PORT_PCR_MUX(5);  // PTE24 - SCL
    PORTE->PCR[25] |= PORT_PCR_MUX(5);  // PTE25 - SDA

    I2C0->F = 0x27;  // ~100 kHz
    I2C0->C1 = I2C_C1_IICEN_MASK;
}

void I2C0_Start(void){
    I2C0->C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK;
}

void I2C0_Stop(void){
    I2C0->C1 &= ~I2C_C1_MST_MASK;
    I2C0->C1 &= ~I2C_C1_TX_MASK;
}

void I2C0_Wait(void){
    while (!(I2C0->S & I2C_S_IICIF_MASK));
    I2C0->S |= I2C_S_IICIF_MASK;
}

void I2C0_Write(uint8_t data){
    I2C0->D = data;
    I2C0_Wait();
}

static uint8_t rtc_ler_ram_byte(uint8_t endereco_ram) {
    uint8_t dado;
    I2C0_Start();
    I2C0_Write(RTC_ADDR << 1);
    I2C0_Write(endereco_ram);
    I2C0->C1 |= I2C_C1_RSTA_MASK; // Repeated start
    I2C0_Write((RTC_ADDR << 1) | 0x01); // Modo Leitura
    I2C0->C1 &= ~I2C_C1_TX_MASK;      // Mestre em modo RX
    I2C0->C1 |= I2C_C1_TXAK_MASK;     // Prepara NACK (vamos ler só 1 byte)
    
    (void)I2C0->D; // Leitura falsa para iniciar
    I2C0_Wait();   // Espera byte chegar
    dado = I2C0->D; // Lê o byte
    I2C0_Stop();
    return dado;
}

// Função para escrever um byte na RAM (ou registrador) do RTC
// Esta função pode ser 'static' se só for usada dentro de rtc.c
static void rtc_escrever_ram_byte(uint8_t endereco_ram, uint8_t valor) {
    I2C0_Start();
    I2C0_Write(RTC_ADDR << 1);
    I2C0_Write(endereco_ram);
    I2C0_Write(valor);
    I2C0_Stop();
}

// Função para armazenar no RTC
void configurar_RTC(data_t *calibracao){
    uint8_t dados[7];

    dados[0] = dec_to_bcd(calibracao->segundo) & 0x7F;
    dados[1] = dec_to_bcd(calibracao->minuto);
    dados[2] = dec_to_bcd(calibracao->hora) & 0xBF;
    dados[3] = 0;  // Dia da semana (não usamos)
    dados[4] = dec_to_bcd(calibracao->dia);
    dados[5] = dec_to_bcd(calibracao->mes);
    dados[6] = dec_to_bcd(calibracao->ano - 2000);

    // Seleciona registrador de início (0x00)
    I2C0_Start();
    I2C0_Write(RTC_ADDR << 1);  // Write
    I2C0_Write(0x00);

    // Escreve os 7 bytes
    for (int i = 0; i < 7; i++) {
        I2C0_Write(dados[i]);
    }
    I2C0_Stop();
}

void RTC_Verificar_E_Configurar_Se_Necessario(data_t *valores_calibracao_padrao){
    uint8_t flag_config = rtc_ler_ram_byte(ENDERECO_FLAG_RTC_RAM);

    if (flag_config != VALOR_FLAG_RTC_CONFIGURADO){
        printf("RTC: Flag de configuração não encontrada ou inválida. Configurando com valores padrão...\n");
        
        configurar_RTC(valores_calibracao_padrao); // Usa os valores de calibração
        rtc_escrever_ram_byte(ENDERECO_FLAG_RTC_RAM, VALOR_FLAG_RTC_CONFIGURADO);
        
        printf("RTC: Configurado e flag salva na RAM do RTC.\n");
    }
    else printf("RTC: Já configurado anteriormente (flag encontrada na RAM do RTC).\n");
}

// Função para ler o RTC
void ler_RTC(data_t *agora){
    uint8_t dados[7];

    I2C0_Start();
    I2C0_Write(RTC_ADDR << 1); // Write
    I2C0_Write(0x00);
    I2C0->C1 |= I2C_C1_RSTA_MASK;
    I2C0_Write((RTC_ADDR << 1) | 0x01); // Read
    I2C0->C1 &= ~I2C_C1_TX_MASK;

    for(int i=0;i<6;i++){
        I2C0->C1 &= ~I2C_C1_TXAK_MASK;
        if(i==0){
            (void)I2C0->D;
        }
        I2C0_Wait();
        dados[i] = I2C0->D;
    }

    I2C0->C1 |= I2C_C1_TXAK_MASK;
    I2C0_Wait();
    dados[6] = I2C0->D;
    I2C0_Stop();

    // Converte e armazena no "agora"
    agora->segundo = bcd_to_dec(dados[0] & 0x7F);
    agora->minuto = bcd_to_dec(dados[1]);
    agora->hora = bcd_to_dec(dados[2] & 0x3F);
    agora->dia = bcd_to_dec(dados[4]);
    agora->mes = bcd_to_dec(dados[5] & 0x7F);
    agora->ano = bcd_to_dec(dados[6]) + 2000;   
}

// Função de delay
void delay_ms(volatile uint32_t ms){
    while (ms--) {
        for (volatile uint32_t i = 0; i < 7000; i++);
    }
}