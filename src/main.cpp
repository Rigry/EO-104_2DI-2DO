#define STM32F030x6
#define F_CPU   48000000UL

#include "periph_rcc.h"
#include "init_clock.h"
#include "flow.h"

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
   struct Flash_data {
      uint16_t factory_number = 0;
      UART::Settings uart_set = {
       .parity_enable  = false,
       .parity         = USART::Parity::even,
       .data_bits      = USART::DataBits::_8,
       .stop_bits      = USART::StopBits::_1,
       .baudrate       = USART::Baudrate::BR9600,
         .res            = 0
      };
      uint8_t  modbus_address = 1;
      uint16_t model_number   = 0;
   }flash;

   // decltype(auto) led = Pin::make<DO1, mcu::PinMode::Output>();
   
   decltype(auto) counter = Counter::make<mcu::Periph::TIM1, DI1>(45);

   [[maybe_unused]] auto _ = Flash_updater<
        mcu::FLASH::Sector::_10
      , mcu::FLASH::Sector::_9
   >::make (&flash);

   // Flash<Flash_data, mcu::FLASH::Sector::_8> flash{};

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

   Flow<Flash, Modbus> flow {counter, flash, modbus}; 

   while(1){
      flow();
      // led = modbus.outRegs.count > 5000 ? true : false;
      __WFI();
   }

}

