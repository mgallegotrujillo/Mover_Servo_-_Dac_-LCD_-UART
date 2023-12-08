/* Conexiones: 
	Puerto A:			Puerto F:										Teclado: Mirar Simulacion Proteus
		Pin 0: D0			Pin 3: Conversor ADC3 			 
		Pin 1: D1																		
		Pin 2: D2																	
		Pin 3: D3																	
		Pin 4: D4 
 		Pin 5: D5
		Pin 6: D6
		Pin 7: D7
		Pin 8: RS
		Pin 9: Enable 
*/

// ************************ LIBRERIAS *******************
		#include <stdio.h>
		#include "STM32F7xx.h"
		#include "math.h"
// ******************************************************

// ************************ VARIABLES GLOBALES **********
		short valor_adc_teclado=0;
		short valor_adc_sensor_1=0;
		short valor_adc_sensor_2=0;
		char caracter_frase='1'; // Variable donde le guardare los caracteres que vaya recorriendo en una frase 
		short voltaje=0; 
		short digito1=0; 
		short digito2=0; 
		short digito3=0; 
		short digito4=0;
		bool mostrar=0;
		short dato_recibido=0;
		short valor_ingresado[4]={0,0,0,0};
		short iterador=0;
		bool bandera_aceptar=0;
		short divisor=0;
		short tiempo_servo=0;
		// DAC
		short amplitud=2047;
		short tiempo=0;
		short funcion=0;
		short frecuencia=1;
// ******************************************************

// ************************ COMANDOS LCD ****************
		char limpiar_pantalla = 0x01; // Con este comando limpio todo lo que se muestre en la LCD
		char home = 0x02; // Este es el comando con el que el cursor de la LCD vuelve a su posicion inicial
		char set = 0x3C; // Con este comando le digo a la LCD que la uso a 8 bits 
	
		char set_modo_1 = 0x06; // Este es el comando del modo de cursor 1 donde el cursor incrementa al escribir y la pantalla es estatica
		char set_modo_2 = 0x04; // Este es el comando del modo de cursor 2 donde el cursor incrementa 
													// Al escribir y la pantalla se desplaza cada vez que se escribe un dato
	
		char prender_display_cursor_parpadea = 0x0F; // Con este comando prendo la LCD, el cursor y el activo el parpadeo del cursor
		char prender_display = 0x0E; // Con este comando prendo la LCD, el cursor pero apago el parpadeo del cursor 
		char apagar_LCD = 0x08; // Con este comando apago todo
	
		char desplazar_display_derecha = 0x1C; // Con este comando desplazo a la derecha el display 
		char desplazar_display_izquierda = 0x18; // Con este comando desplazo a la izquierda el display
		char desplazar_cursor_derecha =0x14; // Con este comando desplazo el cursor a la derecha
		char desplazar_cursor_izquierda =0x10; // Con este comando desplazo el cursor a la izquierda
	
		char posicion_fila_1 = 0x80; // Con este comando le digo que se ubique a una posicion X que quiero DE LA PRIMERA FILA
		char posicion_fila_2 = 0xC0; // Con este comando le digo que se ubique a una posicion X que quiero DE LA SEGUNDA FILA
// ******************************************************

