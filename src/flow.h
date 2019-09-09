#pragma once

#include "pin.h"
#include "flash.h"
#include "counter.h"
#include "modbus_slave.h"


struct In_regs {
   
   UART::Settings uart_set;         // 0
   uint16_t modbus_address;         // 1
   uint16_t password;               // 2
   uint16_t factory_number;         // 3
   uint16_t value;                  // 4

}__attribute__((packed));

struct Out_regs {

   uint16_t device_code;            // 0
   uint16_t factory_number;         // 1
   UART::Settings uart_set;         // 2
   uint16_t modbus_address;         // 3
   uint16_t count;                  // 4

}__attribute__((packed));

#define ADR(reg) GET_ADR(In_regs, reg)

template<class Flash, class Modbus>
class Flow
{
   Counter& counter;
   Flash& flash;
   Modbus& modbus;
   Timer timer;
   bool save {false};
   // uint16_t count;
   // uint16_t reset_time = 30_s;

public:
   Flow(Counter& counter, Flash& flash, Modbus& modbus) 
      : counter {counter}
      , flash {flash}
      , modbus {modbus}
   {}

   void operator()() {

      modbus.outRegs.count = counter;

      modbus([&](uint16_t registrAddress) {
            static bool unblock = false;
         switch (registrAddress) {
            case ADR(uart_set):
               flash.uart_set
                  = modbus.outRegs.uart_set
                  = modbus.inRegs.uart_set;
            break;
            case ADR(modbus_address):
               flash.modbus_address 
                  = modbus.outRegs.modbus_address
                  = modbus.inRegs.modbus_address;
            break;
            case ADR(password):
               unblock = modbus.inRegs.password == 208;
            break;
            case ADR(factory_number):
               if (unblock) {
                  unblock = false;
                  flash.factory_number 
                     = modbus.outRegs.factory_number
                     = modbus.inRegs.factory_number;
               }
               unblock = true;
            break;
            case ADR(value):
               counter.set_value(modbus.inRegs.value);
            break;
         } // switch
      });

      // if (timer.done())
      //    counter.reset();  

   }//operator
};