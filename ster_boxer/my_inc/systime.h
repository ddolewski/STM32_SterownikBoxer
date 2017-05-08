/*
 * systime.h
 *
 *  Created on: 25-04-2012
 *      Author: Krzysztof F
 */

#ifndef SYSTIME_H_
#define SYSTIME_H_

#include <stdint.h>
#include "global.h"

#define SYSTICKHZ      100
#define SYSTICKMS      (1000 / SYSTICKHZ)
#define SYSTICKUS      (1000000 / SYSTICKHZ)
#define SYSTICKNS      (1000000000 / SYSTICKHZ)

typedef uint32_t systime_t;

void systimeInit (void);
systime_t systimeGet (void);
uint8_t systimeTimeoutControl(systime_t * xLastTime, systime_t xCheckTime);
void  systimeDelayMs(uint32_t xTimeMs);

#endif /* SYSTIME_H_ */
