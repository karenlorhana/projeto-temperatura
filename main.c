/**
	Projeto Microcontroladores e Microprocessadores - junho de 2023
	Medição de temperatura e exibição em um display LCD
*/


#include "stm32f4xx.h"	//bibliotecas padrão STM32F4
#include "Utility.h"	//biblioteca de funções utilitárias
#include <stdio.h>		//para uso da função printf


//função principal
int main(void)
{
    Utility_Init();		//configura o sistema de clock e timer2
    USART1_Init();

    Delay_ms(2000);		//tempo de acomodação da tensão de alimentação no sensor DHT11 (mínimo 1s)
	LCD_Init(); //inicia o LCD

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Habilita o clock do GPIOC
    GPIOC->MODER |= 0b01 << 1;					//pino PC0 como saída
    GPIOC->MODER |= 0b01 << 2;					//pino PC1 como saída
    GPIOC->MODER |= 0b01 << 4;					//pino PC2 como saída
    GPIOC->MODER |= 0b01 << 6;					//pino PC3 como saída
    GPIOC->MODER |= 0b01 << 8;					//pino PC4 como saída
    GPIOC->MODER |= 0b01 << 10;					//pino PC5 como saída


    //Configuração do pino de dados
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	//habilita o clock do GPIOA
    GPIOA->OTYPER |= 1;						//pino PA0 como open-drain
    GPIOA->PUPDR |= 0b01 ;					//resistor de pull-up em PA0
    GPIOA->ODR |= 1;						//pino PA0 inicialmente em nível alto
    GPIOA->MODER |= 0b01;					//pino PA0 como saída


    //Comunicação com o sensor
    while(1)
    {
    	LCD_Init(); //inicia o LCD

    	LCD_Write_Char("oi");
    	LCD_Clear();


        Delay_ms(1000);			//delay entre leituras do sensor

        GPIOA->ODR &= ~1;		//nível baixo em PA0
        Delay_ms(20);			//aguarda 20ms
        GPIOA->ODR |= 1;		//libera a linha de dados para aguardar a resposta do sensor
        Delay_us(5);			//aguarda 5us para iniciar o processo de leitura da linha de dados

        //leitura do ACK do sensor
        while(GPIOA->IDR & 1);		//aguarda o início da resposta do sensor
        while(!(GPIOA->IDR & 1));	//aguarda o primeiro período de 80us
        while(GPIOA->IDR & 1);		//aguarda o segundo período de 80us

        //leitura dos bits de dados
        uint32_t data = 0;
        for(uint8_t contador=0; contador < 32; ++contador)
        {
            while(!(GPIOA->IDR & 1));		//aguarda o período em nível baixo
            TIM2->CNT = 0;					//inicia o temporizador
            while(GPIOA->IDR & 1);			//aguarda o período em nível alto, contando o tempo
            data <<= 1;						//desloca o dado para inserir o novo bit
            if(TIM2->CNT > 40) data |= 1;	//insere o bit 1 se o tempo em alto for maior que 40us
        }

        //leitura dos bits de checksum
        uint8_t checksum = 0;
        for(uint8_t contador=0; contador < 8; ++contador)
        {
            while(!(GPIOA->IDR & 1));			//aguarda o período em nível baixo
            TIM2->CNT = 0;						//inicia o temporizador
            while(GPIOA->IDR & 1);				//aguarda o período em nível alto, contando o tempo
            checksum <<= 1;						//desloca o dado para inserir o novo bit
            if(TIM2->CNT > 40) checksum |= 1;	//insere o bit 1 se o tempo em alto for maior que 40us
        }

        //Verificação de checksum e impressão do valor da temperatura
        uint8_t soma = ((data & 0xFF000000) >> 24) + ((data & 0x00FF0000) >> 16) + ((data & 0x0000FF00) >> 8) + (data & 0x000000FF);

        // Declaração da variável Temperatura
        int Temperatura = 0;

        if (soma == checksum) {
            Temperatura = ((data & 0x0000FF00) >> 8) + (int)(data & 0x000000FF)/10;
            printf ("Temperatura = %d C \n", Temperatura);

        }}}


