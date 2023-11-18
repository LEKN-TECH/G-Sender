#ifndef cmd_cache
#define cmd_cache

#include "gsender.h"

#define CMD_WRITE	0
#define CMD_READ	1

#define CMD_SIZE			48
#define CMDCACHE_SIZE 		4
#define CMDCACHE_RING (CMDCACHE_SIZE+1)

#define CMCACHE_NORMAL 0xfd
#define CMDCACHE_NO_DATA 0xfe
#define CMDCACHE_FULL	0xff

typedef struct
{
  uint8_t cmd_len;
  uint8_t cmd[CMD_SIZE];
}cmd_t;

uint8_t cmdcache_wirte(uint8_t *cmd, uint8_t len);
uint8_t cmdcache_read(uint8_t *cmd);
void cmdcache_flush(void);
uint8_t cmdcache_state(uint8_t flag);

#endif
