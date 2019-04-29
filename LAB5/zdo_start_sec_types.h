#ifndef ZDO_START_SEC_TYPES_H
#define ZDO_START_SEC_TYPES_H

typedef enum {
  CMD_TURN_OFF     = 'A',
  CMD_TURN_ON      = 'B',
  CMD_TOGGLE       = 'C',
  CMD_SET_COLOR    = 'D',
  CMD_BRIGHT_UP    = 'E',
  CMD_BRIGHT_DOWN  = 'F'
} cmd_t;
  
typedef struct {
  zb_uint32_t color;
  zb_uint16_t addr;
} param_set_color_t;

#endif
