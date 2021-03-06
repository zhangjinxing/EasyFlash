/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
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
 * Function: IAP(In-Application Programming) operating interface.
 * Created on: 2015-01-05
 */

#include "flash.h"

#ifdef FLASH_USING_IAP

/* IAP section backup application section start address in flash */
static uint32_t bak_app_start_addr = NULL;

static uint32_t get_bak_app_start_addr(void);

/**
 * Flash IAP function initialize.
 *
 * @param start_addr IAP section backup application section start address in flash
 *
 * @return result
 */
FlashErrCode flash_iap_init(uint32_t start_addr) {
    FlashErrCode result = FLASH_NO_ERR;

    FLASH_ASSERT(start_addr);

    bak_app_start_addr = start_addr;
    return result;
}

/**
 * Erase backup area application data.
 *
 * @param app_size application size
 *
 * @return result
 */
FlashErrCode flash_erase_bak_app(size_t app_size) {
    FlashErrCode result = FLASH_NO_ERR;

    result = flash_erase(get_bak_app_start_addr(), app_size);
    switch (result) {
    case FLASH_NO_ERR: {
        FLASH_INFO("Erased backup area application OK.\n");
        break;
    }
    case FLASH_ERASE_ERR: {
        FLASH_INFO("Warning: Erase backup area application fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}


/**
 * Erase user old application
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 *
 * @return result
 */
FlashErrCode flash_erase_user_app(uint32_t user_app_addr, size_t app_size) {
    FlashErrCode result = FLASH_NO_ERR;

    result = flash_erase(user_app_addr, app_size);
    switch (result) {
    case FLASH_NO_ERR: {
        FLASH_INFO("Erased user application OK.\n");
        break;
    }
    case FLASH_ERASE_ERR: {
        FLASH_INFO("Warning: Erase user application fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}

/**
 * Erase old bootloader
 *
 * @param bl_addr bootloader entry address
 * @param bl_size bootloader size
 *
 * @return result
 */
FlashErrCode flash_erase_bl(uint32_t bl_addr, size_t bl_size) {
    FlashErrCode result = FLASH_NO_ERR;

    result = flash_erase(bl_addr, bl_size);
    switch (result) {
    case FLASH_NO_ERR: {
        FLASH_INFO("Erased bootloader OK.\n");
        break;
    }
    case FLASH_ERASE_ERR: {
        FLASH_INFO("Warning: Erase bootloader fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}

/**
 * Write data of application to backup area.
 *
 * @param data a part of application
 * @param size data size
 * @param cur_size current write application size
 * @param total_size application total size
 *
 * @return result
 */
FlashErrCode flash_write_data_to_bak(uint8_t *data, size_t size, size_t *cur_size,
        size_t total_size) {
    FlashErrCode result = FLASH_NO_ERR;

    /* make sure don't write excess data */
    if (*cur_size + size > total_size) {
        size = total_size - *cur_size;
    }

    result = flash_write(get_bak_app_start_addr() + *cur_size, (uint32_t *) data, size);
    switch (result) {
    case FLASH_NO_ERR: {
        *cur_size += size;
        FLASH_INFO("Write data to backup area OK.\n");
        break;
    }
    case FLASH_WRITE_ERR: {
        FLASH_INFO("Warning: Write data to backup area fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Copy backup area application to application entry.
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 *
 * @return result
 */
FlashErrCode flash_copy_app_from_bak(uint32_t user_app_addr, size_t app_size) {
    size_t cur_size;
    uint32_t app_cur_addr, bak_cur_addr;
    FlashErrCode result = FLASH_NO_ERR;
    /* 32 words size buffer */
    uint32_t buff[32];

    /* cycle copy data */
    for (cur_size = 0; cur_size < app_size; cur_size += sizeof(buff)) {
        app_cur_addr = user_app_addr + cur_size;
        bak_cur_addr = get_bak_app_start_addr() + cur_size;
        flash_read(bak_cur_addr, buff, sizeof(buff));
        result = flash_write(app_cur_addr, buff, sizeof(buff));
        if (result != FLASH_NO_ERR) {
            break;
        }
    }

    switch (result) {
    case FLASH_NO_ERR: {
        FLASH_INFO("Write data to application entry OK.\n");
        break;
    }
    case FLASH_WRITE_ERR: {
        FLASH_INFO("Warning: Write data to application entry fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Copy backup area bootloader to bootloader entry.
 *
 * @param bl_addr bootloader entry address
 * @param bl_size bootloader size
 *
 * @return result
 */
FlashErrCode flash_copy_bl_from_bak(uint32_t bl_addr, size_t bl_size) {
    size_t cur_size;
    uint32_t bl_cur_addr, bak_cur_addr;
    FlashErrCode result = FLASH_NO_ERR;
    /* 32 words buffer */
    uint32_t buff[32];

    /* cycle copy data by 32bytes buffer */
    for (cur_size = 0; cur_size < bl_size; cur_size += sizeof(buff)) {
        bl_cur_addr = bl_addr + cur_size;
        bak_cur_addr = get_bak_app_start_addr() + cur_size;
        flash_read(bak_cur_addr, buff, sizeof(buff));
        result = flash_write(bl_cur_addr, buff, sizeof(buff));
        if (result != FLASH_NO_ERR) {
            break;
        }
    }

    switch (result) {
    case FLASH_NO_ERR: {
        FLASH_INFO("Write data to bootloader entry OK.\n");
        break;
    }
    case FLASH_WRITE_ERR: {
        FLASH_INFO("Warning: Write data to bootloader entry fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Get IAP section start address in flash.
 *
 * @return size
 */
static uint32_t get_bak_app_start_addr(void) {
    FLASH_ASSERT(bak_app_start_addr);
    return bak_app_start_addr;
}

#endif /* FLASH_USING_IAP */