// ************************ FUNCIONES *******************
		void mover_servo(){
			tiempo_servo=(valor_adc_sensor_1*2500)/4095;
			GPIOA->ODR |=0x02; 
			for(int i=10; i<tiempo_servo; i++){}
			GPIOA->ODR &=0x01;
		}
		void enviar_caracter(char caracter){ // Inicio de enviar uncaracter 
					UART4->TDR =caracter;
					while((UART4->ISR &=0x80)==0){}  
		} // Fin funcion de enviar un caracter 
		void convercion_analogica(){ // Inicio de la funcion convertir ADC 
				ADC3->CR2 |=0x40000000; // Activo SWSTART que es el pin para empezar a hacer la conversion ADC
				while((ADC3->SR & 0x02)==1){} // Si la bandera que se ubica en el pin 1 esta en '1' la conversion acabo 
		} // Fin de la funcion de covertir ADC
		
		void mandar_comandos(char comando){ // Inicio de funcion de mandar comandos 
			GPIOC->ODR = comando; // Envio el comando 
			GPIOC->ODR &=0xEFF; // Con esto coloco un '0' en el 8 que es RS
			GPIOC->ODR |=0x200; // Con esto coloco un '1' en el 9 que es ENABLE
			for(int i=0; i<10000; i++){} // Con esto hago un delay para que el comando se ejecute perfectamente
			GPIOC->ODR &=0xDFF; // Con esto hago ENABLE a 	'0' 
		} // Fin funcion de mandar comandos 
		
		void mandar_caracteres(char caracter){ // Inicio de funcion mandar caracteres 
			GPIOC->ODR = caracter; 
			GPIOC->ODR |=0x100; // Con esto pongo RS = '1'
			GPIOC->ODR |=0x200; // ENABLE = 1
			for(int i=0; i<100000; i++){} // Delay para que el dato sea mostrado por la LCD
			GPIOC->ODR &=0xDFF; // ENABLE = 0
		} // Fin funcion de enviar caracteres

		void mandar_frase(char frase[]){
			for(int i=0;caracter_frase!=':';i++){ // For que recorre la frase caracter por caracter hasta encontrar un '-'
				caracter_frase=frase[i]; // Le asignla el caracter de la frase que ha recorrido a la variable externa 
				GPIOC->ODR = caracter_frase;	// Con esto hago que se envie el hexa correspondiente al caracter reccorrido 
				GPIOC->ODR |=0x100; // Con esto pongo RS = '1'
				GPIOC->ODR |=0x200; // ENABLE = 1
				for(int i=0; i<100000; i++){} // Delay para que el dato sea mostrado por la LCD
				GPIOC->ODR &=0xDFF; // ENABLE = 0
		} // Fin for de enviar frase
		caracter_frase='1'; // Vuelvo la varaible externa a su valor de inicio 
		} 
		void dividir_datos(int resultado){ // Inicio de funcion de dividir datos 
				digito1 = resultado/1000;
				digito2 = (resultado/100)%10;
				digito3 = (resultado%100)/10;
				digito4 = (resultado%10);
		} // Fin funcion de dividir numeros 
		void decodificacion_analogica(){
			if(valor_adc_teclado>3700 && valor_adc_teclado < 3900){
				mandar_caracteres('1');
				valor_ingresado[iterador]=1;
				iterador++;
			}
			else if(valor_adc_teclado>3500 && valor_adc_teclado <3650){
				mandar_caracteres('2');
				valor_ingresado[iterador]=2;
				iterador++;
			}
			else if(valor_adc_teclado>3300 && valor_adc_teclado <3400){
				mandar_caracteres('3');
				valor_ingresado[iterador]=3;
				iterador++;
			}
			else if(valor_adc_teclado>3030 && valor_adc_teclado <3050){
				mandar_caracteres('A');
				valor_ingresado[iterador]=10;
				iterador++;
			}
			else if(valor_adc_teclado>2900 && valor_adc_teclado <3000){
				mandar_caracteres('4');
				valor_ingresado[iterador]=4;
				iterador++;
			} 
			else if(valor_adc_teclado>2800 && valor_adc_teclado <2900){
				mandar_caracteres('5');
				valor_ingresado[iterador]=5;
				iterador++;
			}
			else if(valor_adc_teclado>2680 && valor_adc_teclado <2705){
				mandar_caracteres('6');
				valor_ingresado[iterador]=6;
				iterador++;
			}
			else if(valor_adc_teclado>2490 && valor_adc_teclado <2502){
				mandar_caracteres('B');
				valor_ingresado[iterador]=11;
				iterador++;
			} //
			else if(valor_adc_teclado>2340 && valor_adc_teclado <2400){
				mandar_caracteres('7');
				valor_ingresado[iterador]=7;
				iterador++;
			}
			else if(valor_adc_teclado>2290 && valor_adc_teclado <2310){
				mandar_caracteres('8');
				valor_ingresado[iterador]=8;
				iterador++;
			}
			else if(valor_adc_teclado>2190 && valor_adc_teclado <2210){
				mandar_caracteres('9');
				valor_ingresado[iterador]=9;
				iterador++;
			}
			else if(valor_adc_teclado>2060 && valor_adc_teclado <2080){
				mandar_caracteres('C');
				valor_ingresado[iterador]=12;
				iterador++;
			} 
			else if(valor_adc_teclado>2000 && valor_adc_teclado <2030){
				mandar_caracteres('*');
				valor_ingresado[iterador]=14;
				iterador++;
			}
			else if(valor_adc_teclado>1940 && valor_adc_teclado <1970){
				mandar_caracteres('0');
				valor_ingresado[iterador]=0;
				iterador++;
			}
			else if(valor_adc_teclado>1887 && valor_adc_teclado <1895){
				mandar_caracteres('#');
				valor_ingresado[iterador]=15;
				iterador++;
			}
			else if(valor_adc_teclado>1780 && valor_adc_teclado <1795){
				mandar_caracteres('D');
				bandera_aceptar=1;
			}
			else if(valor_adc_teclado<50){
				mandar_caracteres(' ');
				
			}
		}
