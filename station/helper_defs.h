#ifndef HELPER_DEFS_H
#define HELPER_DEFS_H

#define DEBUG 0

#define DEBUGPRINT(x) \
if(DEBUG) \
{ \
  Serial.print((x)); \
}

#define DEBUGPRINTHEX(x) \
if(DEBUG) \
{ \
  Serial.print((x),HEX); \
}

#define DEBUGPRINTLN(x) \
if(DEBUG) \
{ \
  Serial.println((x)); \
}

#endif // HELPER_DEFS_H
