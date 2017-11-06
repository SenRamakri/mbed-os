/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "drivers/QSPI.h"
#include "platform/mbed_critical.h"

#if DEVICE_QSPI
#define IS_BUS_WIDTH_VALID(width)          ((width == 1) || (width == 2) || (width == 4))    
#define IS_SIZE_VALID(size)                ((size == 8) || (size == 16) || (size == 24) || (size == 32))      
#define IS_ALT_SIZE_VALID(alt_size)        ((alt_size == 0) || (alt_size == 8) || (alt_size == 16) || (alt_size == 24) || (alt_size == 32))      

namespace mbed {

QSPI* QSPI::_owner = NULL;
SingletonPtr<PlatformMutex> QSPI::_mutex;  
  
QSPI::QSPI(PinName io0, PinName io1, PinName io2, PinName io3, PinName sclk, PinName ssel) :
        _qspi(),
        _inst_width(QSPI_DEFAULT_INST_WIDTH),
        _inst_size(QSPI_DEFAULT_INST_SIZE),
        _address_width(QSPI_DEFAULT_ADDRESS_WIDTH),
        _address_size(QSPI_DEFAULT_ADDRESS_SIZE),
        _alt_width(QSPI_DEFAULT_ALT_WIDTH),
        _alt_size(QSPI_DEFAULT_ALT_SIZE),
        _data_width(QSPI_DEFAULT_DATA_WIDTH),
        _num_dummy_cycles(QSPI_DEFAULT_DUMMY_CYCLES),
        _hz(_1_MHZ_) {
    // No lock needed in the constructor
    qspi_init(&_qspi, io0, io1, io2, io3, sclk, ssel, _hz, QSPI_CFG_BUS_QUAD);
    _acquire();
}
        
bool QSPI::configure_format(int inst_width, int inst_size, 
                      int address_width, int address_size,  
                      int alt_width, int alt_size,
                      int data_width,
                      int dummy_cycles) {
    if(!IS_BUS_WIDTH_VALID(inst_width)) return false;
    if(!IS_SIZE_VALID(inst_size)) return false;
    if(!IS_BUS_WIDTH_VALID(address_width)) return false;
    if(!IS_SIZE_VALID(address_size)) return false;
    if(!IS_BUS_WIDTH_VALID(alt_width)) return false;
    if(!IS_ALT_SIZE_VALID(alt_size)) return false;
    if(!IS_BUS_WIDTH_VALID(data_width)) return false;
    if(dummy_cycles < 0) return false;                         
    
    lock();
    _inst_width = (qspi_bus_width_t)inst_width;
    _inst_size = (qspi_address_size_t)inst_size;
    _address_width = (qspi_bus_width_t)address_width;
    _address_size = (qspi_address_size_t)address_size;
    _alt_width = (qspi_bus_width_t)alt_width;
    _alt_size = (qspi_alt_size_t)alt_size;
    _data_width = (qspi_bus_width_t)data_width;
    _num_dummy_cycles = dummy_cycles;
    unlock();
    
    return true;
}

void QSPI::set_frequency(int hz) {
    lock();
    _hz = hz;
    // If changing format while you are the owner than just
    // update frequency, but if owner is changed than even frequency should be
    // updated which is done by acquire.
    if (_owner == this) {
        qspi_frequency(&_qspi, _hz);
    } else {
        _acquire();
    }
    unlock();
}

void QSPI::acquire() {
    lock();
    if (_owner != this) {
        qspi_frequency(&_qspi, _hz);
        _owner = this;
    }
    unlock();
}

// Note: Private function with no locking
void QSPI::_acquire() {
     if (_owner != this) {
        qspi_frequency(&_qspi, _hz);
        _owner = this;
    }
}

void QSPI::set_read_instruction(int read_instruction) {
  lock();
  _read_instruction = read_instruction;
  unlock();
}

void QSPI::set_write_instruction(int write_instruction) {
  lock();
  _write_instruction = write_instruction;
  unlock();
}

void QSPI::set_alt_value(int alt_value) {
  lock();
  _alt_value = alt_value;
  unlock();
}

int QSPI::read(unsigned int address, char *rx_buffer, size_t *rx_length) {
    return read(_read_instruction, address, _alt_value, rx_buffer, rx_length);
}

int QSPI::write(unsigned int address, const char *tx_buffer, size_t *tx_length) {
    return write(_write_instruction, address, _alt_value, tx_buffer, tx_length);
}

int QSPI::read(unsigned int instruction, unsigned int address, unsigned int alt, char *rx_buffer, size_t *rx_length) {
    int ret = 0;
    lock();
    _acquire();
    qspi_command_t *qspi_cmd = _build_qspi_command(instruction, address, alt);
    if(QSPI_STATUS_OK != qspi_read(&_qspi, qspi_cmd, rx_buffer, rx_length)) {
        //We got error status, return 0
        ret = 0;
    }
    unlock();
    
    return ret;
}

int QSPI::write(unsigned int instruction, unsigned int address, unsigned int alt, const char *tx_buffer, size_t *tx_length) {
    int ret = 1;
    lock();
    _acquire();
    qspi_command_t *qspi_cmd = _build_qspi_command(instruction, address, alt);
    if(QSPI_STATUS_OK != qspi_write(&_qspi, qspi_cmd, tx_buffer, tx_length)) {
        //We got error status, return 0
        ret = 0;
    }
    unlock();
    
    return ret;
}

int QSPI::perform_command(unsigned int instruction, unsigned int alt) { 
    int ret = 1;
    lock();
    _acquire();
    qspi_command_t *qspi_cmd = _build_qspi_command(instruction, -1, alt);
    if(QSPI_STATUS_OK != qspi_write_command(&_qspi, qspi_cmd)) {
        //We got error status, return 0
        ret = 0;
    }
    unlock();
    
    return ret;
}

void QSPI::lock() {
    _mutex->lock();
}

void QSPI::unlock() {
    _mutex->unlock();
}

qspi_command_t *QSPI::_build_qspi_command(int instruction, int address, int alt) {

    //Set up instruction phase parameters
    _qspi_command.instruction.bus_width = _inst_width; 
    _qspi_command.instruction.size = _inst_size; 
    _qspi_command.instruction.value =  instruction; 
    
    //Set up address phase parameters
    _qspi_command.address.bus_width = _address_width;
    if(address != -1) {
        _qspi_command.address.size = _address_size; 
        _qspi_command.address.value = address; 
    } else {
        //In the case command operations address may not be present, so set the address size to be NONE
        _qspi_command.address.size = QSPI_CFG_ADDR_SIZE_NONE; 
        _qspi_command.address.value = 0; 
    }
    
    //Set up alt phase parameters
    _qspi_command.alt.bus_width = _alt_width; 
    if(alt != -1) {
        _qspi_command.alt.size = _alt_size; 
        _qspi_command.alt.value = alt;  
    } else {
        //In the case command operations address may not be present, so set the address size to be NONE
        _qspi_command.alt.size = QSPI_CFG_ALT_SIZE_NONE; 
        _qspi_command.alt.value = 0; 
    }
    
    //Set up dummy cycle count
    _qspi_command.dummy_count = _num_dummy_cycles;
    
    //Set up bus width for data phase
    _qspi_command.data.bus_width = _data_width;
    
    return &_qspi_command;
}

} // namespace mbed

#endif
