/*
  Copyright (C) 2003-2009 FreeIPMI Core Team

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else /* !TIME_WITH_SYS_TIME */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else /* !HAVE_SYS_TIME_H */
#include <time.h>
#endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <assert.h>
#include <errno.h>

#include "freeipmi/sel-parse/ipmi-sel-parse.h"

#include "freeipmi/cmds/ipmi-sel-cmds.h"
#include "freeipmi/record-format/ipmi-sdr-record-format.h"
#include "freeipmi/record-format/ipmi-sel-record-format.h"
#include "freeipmi/spec/ipmi-event-reading-type-code-spec.h"
#include "freeipmi/spec/ipmi-event-reading-type-code-oem-spec.h"
#include "freeipmi/spec/ipmi-iana-enterprise-numbers-spec.h"
#include "freeipmi/spec/ipmi-product-id-spec.h"
#include "freeipmi/spec/ipmi-sensor-and-event-code-tables-spec.h"
#include "freeipmi/spec/ipmi-sensor-and-event-code-tables-oem-spec.h"
#include "freeipmi/spec/ipmi-sensor-numbers-oem-spec.h"
#include "freeipmi/spec/ipmi-sensor-types-spec.h"
#include "freeipmi/spec/ipmi-sensor-types-oem-spec.h"
#include "freeipmi/spec/ipmi-sensor-units-spec.h"
#include "freeipmi/spec/ipmi-slave-address-spec.h"
#include "freeipmi/spec/ipmi-slave-address-oem-spec.h"
#include "freeipmi/util/ipmi-iana-enterprise-numbers-util.h"
#include "freeipmi/util/ipmi-sensor-and-event-code-tables-util.h"

#include "ipmi-sel-parse-common.h"
#include "ipmi-sel-parse-defs.h"
#include "ipmi-sel-parse-string.h"
#include "ipmi-sel-parse-string-inventec.h"
#include "ipmi-sel-parse-trace.h"
#include "ipmi-sel-parse-util.h"

