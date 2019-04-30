/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_start_sec_types.h"
#include "stm32f4xx.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

zb_ieee_addr_t g_ze_addr = {0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};

static const zb_uint32_t g_code_color[] = {0xFFD700, 0xFF00FF, 0x6A5ACD, 0x228822, 0x00AAFA};
static zb_uint8_t g_index_color = 0;

static void init_periph (void);
static void check_btn_status (zb_uint8_t param) ZB_CALLBACK;
static void send_next_cmd (zb_uint8_t param) ZB_CALLBACK;
static void send_data (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_turn_on (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_turn_off (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_toggle (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_set_color (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_bright_up (zb_uint8_t param) ZB_CALLBACK;
static void send_cmd_bright_down (zb_uint8_t param) ZB_CALLBACK;

/*
  ZE joins to ZC(ZR), then sends APS packet.
*/


MAIN()
{
  ARGV_UNUSED;

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
  ZB_INIT((char*)"zdo_ze", (char*)"3", (char*)"3");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ze_addr);
  ZB_PIB_RX_ON_WHEN_IDLE() = ZB_FALSE;
  ZB_AIB().aps_channel_mask = (1l << 17);
	
  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    init_periph();
    ZB_SCHEDULE_ALARM(check_btn_status, param, 1);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

static void check_btn_status (zb_uint8_t param) ZB_CALLBACK
{
  #define PRESS_BORDER 120
  static uint32_t btn0_pressed = 0;
  static uint32_t btn1_pressed = 0;

  //обе кнопки зажаты
  if ((GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0) == 0) &&
       (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1) == 0)) 
  {
    btn0_pressed++;
    btn1_pressed++;
    if ((btn0_pressed >= PRESS_BORDER) && (btn1_pressed >= PRESS_BORDER))
    {
      btn0_pressed = 0;
      btn1_pressed = 0;

      //меняем цвет
      zb_buf_t *buf_tmp = ZB_BUF_FROM_REF(param);

      param_set_color_t * tmp_color;
      tmp_color = ZB_GET_BUF_TAIL (buf_tmp, sizeof(param_set_color_t));
      tmp_color->addr = 0;
      tmp_color->color = g_code_color[g_index_color];

      g_index_color++;
      if (g_index_color >= ARRAY_SIZE(g_code_color)) g_index_color = 0;

      ZB_SCHEDULE_CALLBACK(send_cmd_set_color, ZB_REF_FROM_BUF(buf_tmp));
      ZB_GET_OUT_BUF_DELAYED(check_btn_status);
    }
    else ZB_SCHEDULE_ALARM(check_btn_status, param, 1);
  }
  //нулевая кнопка зажата
  else if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0) == 0)
  {
    btn0_pressed++;
    if (btn0_pressed >= PRESS_BORDER)
    {
      btn0_pressed = 0;
      //переключаем светодиод (on/off)
      zb_buf_t *buf_tmp = ZB_BUF_FROM_REF(param);
      zb_uint16_t * paddr;
      paddr = ZB_GET_BUF_TAIL (buf_tmp, sizeof(zb_uint16_t));
      *paddr = 0;
 
      ZB_SCHEDULE_CALLBACK(send_cmd_toggle, ZB_REF_FROM_BUF(buf_tmp)); 
      ZB_GET_OUT_BUF_DELAYED(check_btn_status);  
    }
    else ZB_SCHEDULE_ALARM(check_btn_status, param, 1);
  }
  //первая кнопка зажата
  else if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1) == 0)
  {
    btn1_pressed++;
    if (btn1_pressed >= PRESS_BORDER)
    {
      btn1_pressed = 0;
      //повышаем яркость
      zb_buf_t *buf_tmp = ZB_BUF_FROM_REF(param);
      zb_uint16_t * paddr;
      paddr = ZB_GET_BUF_TAIL (buf_tmp, sizeof(zb_uint16_t));
      *paddr = 0;

      ZB_SCHEDULE_CALLBACK(send_cmd_bright_up, ZB_REF_FROM_BUF(buf_tmp)); 
      ZB_GET_OUT_BUF_DELAYED(check_btn_status);
    }
    else ZB_SCHEDULE_ALARM(check_btn_status, param, 1);
  }
  //кнопки не нажаты
  else
  {
    btn0_pressed = 0;
    btn1_pressed = 0;
    ZB_SCHEDULE_ALARM(check_btn_status, param, 1);
  }
}

static void send_cmd_turn_off (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_turn_off", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t), ptr);
  *ptr = CMD_TURN_OFF;
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_cmd_turn_on (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_turn_on", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t), ptr);
  *ptr = CMD_TURN_ON;
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_cmd_toggle (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_toggle", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t), ptr);
  *ptr = CMD_TOGGLE;
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_cmd_set_color (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_set_color", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;
  param_set_color_t * param_tmp;
  zb_uint32_t tmp_color;
  zb_uint16_t addr;
  zb_uint16_t * paddr;

  param_tmp = ZB_GET_BUF_TAIL (buf, sizeof(param_set_color_t));
  tmp_color = param_tmp->color;
  addr = param_tmp->addr;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t) + sizeof(zb_uint32_t), ptr);
  *ptr = CMD_SET_COLOR;
  *((zb_uint32_t *) (ptr+1)) = tmp_color;

  paddr = ZB_GET_BUF_TAIL (buf, sizeof(zb_uint16_t));
  *paddr = addr;
 
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_cmd_bright_up (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_up_bright", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t), ptr);
  *ptr = CMD_BRIGHT_UP;
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_cmd_bright_down (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "CMD: send_cmd_down_bright", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  cmd_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(cmd_t), ptr);
  *ptr = CMD_BRIGHT_DOWN;
  ZB_SCHEDULE_CALLBACK(send_data, param);
}

static void send_data (zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS1, "send_data", (FMT__0));
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
  zb_uint16_t * paddr = ZB_GET_BUF_TAIL (buf, sizeof(zb_uint16_t));
  zb_uint16_t addr = *paddr;

  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = addr; /* send to ZC */
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;

  buf->u.hdr.handle = 0x11;

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}


static void init_periph () {

  /* Init GPIO */
  GPIO_InitTypeDef GPIO_InitStructure;

  //GPIOE, 2 buttons
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
}
/*! @} */
