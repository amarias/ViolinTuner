// Violin Tuner Project for the Tiva C Series TM4C123G LaunchPad by Allison M.
// Press the switch to turn on the tuner.
// Continue to press the switch to cycle through the pitches G3, D4, A4, and E5.
// After E5, pressing the switch turns off the sound.

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

// SysTick Reload values for the pitches
uint32_t sounds[] = {
                     204081,    // G3 (Sound off)
                     204081,    // G3
                     136193,    // D4
                     90909,     // A4
                     60670};    // E5

uint8_t button_press = 0;       // Index for the sounds array
                                // Correlates with number of presses
uint8_t button_released = 0;    // Equals 1 when button is released after previously pressed
                                // Equals 0 when button has been pressed but not released yet
uint32_t toggle = 0;        // 0 equals sound off; 1 equals sound on

/**
 * Called at either 392Hz, 587.4Hz, 880Hz, or 1318.6Hz.
 * Toggles the output to create sound.
 */
void SysTickHandler(void){

    uint8_t val = 0;

    if(toggle){
        val = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2);
        val ^= 0x04;
    }

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, val);
}

/**
 * main.c
 */
int main(void)
{

    //Disable interrupts while initializing
    IntMasterDisable();

    // Set the clock to run with a 16MHz main oscillator at 80MHz from the PLL
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Initialize PA3 as input and PA2 as output
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}

    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);

    // Initialize SysTick
    SysTickDisable();
    SysTickPeriodSet(sounds[button_press]);    // Starts at 392Hz
    SysTickIntRegister(SysTickHandler);
    IntPrioritySet(FAULT_SYSTICK, 2);
    SysTickIntEnable();
    SysTickEnable();

    // Enabled Interrupts
    IntMasterEnable();
    while(1){
        // Check if button has been pressed and set the correct pitch
        if(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3)){

            SysCtlDelay(133333);    // Delay of 5ms for debounce

            if(button_press && button_released){
                if(button_press == 4){
                    SysTickPeriodSet(sounds[0]);
                    toggle = 0;
                }
                else
                    SysTickPeriodSet(sounds[++button_press]);
            }

            button_released = 0;
            if(!toggle && !button_press){
                toggle = 1;
                button_press = 1;
            }
        }
        else{
            if(toggle)
                button_released = 1;
            else
                button_press = 0;
        }
    }
}