#include "freeipmi-portability.h"

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * in oem_rv, return
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_sensor_name (ipmi_sel_parse_ctx_t ctx,
					    struct ipmi_sel_parse_entry *sel_parse_entry,
					    uint8_t sel_record_type,
					    char *buf,
					    unsigned int buflen,
					    unsigned int flags,
					    unsigned int *wlen,
					    struct ipmi_sel_system_event_record_data *system_event_record_data,
					    int *oem_rv)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (buf);
  assert (buflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);
  assert (oem_rv);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441
      && ((system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS 
           && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_OEM_INVENTEC_BIOS
           && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_OEM_INVENTEC_BIOS
           && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_START)
          || (system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS 
              && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
              && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_SYSTEM_EVENT
              && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_OK)
          || (system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_POST_ERROR_CODE
              && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
              && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS
              && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_ERROR_CODE)
          || (system_event_record_data->generator_id == IPMI_SLAVE_ADDRESS_BMC
              && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
              && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS
              && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_PORT80_CODE_EVENT)))
    {
      if (ipmi_sel_parse_string_snprintf (buf,
					  buflen,
					  wlen,
					  "BIOS"))
        (*oem_rv) = 1;
      else
        (*oem_rv) = 0;
      
      return (1);
    }
  
  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_offset_class_oem (ipmi_sel_parse_ctx_t ctx,
						       struct ipmi_sel_parse_entry *sel_parse_entry,
						       uint8_t sel_record_type,
						       char *tmpbuf,
						       unsigned int tmpbuflen,
						       unsigned int flags,
						       unsigned int *wlen,
						       struct ipmi_sel_system_event_record_data *system_event_record_data)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (tmpbuf);
  assert (tmpbuflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   *
   * achu note: There is no official "string" defining the event
   * from the vendor.  "BMC enabled by BIOS" is simply what
   * occurs, so that's what I'm going to say.
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441
      && system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_START
      && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_OEM_INVENTEC_BIOS
      && !system_event_record_data->offset_from_event_reading_type_code /* no event */
      && system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE
      && system_event_record_data->event_data3_flag == IPMI_SEL_EVENT_DATA_OEM_CODE)
    {
      snprintf (tmpbuf,
                tmpbuflen,
                "BMC enabled by BIOS");

      return (1);
    }

  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_data2_discrete_oem (ipmi_sel_parse_ctx_t ctx,
							 struct ipmi_sel_parse_entry *sel_parse_entry,
							 uint8_t sel_record_type,
							 char *tmpbuf,
							 unsigned int tmpbuflen,
							 unsigned int flags,
							 unsigned int *wlen,
							 struct ipmi_sel_system_event_record_data *system_event_record_data)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (tmpbuf);
  assert (tmpbuflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);
  assert (system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441
      && system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_SMI
      && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_MEMORY
      && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_MEMORY
      && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
      && (system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_CORRECTABLE_MEMORY_ERROR
          || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_UNCORRECTABLE_MEMORY_ERROR
          || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_PARITY
          || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_CORRECTABLE_MEMORY_ERROR_LOGGING_LIMIT_REACHED)
      && system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE
      && (system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_SBE_WARNING_THRESHOLD
          || system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_SBE_CRITICAL_THRESHOLD
          || system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_OTHER))
    {
      /* achu: I'm assuming no output for this one */
      if (system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_OTHER)
        return (0);

      if (system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_SBE_WARNING_THRESHOLD)
        snprintf (tmpbuf,
                  tmpbuflen,
                  "SBE warning threshold");
      else /* system_event_record_data->event_data2 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA2_OEM_INVENTEC_SBE_CRITICAL_THRESHOLD */
        snprintf (tmpbuf,
                  tmpbuflen,
                  "SBE critical threshold");

      return (1);
    }

  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_data2_class_oem (ipmi_sel_parse_ctx_t ctx,
						      struct ipmi_sel_parse_entry *sel_parse_entry,
						      uint8_t sel_record_type,
						      char *tmpbuf,
						      unsigned int tmpbuflen,
						      unsigned int flags,
						      unsigned int *wlen,
						      struct ipmi_sel_system_event_record_data *system_event_record_data)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (tmpbuf);
  assert (tmpbuflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441
      && system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_START
      && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_OEM_INVENTEC_BIOS
      && !system_event_record_data->offset_from_event_reading_type_code /* no event */
      && system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE
      && system_event_record_data->event_data3_flag == IPMI_SEL_EVENT_DATA_OEM_CODE)
    {
      snprintf (tmpbuf,
                tmpbuflen,
                "BIOS Major Version %X",
                system_event_record_data->event_data2);

      return (1);
    }
  
  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_data3_discrete_oem (ipmi_sel_parse_ctx_t ctx,
							 struct ipmi_sel_parse_entry *sel_parse_entry,
							 uint8_t sel_record_type,
							 char *tmpbuf,
							 unsigned int tmpbuflen,
							 unsigned int flags,
							 unsigned int *wlen,
							 struct ipmi_sel_system_event_record_data *system_event_record_data)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (tmpbuf);
  assert (tmpbuflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);
  assert (system_event_record_data->event_data3_flag == IPMI_SEL_EVENT_DATA_OEM_CODE);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */

  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441)
    {
      /* achu: Note that the Dimm locations are not in a pattern,
       * this is what the doc says.
       *
       * If an invalid dimm location is indicated, fall through
       * and output normal stuff.
       */
      if (system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_SMI
          && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
          && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_MEMORY
          && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_MEMORY
          && (system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_CORRECTABLE_MEMORY_ERROR
              || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_UNCORRECTABLE_MEMORY_ERROR
              || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_PARITY
              || system_event_record_data->offset_from_event_reading_type_code == IPMI_SENSOR_TYPE_MEMORY_CORRECTABLE_MEMORY_ERROR_LOGGING_LIMIT_REACHED)
          && (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH0_DIM1
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH0_DIM0
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH1_DIM1
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH1_DIM0
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH2_DIM1
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH2_DIM0
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH0_DIM0
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH1_DIM0
              || system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH2_DIM0))
        {
          if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH0_DIM1)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch0/DIM1");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH0_DIM0)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch0/DIM0");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH1_DIM1)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch1/DIM1");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH1_DIM0)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch1/DIM0");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH2_DIM1)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch2/DIM1");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU0_CH2_DIM0)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU0/Ch2/DIM0");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH0_DIM0)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU1/Ch0/DIM0");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH1_DIM0)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU1/Ch1/DIM0");
          else /* system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_MEMORY_EVENT_DATA3_OEM_INVENTEC_DIMM_CPU1_CH2_DIM0 */
            snprintf (tmpbuf,
                      tmpbuflen,
                      "Dimm Number - CPU1/Ch2/DIM0");

          return (1);
        }

      if (system_event_record_data->generator_id == IPMI_SLAVE_ADDRESS_BMC
          && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
          && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS
          && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_PORT80_CODE_EVENT)
        {
          
          if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_EVENT_DATA3_OEM_INVENTEC_PORT80_CODE_EXTENDED_MEMORY_TEST)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "PORT80 Code Event = Extended Memory Test");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_EVENT_DATA3_OEM_INVENTEC_PORT80_CODE_SETUP_MENU)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "PORT80 Code Event = Setup Menu");
          else if (system_event_record_data->event_data3 == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_EVENT_DATA3_OEM_INVENTEC_PORT80_CODE_OPTION_ROM_SCAN)
            snprintf (tmpbuf,
                      tmpbuflen,
                      "PORT80 Code Event = Option ROM Scan");
          else
            snprintf (tmpbuf,
                      tmpbuflen,
                      "PORT80 Code Event = %02Xh",
                      system_event_record_data->event_data3);
          
          return (1);
        }
    }

  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_data3_class_oem (ipmi_sel_parse_ctx_t ctx,
						      struct ipmi_sel_parse_entry *sel_parse_entry,
						      uint8_t sel_record_type,
						      char *tmpbuf,
						      unsigned int tmpbuflen,
						      unsigned int flags,
						      unsigned int *wlen,
						      struct ipmi_sel_system_event_record_data *system_event_record_data)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (tmpbuf);
  assert (tmpbuflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441
      && system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS
      && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_OEM_INVENTEC_BIOS
      && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_START
      && !system_event_record_data->offset_from_event_reading_type_code /* no event */
      && system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE
      && system_event_record_data->event_data3_flag == IPMI_SEL_EVENT_DATA_OEM_CODE)
    {
      snprintf (tmpbuf,
                tmpbuflen,
                "BIOS Minor Version %02X",
                system_event_record_data->event_data3);
      
      return (1);
    }
  
  return (0);
}

