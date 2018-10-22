#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG

#ifdef DEBUG
#define DMSG(args...)       Serial.print(args)
#define DMSG_STR(str)       Serial.println(str)
#define DMSG_HEX(num)       DMSG(num, HEX)
#define DMSG_INT(num)       DMSG(num)
#else
#define DMSG(args...)
#define DMSG_STR(str)
#define DMSG_HEX(num)
#define DMSG_INT(num)
#endif

#endif