// ******************************************************
		
// ************************ INTERRUPCIONES **************
		extern "C"{
			void ADC_IRQHandler(void){
				if(ADC3->SQR3 ==0x09){
					valor_adc_teclado=ADC3->DR; // Le asigno el valor leido a una variable
				}else if(ADC3->SQR3 ==0x0E){
					valor_adc_sensor_1=ADC3->DR;
				}else if(ADC3->SQR3 ==0x0F){
					valor_adc_sensor_2=ADC3->DR;
				}
			}
			void SysTick_Handler(void){
				divisor++;
				if(divisor>1){
					divisor=0;
					mostrar=1;
				}
			}
			void UART4_IRQHandler(void){
				if(UART4->ISR & 0x20){ // Si el dato esta completamente recibido 
					dato_recibido = UART4->RDR; // Guardo el dato 
				}
			}
		}
// ******************************************************

// ************************ MAIN ************************
int main(void){
	// ********************** PUERTOS *********************
			RCC->AHB1ENR |=0x27;// Habilito el puerto A, B y F 
	// ****************************************************
	
	// ********************** PINES ***********************
			GPIOA->MODER |=0x306; //Alternativo pin 0, OUTPUT pin 1 Analogico pin 4 
			GPIOA->OTYPER |=0x00; // PUSH-PULL
			GPIOA->OSPEEDR |=0x04; // MEDIUM-SPEED pin 1
			GPIOA->PUPDR |=0x04; // PULL-UP pin 1
			GPIOA->AFR[0]=0x08; // Funcion alternativa Rx

			GPIOC->MODER |=0x855555; // OUTPUT pines 0 a 9, ALTERNATIVO pin 11
			GPIOC->OTYPER |=0x00; // PUSH-PULL
			GPIOC->OSPEEDR |=0x55555; // MEDIUM-SPEED pines 0 a 9
			GPIOC->PUPDR |=0x55555; // PULL-UP pines 0 a 9
			GPIOC->AFR[1]=0x8000; // Funcion alternativa Tx
	
			GPIOF->MODER |=0xFC0; // ANALOGICO pin 3,4 y 5 
	// ****************************************************
	
	// ********************** ADC *************************
			RCC->APB2ENR |=0x400; // Activo el reloj del AC3
			ADC3->CR1 |=0x20; // Activo la interrupcion EOCIE
			ADC3->CR2 |=0x400; // Activo EOCIE de seguido no por paquetes
			ADC3->CR2 |=0x01; // Habilito la conversion ADC3
			NVIC_EnableIRQ(ADC_IRQn); // Activo el vector de interrupciones para el ADC
	// ****************************************************
	
	// ************************ SYSTICK **********************
			SystemCoreClockUpdate();
			SysTick_Config(SystemCoreClock/1000); // 1ms
	// *******************************************************
	
	// ************************ DAC **************************
			RCC->APB1ENR |=0x20000000; // Activo el reloj del DAC
			DAC->CR |=0x01; // Activo el Dac 1 (PA4)
	// *******************************************************
	
	// ************************ UART *************************
			RCC->APB1ENR |=0x80000; // Activo el reloj del UART 4 
			UART4->BRR =0x683; // 9600 Baudios
			UART4->CR1 |=0x2C; // Activo Rx, Tx y la interrupcion por Rx
			UART4->CR1 |=0x01; // Habilito el modulo UART
			NVIC_EnableIRQ(UART4_IRQn); 
	// *******************************************************
	
	// ********************** LCD *************************
			mandar_comandos(limpiar_pantalla);
			mandar_comandos(home);
			mandar_comandos(set);
			mandar_comandos(prender_display);
			mandar_comandos(set_modo_1);
			mandar_comandos(posicion_fila_1); 
			
			mandar_frase("Teclado:");
			mandar_comandos(posicion_fila_2);
			mandar_frase("Analogico:");
			for(int i=0; i<1000000; i++){}
			mandar_comandos(limpiar_pantalla);
			mandar_comandos(home);
	// ****************************************************
				mandar_frase("Letra:");
	// ********************** BUCLE ***********************
				enviar_caracter('H');
			while(true){
				mover_servo();
				// DAC
				tiempo++;
				if(tiempo>100){
					tiempo=0;
				}
					funcion=(sin(2*3.1416*frecuencia*tiempo/100)*amplitud)+2047;
					DAC->DHR12R1=funcion;
			
				// Visualizacion Señal DAC
					dividir_datos(funcion);
					enviar_caracter('0');
					enviar_caracter(',');
					enviar_caracter(digito1+0x30);
					enviar_caracter(digito2+0x30);
					enviar_caracter(digito3+0x30);
					enviar_caracter(digito4+0x30);
					enviar_caracter(',');
					enviar_caracter('4');
					enviar_caracter('0');
					enviar_caracter('0');
					enviar_caracter('0');
					enviar_caracter('\n');
				
				// PROCESAMIENTO ADC Y DEMAS 
				if(mostrar==1){
					
					// Lectura Teclado Matricial
					ADC3->SQR3=0x09;
					mandar_comandos(0x80 + 0x06); // Lo posiciono en una parte de la fila 1
					convercion_analogica(); // Realizo la conversion ADC
					decodificacion_analogica();

					// Mostrar la cantidad de teclas presionadas
					mandar_comandos(posicion_fila_2);
					dividir_datos(iterador);
					mandar_caracteres(digito3+0x30);
					mandar_caracteres(digito4+0x30);
					
					// LECTURA SENSOR 1
					ADC3->SQR3 =0x0E;
					convercion_analogica();
					// Visualizacion Sensor 1
					mandar_comandos(posicion_fila_1 + 0x0B);
					dividir_datos(valor_adc_sensor_1);
					mandar_caracteres(digito1+0x30);
					mandar_caracteres(digito2+0x30);
					mandar_caracteres(digito3+0x30);
					mandar_caracteres(digito4+0x30);
					
					// LECTURA SENSOR 2
					ADC3->SQR3 =0x0F;
					convercion_analogica();
					// Visualizacion Sensor 2
					mandar_comandos(posicion_fila_2 + 0x0B);
					dividir_datos(valor_adc_sensor_2);
					mandar_caracteres(digito1+0x30);
					mandar_caracteres(digito2+0x30);
					mandar_caracteres(digito3+0x30);
					mandar_caracteres(digito4+0x30);
					
					// Si le doy aceptar
					if(bandera_aceptar==1){
						iterador=0;
						// Mostrar los numeros ingresados
						mandar_comandos(posicion_fila_1 + 0x07);
						frecuencia=valor_ingresado[0]*1000+valor_ingresado[1]*100+valor_ingresado[2]*10+valor_ingresado[3];
						mandar_caracteres(valor_ingresado[0]+0x30);
						mandar_caracteres(valor_ingresado[1]+0x30);
						mandar_caracteres(valor_ingresado[2]+0x30);
						bandera_aceptar=0;
					}
					mostrar=0;
				}
			}
	// ****************************************************
}
// ******************************************************