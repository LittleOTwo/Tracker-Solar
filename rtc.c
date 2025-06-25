#include "rtc_e_sol.h"
#include <drivers/i2c.h> // API I2C oficial do Zephyr
#include <stdio.h>       // Para a função de impressão do Zephyr, printk

#define RTC_I2C_ADDR 0x68
#define ENDERECO_FLAG_RTC_RAM 0x08
#define VALOR_FLAG_RTC_CONFIGURADO 0xA5

// A definição real da nossa variável global. Ela "vive" aqui.
data_t agora;

// Funções de conversão BCD (não mudam, são C puro e internas a este arquivo)
static uint8_t dec_to_bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
static uint8_t bcd_to_dec(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }

// Função para escrever um byte na RAM do RTC usando a API do Zephyr
static int rtc_escrever_ram_byte(const struct device *i2c_dev, uint8_t endereco_ram, uint8_t valor) {
    uint8_t buffer[] = {endereco_ram, valor};
    return i2c_write(i2c_dev, buffer, sizeof(buffer), RTC_I2C_ADDR);
}

// Função para ler um byte da RAM do RTC usando a API do Zephyr
static int rtc_ler_ram_byte(const struct device *i2c_dev, uint8_t endereco_ram, uint8_t *valor_lido) {
    return i2c_write_read(i2c_dev, RTC_I2C_ADDR, &endereco_ram, 1, valor_lido, 1);
}

// Função interna para configurar a hora no RTC
static void configurar_rtc_interno(const struct device *i2c_dev, const data_t *valores_config) {
    uint8_t buffer_escrita[8];
    buffer_escrita[0] = 0x00; // Endereço do registrador de segundos
    buffer_escrita[1] = dec_to_bcd(valores_config->segundo) & 0x7F;
    buffer_escrita[2] = dec_to_bcd(valores_config->minuto);
    buffer_escrita[3] = dec_to_bcd(valores_config->hora) & 0xBF;
    buffer_escrita[4] = dec_to_bcd(1);
    buffer_escrita[5] = dec_to_bcd(valores_config->dia);
    buffer_escrita[6] = dec_to_bcd(valores_config->mes);
    buffer_escrita[7] = dec_to_bcd(valores_config->ano - 2000);

    int ret = i2c_write(i2c_dev, buffer_escrita, sizeof(buffer_escrita), RTC_I2C_ADDR);
    if (ret != 0) {
        printk("ERRO: Falha ao escrever para configurar RTC! (erro: %d)\n", ret);
    }
}

// Implementação da nossa nova função pública de setup
void RTC_Setup(const struct device *i2c_dev, const data_t *calibracao_padrao) {
    if (!device_is_ready(i2c_dev)) {
        printk("ERRO: Dispositivo I2C nao esta pronto!\n");
        return;
    }

    uint8_t flag_lida;
    int ret = rtc_ler_ram_byte(i2c_dev, ENDERECO_FLAG_RTC_RAM, &flag_lida);

    if (ret != 0 || flag_lida != VALOR_FLAG_RTC_CONFIGURADO) {
        printk("RTC nao configurado. Configurando com valores padrao...\n");
        configurar_rtc_interno(i2c_dev, calibracao_padrao);
        rtc_escrever_ram_byte(i2c_dev, ENDERECO_FLAG_RTC_RAM, VALOR_FLAG_RTC_CONFIGURADO);
        printk("RTC configurado e flag salva.\n");
    } else {
        printk("RTC ja configurado anteriormente.\n");
    }
}

// Implementação da nossa nova função de leitura
void RTC_Ler(const struct device *i2c_dev) {
    uint8_t reg_addr_inicio = 0x00;
    uint8_t dados_lidos[7];

    if (!device_is_ready(i2c_dev)) { return; }

    int ret = i2c_write_read(i2c_dev, RTC_I2C_ADDR, &reg_addr_inicio, 1, dados_lidos, 7);
    if (ret != 0) {
        printk("ERRO ao ler do RTC! (erro: %d)\n", ret);
        return;
    }

    // Atualiza diretamente a variável global 'agora'
    agora.segundo = bcd_to_dec(dados_lidos[0] & 0x7F);
    agora.minuto  = bcd_to_dec(dados_lidos[1]);
    agora.hora    = bcd_to_dec(dados_lidos[2] & 0x3F);
    agora.dia     = bcd_to_dec(dados_lidos[4]);
    agora.mes     = bcd_to_dec(dados_lidos[5]);
    agora.ano     = bcd_to_dec(dados_lidos[6]) + 2000;
}
