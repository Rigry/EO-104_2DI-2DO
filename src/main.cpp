#define STM32F030x6
#define F_CPU   48000000UL

#include "periph_rcc.h"
#include "flash.h"
#include "pin.h"
#include "modbus_slave.h"
#include "literals.h"
#include "init_clock.h"
#include "counter.h"

/// эта функция вызываеться первой в startup файле
extern "C" void init_clock () { init_clock<8_MHz,F_CPU>(); }

using DI1 = mcu::PA9;
using DI2 = mcu::PA8;

using DO1 = mcu::PB0; 
using DO2 = mcu::PB1; 

using TX  = mcu::PA2;
using RX  = mcu::PA3;
using RTS = mcu::PA4;

int main()
{
   decltype(auto) pin = Pin::make<DI1, mcu::PinMode::Input>();

   Flash<Flash_data, mcu::FLASH::Sector::_8> flash{};

   decltype(auto) modbus = Modbus_slave<In_regs, Out_regs>
                 ::make<mcu::Periph::USART1, TX, RX, RTS>
                       (flash.modbus_address, flash.uart_set);

   modbus.outRegs.device_code       = 12;
   modbus.outRegs.factory_number    = flash.factory_number;
   modbus.outRegs.modbus_address    = flash.modbus_address;
   modbus.outRegs.uart_set          = flash.uart_set;
   modbus.arInRegsMax[ADR(uart_set)]= 0b11111111;
   modbus.inRegsMin.modbus_address  = 1;
   modbus.inRegsMax.modbus_address  = 255;

   using Flash  = decltype(flash);
   using Modbus = Modbus_slave<In_regs, Out_regs>;

   Counter<Flash, Modbus> counter {pin, flash, modbus}; 

   while(1){
      counter();
      __WFI();
   }

}

