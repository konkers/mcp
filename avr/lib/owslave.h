/*
 * Copyright 2014 Erik Gilling <konkers@konkers.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the Licene.
 */

#ifndef __MCP_AVR_LIB_OWSLAVE_H__
#define __MCP_AVR_LIB_OWSLAVE_H__

#include <avr/pgmspace.h>

void ows_init(volatile uint8_t *dq_pin,
              volatile uint8_t *dq_port,
              volatile uint8_t *dq_ddr,
              volatile uint8_t dq_mask);


/* slave device needs to implement/devfine these */
extern const PROGMEM uint8_t ows_addr[8];
void ows_dev_scratchpad_update(uint8_t idx, uint8_t data);
uint8_t ows_dev_get_scratchpad_read_size(void);
uint8_t ows_dev_scratchpad_read(uint8_t idx);
#endif /* __MCP_AVR_LIB_OWSLAVE_H__ */

