/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize interface for this library.
 * Created on: 2014-09-09
 */

/**
 * All Backup Area Flash storage index
 * |----------------------------|   Storage Size
 * | Environment variables area |   FLASH_ENV_SECTION_SIZE @see FLASH_ENV_SECTION_SIZE
 * |      1.system section      |   FLASH_ENV_SYSTEM_SIZE
 * |      2:data section        |   FLASH_ENV_SECTION_SIZE - FLASH_ENV_SYSTEM_SIZE
 * |----------------------------|
 * |      Saved log area        |   Storage size: @see FLASH_LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application size
 * |----------------------------|
 * |       Remain flash         |   All remaining
 * |----------------------------|
 *
 * Backup area storage index
 * 1.Environment variables area: @see FLASH_ENV_SECTION_SIZE
 * 2.Already downloaded application area for IAP function: unfixed size
 * 3.Remain
 *
 * Environment variables area has 2 section
 * 1.system section.(unit: 4 bytes, storage Environment variables's parameter)
 * 2.data section.(storage all environment variables)
 *
 */
#include "flash.h"

/**
 * Flash system initialize.
 *
 * @return result
 */
FlashErrCode flash_init(void) {
    extern FlashErrCode flash_port_init(uint32_t *env_addr, size_t *env_total_size,
            size_t *erase_min_size, flash_env const **default_env, size_t *default_env_size,
            size_t *log_size);
    extern FlashErrCode flash_env_init(uint32_t start_addr, size_t total_size,
            size_t erase_min_size, flash_env const *default_env, size_t default_env_size);
    extern FlashErrCode flash_iap_init(uint32_t start_addr);
    extern FlashErrCode flash_log_init(uint32_t start_addr, size_t log_size, size_t erase_min_size);

    uint32_t env_start_addr;
    size_t env_total_size = 0, erase_min_size = 0, default_env_set_size = 0, log_size = 0;
    const flash_env *default_env_set;
    FlashErrCode result = FLASH_NO_ERR;

    result = flash_port_init(&env_start_addr, &env_total_size, &erase_min_size, &default_env_set,
            &default_env_set_size, &log_size);

#ifdef FLASH_USING_ENV
    if (result == FLASH_NO_ERR) {
        result = flash_env_init(env_start_addr, env_total_size, erase_min_size, default_env_set,
                default_env_set_size);
    }
#endif

#ifdef FLASH_USING_IAP
    if (result == FLASH_NO_ERR) {
        if (flash_get_env_total_size() < erase_min_size) {
            result = flash_iap_init(env_start_addr + erase_min_size + log_size);
        } else if (flash_get_env_total_size() % erase_min_size == 0) {
            result = flash_iap_init(env_start_addr + flash_get_env_total_size() + log_size);
        } else {
            result = flash_iap_init((flash_get_env_total_size() / erase_min_size + 1) * erase_min_size + log_size);
        }
    }
#endif

#ifdef FLASH_USING_LOG
    if (result == FLASH_NO_ERR) {
        if (flash_get_env_total_size() < erase_min_size) {
            result = flash_log_init(env_start_addr + erase_min_size, log_size, erase_min_size);
        } else if (flash_get_env_total_size() % erase_min_size == 0) {
            result = flash_log_init(env_start_addr + flash_get_env_total_size(), log_size, erase_min_size);
        } else {
            result = flash_log_init((flash_get_env_total_size() / erase_min_size + 1) * erase_min_size,
                    log_size, erase_min_size);
        }
    }
#endif

    if (result == FLASH_NO_ERR) {
        FLASH_DEBUG("EasyFlash V%s is initialize success.\n", FLASH_SW_VERSION);
    } else {
        FLASH_DEBUG("EasyFlash V%s is initialize fail.\n", FLASH_SW_VERSION);
    }

    return result;
}