/* return (0) - no OEM match
 * return (1) - OEM match
 * return (-1) - error, cleanup and return error
 *
 * in oem_rv, return
 * 0 - continue on
 * 1 - buffer full, return full buffer to user
 */
int
ipmi_sel_parse_output_inventec_event_data2_event_data3 (ipmi_sel_parse_ctx_t ctx,
							struct ipmi_sel_parse_entry *sel_parse_entry,
							uint8_t sel_record_type,
							char *buf,
							unsigned int buflen,
							unsigned int flags,
							unsigned int *wlen,
							struct ipmi_sel_system_event_record_data *system_event_record_data,
							int *oem_rv)
{
  assert (ctx);
  assert (ctx->magic == IPMI_SEL_PARSE_CTX_MAGIC);
  assert (sel_parse_entry);
  assert (buf);
  assert (buflen);
  assert (!(flags & ~IPMI_SEL_PARSE_STRING_MASK));
  assert (flags & IPMI_SEL_PARSE_STRING_FLAGS_INTERPRET_OEM_DATA);
  assert (wlen);
  assert (system_event_record_data);
  assert (oem_rv);

  /* OEM Interpretation
   *
   * Inventec 5441/Dell Xanadu2
   */
  if (ctx->manufacturer_id == IPMI_IANA_ENTERPRISE_ID_INVENTEC
      && ctx->product_id == IPMI_INVENTEC_PRODUCT_ID_5441)
    {
      if (system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_POST_ERROR_CODE
          && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_SENSOR_SPECIFIC
          && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS
          && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_ERROR_CODE)
        {
          uint16_t error_code;
          char *error_code_str = NULL;
          
          error_code = system_event_record_data->event_data2;
          error_code |= (system_event_record_data->event_data3 << 8);
          
          if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_TIMER_COUNT_READ_WRITE_ERROR)
            error_code_str = "Timer Count Read/Write Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_MASTER_PIC_ERROR)
            error_code_str = "Master PIC Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_SLAVE_PIC_ERROR)
            error_code_str = "Slave PIC Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_BATTERY_ERROR)
            error_code_str = "CMOS Battery Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_DIAGNOSTIC_STATUS_ERROR)
            error_code_str = "CMOS Diagnostic Status Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_CHECKSUM_ERROR)
            error_code_str = "CMOS Checksum Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_CONFIG_ERROR)
            error_code_str = "CMOS Config Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_KEYBOARD_LOCK_ERROR)
            error_code_str = "Keyboard Lock Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_NO_KEYBOARD_ERROR)
            error_code_str = "No Keyboard Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_KBC_BAT_TEST_ERROR)
            error_code_str = "KBC Bat Test Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_MEMORY_SIZE_ERROR)
            error_code_str = "CMOS Memory Size Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_RAM_READ_WRITE_TEST_ERROR)
            error_code_str = "RAM Read/Write Test Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_FDD_0_ERROR)
            error_code_str = "FDD 0 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_FLOPPY_CONTROLLER_ERROR)
            error_code_str = "Floppy Controller Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CMOS_DATE_TIME_ERROR)
            error_code_str = "CMOS Date Time Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_NO_PS2_MOUSE_ERROR)
            error_code_str = "No PS2 Mouse Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_REFRESH_TIMER_ERROR)
            error_code_str = "Refresh Timer Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_DISPLAY_MEMORY_ERROR)
            error_code_str = "Display Memory Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_POST_THE_INS_KEY_ERROR)
            error_code_str = "Post the <INS> key Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_DMAC_PAGE_REGISTER_ERROR)
            error_code_str = "DMAC Page Register Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_DMAC1_CHANNEL_REGISTER_ERROR)
            error_code_str = "DMAC1 Channel Register Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_DMAC2_CHANNEL_REGISTER_ERROR)
            error_code_str = "DMAC2 Channel Register Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_PMM_MEMORY_ALLOCATION_ERROR)
            error_code_str = "PMM Memory Allocation Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_PASSWORD_CHECK_ERROR)
            error_code_str = "Password Check Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ADM_MODULE_ERROR)
            error_code_str = "ADM Module Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_LANGUAGE_MODULE_ERROR)
            error_code_str = "Language Module Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_KBC_INTERFACE_ERROR)
            error_code_str = "KBC Interface Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_0_ERROR)
            error_code_str = "HDD 0 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_1_ERROR)
            error_code_str = "HDD 1 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_2_ERROR)
            error_code_str = "HDD 2 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_3_ERROR)
            error_code_str = "HDD 3 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_4_ERROR)
            error_code_str = "HDD 4 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_5_ERROR)
            error_code_str = "HDD 5 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_6_ERROR)
            error_code_str = "HDD 6 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_HDD_7_ERROR)
            error_code_str = "HDD 7 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_0_ERROR)
            error_code_str = "ATAPI 0 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_1_ERROR)
            error_code_str = "ATAPI 1 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_2_ERROR)
            error_code_str = "ATAPI 2 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_3_ERROR)
            error_code_str = "ATAPI 3 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_4_ERROR)
            error_code_str = "ATAPI 4 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_5_ERROR)
            error_code_str = "ATAPI 5 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_6_ERROR)
            error_code_str = "ATAPI 6 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATAPI_7_ERROR)
            error_code_str = "ATAPI 7 Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_ATA_SMART_FEATURE_ERROR)
            error_code_str = "ATA SMART Feature Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_NON_CRITICAL_PASSWORD_CHECK_ERROR)
            error_code_str = "Non-Critical Password Check Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_DUMMY_BIOS_ERROR)
            error_code_str = "Dummy BIOS Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_USB_HC_NOT_FOUND)
            error_code_str = "USB HC Not Found";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_USB_DEVICE_INIT_ERROR)
            error_code_str = "USB Device Init Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_USB_DEVICE_DISABLED)
            error_code_str = "USB Device Disabled";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_USB_OHCI_EMUL_NOT_SUPPORTED)
            error_code_str = "USB OHCI EMUL Not Supported";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_USB_EHCI_64BIT_DATA_STRUCTURE_ERROR)
            error_code_str = "USB EHCI 64bit Data Structure Error";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_SMBIOS_NOT_ENOUGH_SPACE_IN_F000)
            error_code_str = "SMBIOS Not Enough Space In F000";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_AP_APPLICATION_PROCESSOR_FAILED_BIST)
            error_code_str = "AP (Application Processor) failed BIST";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU1_THERMAL_FAILURE_DUE_TO_PROCHOT)
            error_code_str = "CPU1 Thermal Failure due to PROCHOT#";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU2_THERMAL_FAILURE_DUE_TO_PROCHOT)
            error_code_str = "CPU2 Thermal Failure due to PROCHOT#";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU3_THERMAL_FAILURE_DUE_TO_PROCHOT)
            error_code_str = "CPU3 Thermal Failure due to PROCHOT#";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU4_THERMAL_FAILURE_DUE_TO_PROCHOT)
            error_code_str = "CPU4 Thermal Failure due to PROCHOT#";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_PROCESSOR_FAILED_BIST_BSP)
            error_code_str = "Processor failed BIST (BSP)"; /* BSP = Baseboard Service Processor */
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU1_PROCESSOR_MISSING_MICROCODE)
            error_code_str = "CPU1 Processor missing microcode";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU2_PROCESSOR_MISSING_MICROCODE)
            error_code_str = "CPU2 Processor missing microcode";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU3_PROCESSOR_MISSING_MICROCODE)
            error_code_str = "CPU3 Processor missing microcode";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPU4_PROCESSOR_MISSING_MICROCODE)
            error_code_str = "CPU4 Processor missing microcode";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_L2_CACHE_SIZE_MISMATCH)
            error_code_str = "L2 cache size mismatch";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPUID_PROCESSOR_STEPPING_ARE_DIFFERENT)
            error_code_str = "CPUID, Processor stepping are different";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPUID_PROCESSOR_FAMILY_ARE_DIFFERENT)
            error_code_str = "CPUID, Processor family are different";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_FRONT_SIDE_BUS_MISMATCH)
            error_code_str = "Front side bus mismatch";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_CPUID_PROCESSOR_MODEL_ARE_DIFFERENT)
            error_code_str = "CPUID, Processor Model are different";
          else if (error_code == IPMI_SENSOR_TYPE_SYSTEM_FIRMWARE_PROGRESS_OEM_INVENTEC_POST_ERROR_CODE_PROCESSOR_SPEEDS_MISMATCHED)
            error_code_str = "Processor speeds mismatched";
          else
            error_code_str = "Undefined BIOS Error";
      
          if (ipmi_sel_parse_string_snprintf (buf,
					      buflen,
					      wlen,
					      "%s",
					      error_code_str))
            (*oem_rv) = 1;
          else
            (*oem_rv) = 0;
      
          return (1);
        }

      if (system_event_record_data->generator_id == IPMI_GENERATOR_ID_OEM_INVENTEC_BIOS
          && system_event_record_data->event_type_code == IPMI_EVENT_READING_TYPE_CODE_OEM_INVENTEC_BIOS
          && system_event_record_data->sensor_type == IPMI_SENSOR_TYPE_OEM_INVENTEC_BIOS
          && system_event_record_data->sensor_number == IPMI_SENSOR_NUMBER_OEM_INVENTEC_POST_START
          && !system_event_record_data->offset_from_event_reading_type_code /* no event */
          && system_event_record_data->event_data2_flag == IPMI_SEL_EVENT_DATA_OEM_CODE
          && system_event_record_data->event_data3_flag == IPMI_SEL_EVENT_DATA_OEM_CODE)
        {
          if (ipmi_sel_parse_string_snprintf (buf,
					      buflen,
					      wlen,
					      "BIOS Version %X.%02X",
					      system_event_record_data->event_data2,
					      system_event_record_data->event_data3))
            (*oem_rv) = 1;
          else
            (*oem_rv) = 0;
          
          return (1);
        }
    }

  return (0);
}
