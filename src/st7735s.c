/* MIT License
 * Copyright (c) 2021, Michal Kozakiewicz, github.com/michal037
 *
 * code repository: https://github.com/michal037/driver-ST7735S
 * code version: 3.0.0
 *
 * There are many references to ST7735S Datasheet in this code,
 * please provide it as well.
 * When updating a PDF, update all references to the appropriate
 * version and page. This will avoid delivering multiple PDF files.
 * Convention used: (pdf VERSION PAGE/S)
 * Convention example: (pdf v1.4 p10) or (pdf v1.4 p68-70)
 */

#include <string.h> /* memcpy */
#include "../include/st7735s.h"
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>

#define HIGH 1
#define LOW  0

/* documented in header; see struct lcd_t */
#define DATAMODE_ACTIVESTATE HIGH
#define RESET_ACTIVESTATE    LOW

/* MV flag default state based on the Reset Table, see: (pdf v1.4 p89-91) */
#define FLAG_MADCTL_MV_DEFAULT 0

/* default pixel format; Based on the Reset Table, see: (pdf v1.4 p89-91) */
#define INTERFACE_PIXEL_FORMAT_DEFAULT LCD_PIXEL_FORMAT_666

/* ST7735S driver commands (pdf v1.4 p5) */
#define LCD_CMD_SWRESET 0x01 /* Software Reset (pdf v1.4 p108) */
#define LCD_CMD_SLPIN   0x10 /* Sleep In (pdf v1.4 p119) */
#define LCD_CMD_SLPOUT  0x11 /* Sleep Out (pdf v1.4 p120) */
#define LCD_CMD_INVOFF  0x20 /* Display Inversion Off (pdf v1.4 p123) */
#define LCD_CMD_INVON   0x21 /* Display Inversion On (pdf v1.4 p124) */
#define LCD_CMD_GAMSET  0x26 /* Gamma Set (pdf v1.4 p125) */
#define LCD_CMD_DISPOFF 0x28 /* Display Off (pdf v1.4 p126) */
#define LCD_CMD_DISPON  0x29 /* Display On (pdf v1.4 p127) */
#define LCD_CMD_CASET   0x2A /* Column Address Set (pdf v1.4 p128-129) */
#define LCD_CMD_RASET   0x2B /* Row Address Set (pdf v1.4 p130-131) */
#define LCD_CMD_RAMWR   0x2C /* Memory Write (pdf v1.4 p132) */
#define LCD_CMD_TEOFF   0x34 /* Tearing Effect Line OFF (pdf v1.4 p139) */
#define LCD_CMD_TEON    0x35 /* Tearing Effect Line ON (pdf v1.4 p140) */
#define LCD_CMD_MADCTL  0x36 /* Memory Data Access Control (pdf v1.4 p142) */
#define LCD_CMD_IDMOFF  0x38 /* Idle Mode Off (pdf v1.4 p147) */
#define LCD_CMD_IDMON   0x39 /* Idle Mode On (pdf v1.4 p148) */
#define LCD_CMD_COLMOD  0x3A /* Interface Pixel Format (pdf v1.4 p150) */

/* global variable with settings of the active display module */
lcd_ptr_t lcd_settings = NULL;

lcd_ptr_t lcd_createSettings(
    unsigned short int width,
    unsigned short int height,
    unsigned short int width_offset,
    unsigned short int height_offset,
    unsigned short int pin_communicationMode,
    signed   short int pin_reset
) {
    lcd_ptr_t settings = malloc(sizeof(struct lcd_t));

    /* if out of RAM memory */
    if(settings == NULL) {
        return NULL;
    }

    settings->width  = width;
    settings->height = height;
    settings->width_offset  = width_offset;  /* width_offset  :: column :: X */
    settings->height_offset = height_offset; /* height_offset :: row    :: Y */

    settings->pin_communicationMode = pin_communicationMode;
    settings->pin_reset             = pin_reset;

    settings->dataMode_activeState = DATAMODE_ACTIVESTATE;
    settings->reset_activeState    = RESET_ACTIVESTATE;

    /* HAVE TO BE REFRESHED IN INITIALIZATION TO PREVENT BUGS!
     * Use: lcd_setMemoryAccessControl()
     */
    settings->flag_madctl_mv = FLAG_MADCTL_MV_DEFAULT;

    /* HAVE TO BE REFRESHED IN INITIALIZATION TO PREVENT BUGS!
     * Use: lcd_setInterfacePixelFormat()
     */
    settings->interface_pixel_format = INTERFACE_PIXEL_FORMAT_DEFAULT;

    return settings;
}

void lcd_deleteSettings(lcd_ptr_t settings) {
    /* There is no need to check if the pointer is NULL before calling
     * the free() function
     */

    /* Prevent a bug If someone tries to draw through dangling pointer. */
    if(settings == lcd_settings) {
        lcd_settings = NULL;
    }

    /* previously allocated by lcd_createSettings() */
    free(settings);
}

void lcd_setSettingsActive(lcd_ptr_t settings) {
    /* only rewrite; allow NULL pointer */
    lcd_settings = settings;
}

lcd_ptr_t lcd_getSettingsActive() {
    return lcd_settings;
}

lcd_status_t lcd_writeData(unsigned char* buffer, size_t length) {
    /* check inputs */
    if(buffer == NULL) {
        return LCD_FAIL;
    }
    if(length <= 0) {
        return LCD_FAIL;
    }

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* write data to the display driver */
    lcd_digitalWrite(
        lcd_settings->pin_communicationMode,
        lcd_settings->dataMode_activeState
    );
    lcd_spiWrite(buffer, length);

    return LCD_OK;
}

lcd_status_t lcd_writeCommand(unsigned char* buffer, size_t length) {
    /* check inputs */
    if(buffer == NULL) {
        return LCD_FAIL;
    }
    if(length <= 0) {
        return LCD_FAIL;
    }

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* write command to the display driver */
    lcd_digitalWrite(
        lcd_settings->pin_communicationMode,
        !( lcd_settings->dataMode_activeState ) /* command is negative state */
    );
    lcd_spiWrite(buffer, length);

    return LCD_OK;
}

lcd_status_t lcd_writeCommandByte(unsigned char command) {
    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* write command to the display driver */
    lcd_digitalWrite(
        lcd_settings->pin_communicationMode,
        !( lcd_settings->dataMode_activeState ) /* command is negative state */
    );
    lcd_spiWrite(&command, sizeof(command));

    return LCD_OK;
}

lcd_status_t lcd_hardwareReset() {
    /* Reset Timing (pdf v1.4 p93) */

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* check if the reset pin is connected */
    if(lcd_settings->pin_reset < 0) {
        return LCD_FAIL;
    }

    /* activate reset */
    lcd_digitalWrite(
        lcd_settings->pin_reset,
        lcd_settings->reset_activeState
    );
    lcd_delay(1); /* 1 millisecond; have to be longer than 9us (pdf v1.4 p93) */

    /* cancel reset */
    lcd_digitalWrite(
        lcd_settings->pin_reset,
        !( lcd_settings->reset_activeState )
    );
    lcd_delay(120); /* 120 milliseconds */

    return LCD_OK;
}

lcd_status_t lcd_softwareReset() {
    if(lcd_writeCommandByte(LCD_CMD_SWRESET) < LCD_OK) {
        return LCD_FAIL;
    }

    /* It is required to wait 120 milliseconds after issuing the command. */
    lcd_delay(120); /* (pdf v1.4 p108) */

    return LCD_OK;
}

lcd_status_t lcd_initialize() {
    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* If the 'pin_reset' is connected (>= 0) then disable hardware reset. */
    if(lcd_settings->pin_reset >= 0) {
        lcd_digitalWrite(
            lcd_settings->pin_reset,
            /* inactive is negative state */
            !( lcd_settings->reset_activeState )
        );
    }

    return LCD_OK;
}

lcd_status_t lcd_setSleepMode(unsigned char sleep) {
    /* This command turns ON or OFF sleep mode. */
    if(lcd_writeCommandByte(sleep ? LCD_CMD_SLPIN : LCD_CMD_SLPOUT) < LCD_OK) {
        return LCD_FAIL;
    }

    /* It is required to wait 120 milliseconds after issuing the command. */
    lcd_delay(120); /* (pdf v1.4 p119-120) */

    return LCD_OK;
}

lcd_status_t lcd_setIdleMode(unsigned char idle) {
    /* This command turns ON or OFF idle mode. */
    if(lcd_writeCommandByte(idle ? LCD_CMD_IDMON : LCD_CMD_IDMOFF) < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_setDisplayMode(unsigned char display) {
    lcd_status_t status;

    /* This command turns ON or OFF the display. */
    status = lcd_writeCommandByte(display ? LCD_CMD_DISPON : LCD_CMD_DISPOFF);
    if(status < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_setDisplayInversion(unsigned char inversion) {
    lcd_status_t status;

    /* This command turns ON or OFF display inversion. */
    status = lcd_writeCommandByte(inversion ? LCD_CMD_INVON : LCD_CMD_INVOFF);
    if(status < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_setGammaPredefined(unsigned char gamma) {
    /* check the input value; cannot be combined */
    switch(gamma) {
        /* correct arguments */
        case LCD_GAMMA_PREDEFINED_1: break;
        case LCD_GAMMA_PREDEFINED_2: break;
        case LCD_GAMMA_PREDEFINED_3: break;
        case LCD_GAMMA_PREDEFINED_4: break;

        /* incorrect argument */
        default: return LCD_FAIL;
    }

    /* Gamma Set */
    if(lcd_writeCommandByte(LCD_CMD_GAMSET) < LCD_OK) {
        return LCD_FAIL;
    }

    /* write argument */
    if(lcd_writeData(&gamma, sizeof(gamma)) < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_setTearingEffectLine(unsigned char tearing) {
    /* check the input argument */
    if((tearing == LCD_TEARING_MODE_V) || (tearing == LCD_TEARING_MODE_VH)) {
        /* turn on tearing effect line */
        if(lcd_writeCommandByte(LCD_CMD_TEON) < LCD_OK) {
            return LCD_FAIL;
        }
        if(lcd_writeData(&tearing, sizeof(tearing)) < LCD_OK) {
            return LCD_FAIL;
        }
    } else {
        /* turn off tearing effect line */
        if(lcd_writeCommandByte(LCD_CMD_TEOFF) < LCD_OK) {
            return LCD_FAIL;
        }

        /* turning off does not require sending data about arguments */
    }

    return LCD_OK;
}

lcd_status_t lcd_setMemoryAccessControl(unsigned char flags) {
    unsigned char flags_copy = flags;

    /* Memory Access Control */
    if(lcd_writeCommandByte(LCD_CMD_MADCTL) < LCD_OK) {
        return LCD_FAIL;
    }

    /* write flags */
    if(lcd_writeData(&flags_copy, sizeof(flags_copy)) < LCD_OK) {
        return LCD_FAIL;
    }

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* if the new state of the MADCTL_MV flag is different than the previous */
    if((flags & LCD_MADCTL_MV) != lcd_settings->flag_madctl_mv) {
        unsigned short int temp;

        /* swap width with height */
        temp = lcd_settings->width;
        lcd_settings->width  = lcd_settings->height;
        lcd_settings->height = temp;

        /* swap width_offset with height_offset */
        temp = lcd_settings->width_offset;
        lcd_settings->width_offset  = lcd_settings->height_offset;
        lcd_settings->height_offset = temp;

        /* update flag_madctl_mv */
        lcd_settings->flag_madctl_mv = flags & LCD_MADCTL_MV;
    }

    return LCD_OK;
}

lcd_status_t lcd_setInterfacePixelFormat(unsigned char format) {
    unsigned char format_copy = format;

    /* check the input value; cannot be combined */
    switch(format) {
        /* correct arguments */
        case LCD_PIXEL_FORMAT_444: break;
        case LCD_PIXEL_FORMAT_565: break;
        case LCD_PIXEL_FORMAT_666: break;

        /* incorrect argument */
        default: return LCD_FAIL;
    }

    /* Interface Pixel Format Set */
    if(lcd_writeCommandByte(LCD_CMD_COLMOD) < LCD_OK) {
        return LCD_FAIL;
    }

    /* write argument */
    if(lcd_writeData(&format_copy, sizeof(format_copy)) < LCD_OK) {
        return LCD_FAIL;
    }

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* save interface pixel format for drawing functions */
    lcd_settings->interface_pixel_format = format;

    return LCD_OK;
}

lcd_status_t lcd_setWindowPosition(
    unsigned short int column_start,
    unsigned short int row_start,
    unsigned short int column_end,
    unsigned short int row_end
) {
    unsigned char buffer[4]; /* four bytes */

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* column ranges: (pdf v1.4 p105) (pdf v1.4 p128)
     * 'column_start' always must be less than or equal to 'column_end'.
     * When 'column_start' or 'column_end' are greater than maximum column
     * address, data of out of range will be ignored.
     *
     * 0 <= column_start <= column_end <= COLUMN_MAX
     */
    if(column_start > column_end) {
        return LCD_FAIL;
    }

    /* row ranges: (pdf v1.4 p105) (pdf v1.4 p130)
     * 'row_start' always must be less than or equal to 'row_end'.
     * When 'row_start' or 'row_end' are greater than maximum row address,
     * data of out of range will be ignored.
     *
     * 0 <= row_start <= row_end <= ROW_MAX
     */
    if(row_start > row_end) {
        return LCD_FAIL;
    }

    /* append offset to variables; after checking the ranges */
    /* width_offset  :: column :: X */
    /* height_offset :: row    :: Y */
    column_start += lcd_settings->width_offset;
    column_end   += lcd_settings->width_offset;
    row_start    += lcd_settings->height_offset;
    row_end      += lcd_settings->height_offset;

    /* write column address; requires 4 bytes of the buffer */
    buffer[0] = (column_start >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[1] =  column_start       & 0x00FF; /* LSB */
    buffer[2] = (column_end   >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[3] =  column_end         & 0x00FF; /* LSB */
    if(lcd_writeCommandByte(LCD_CMD_CASET) < LCD_OK) {
        return LCD_FAIL;
    }
    if(lcd_writeData(buffer, 4) < LCD_OK) {
        return LCD_FAIL;
    }

    /* write row address; requires 4 bytes of the buffer */
    buffer[0] = (row_start >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[1] =  row_start       & 0x00FF; /* LSB */
    buffer[2] = (row_end   >> 8) & 0x00FF; /* MSB */ /* =0 for ST7735S */
    buffer[3] =  row_end         & 0x00FF; /* LSB */
    if(lcd_writeCommandByte(LCD_CMD_RASET) < LCD_OK) {
        return LCD_FAIL;
    }
    if(lcd_writeData(buffer, 4) < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_activateMemoryWrite() {
    /* This command activate memory write. */
    if(lcd_writeCommandByte(LCD_CMD_RAMWR) < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_drawPixel(
    unsigned short int x,
    unsigned short int y,
    unsigned char      red,
    unsigned char      green,
    unsigned char      blue
) {
    unsigned char buffer[3]; /* three bytes */

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    if(lcd_setWindowPosition(x, y, x, y) < LCD_OK) {
        return LCD_FAIL;
    }

    if(lcd_activateMemoryWrite() < LCD_OK) {
        return LCD_FAIL;
    }

    /* prepare buffer and write data */
    switch(lcd_settings->interface_pixel_format) {
        /* case LCD_PIXEL_FORMAT_444 is not supported for this function */

        case LCD_PIXEL_FORMAT_565:
            buffer[0] = (red & 0xF8)  |  ((green >> 5) & 0x07);
            buffer[1] = ((green << 3) & 0xE0)  |  ((blue >> 3) & 0x1F);

            if(lcd_writeData(buffer, 2) < LCD_OK) {
                return LCD_FAIL;
            }
            break;

        case LCD_PIXEL_FORMAT_666:
            buffer[0] = red;
            buffer[1] = green;
            buffer[2] = blue;

            if(lcd_writeData(buffer, 3) < LCD_OK) {
                return LCD_FAIL;
            }
            break;

        /* unknown interface pixel format */
        default:
            return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_drawHorizontalLine(
    unsigned short int x0,
    unsigned short int y0,
    unsigned short int x1,
    unsigned char      red,
    unsigned char      green,
    unsigned char      blue
) {
    unsigned short int length;
    unsigned char buffer[3];      /* three bytes */
    unsigned char buffer_temp[3]; /* three bytes */
    unsigned char buffer_required_size = 0;

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* prepare buffer */
    switch(lcd_settings->interface_pixel_format) {
        /* case LCD_PIXEL_FORMAT_444 is not supported for this function */

        case LCD_PIXEL_FORMAT_565:
            buffer[0] = (red & 0xF8)  |  ((green >> 5) & 0x07);
            buffer[1] = ((green << 3) & 0xE0)  |  ((blue >> 3) & 0x1F);

            buffer_required_size = 2; /* two bytes required */
            break;

        case LCD_PIXEL_FORMAT_666:
            buffer[0] = red;
            buffer[1] = green;
            buffer[2] = blue;

            buffer_required_size = 3; /* three bytes required */
            break;

        /* unknown interface pixel format */
        default:
            return LCD_FAIL;
    }

    /* if(x0 > x1) then lcd_setWindowPosition will return LCD_FAIL */
    if(lcd_setWindowPosition(x0, y0, x1, y0) < LCD_OK) {
        return LCD_FAIL;
    }

    if(lcd_activateMemoryWrite() < LCD_OK) {
        return LCD_FAIL;
    }

    /* write data */
    for(length = x1 - x0 + 1; length > 0; length--) {
        /* copy buffer to buffer_temp */
        memcpy(buffer_temp, buffer, buffer_required_size);

        /* send a copy of the buffer; copy is overwritten with incoming data */
        if(lcd_writeData(buffer_temp, buffer_required_size) < LCD_OK) {
            return LCD_FAIL;
        }
    }

    return LCD_OK;
}

lcd_status_t lcd_drawVerticalLine(
    unsigned short int x0,
    unsigned short int y0,
    unsigned short int y1,
    unsigned char      red,
    unsigned char      green,
    unsigned char      blue
) {
    unsigned short int length;
    unsigned char buffer[3];      /* three bytes */
    unsigned char buffer_temp[3]; /* three bytes */
    unsigned char buffer_required_size = 0;

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* prepare buffer */
    switch(lcd_settings->interface_pixel_format) {
        /* case LCD_PIXEL_FORMAT_444 is not supported for this function */

        case LCD_PIXEL_FORMAT_565:
            buffer[0] = (red & 0xF8)  |  ((green >> 5) & 0x07);
            buffer[1] = ((green << 3) & 0xE0)  |  ((blue >> 3) & 0x1F);

            buffer_required_size = 2; /* two bytes required */
            break;

        case LCD_PIXEL_FORMAT_666:
            buffer[0] = red;
            buffer[1] = green;
            buffer[2] = blue;

            buffer_required_size = 3; /* three bytes required */
            break;

        /* unknown interface pixel format */
        default:
            return LCD_FAIL;
    }

    /* if(y0 > y1) then lcd_setWindowPosition will return LCD_FAIL */
    if(lcd_setWindowPosition(x0, y0, x0, y1) < LCD_OK) {
        return LCD_FAIL;
    }

    if(lcd_activateMemoryWrite() < LCD_OK) {
        return LCD_FAIL;
    }


    /* write data */
    for(length = y1 - y0 + 1; length > 0; length--) {
        /* copy buffer to buffer_temp */
        memcpy(buffer_temp, buffer, buffer_required_size);

        /* send a copy of the buffer; copy is overwritten with incoming data */
        if(lcd_writeData(buffer_temp, buffer_required_size) < LCD_OK) {
            return LCD_FAIL;
        }
    }

    return LCD_OK;
}

lcd_status_t lcd_drawRectangle(
    unsigned short int x0,
    unsigned short int y0,
    unsigned short int x1,
    unsigned short int y1,
    unsigned char      red,
    unsigned char      green,
    unsigned char      blue
) {
    /* see: documentation/notes/lcd_drawRectangle/lcd_drawRectangle.png */

    if(
        ((x1 - x0 + 1) >= 3) /* width  >= 3 */
        &&
        ((y1 - y0 + 1) >= 3) /* height >= 3 */
    ) {
        /* Horizontal 1 */
        if(lcd_drawHorizontalLine(x0, y0, x1, red, green, blue) < LCD_OK) {
            return LCD_FAIL;
        }

        /* Horizontal 2 */
        if(lcd_drawHorizontalLine(x0, y1, x1, red, green, blue) < LCD_OK) {
            return LCD_FAIL;
        }

        /* Vertical 1 */
        if(
            lcd_drawVerticalLine(x0, y0 + 1, y1 - 1, red, green, blue) < LCD_OK
        ) {
            return LCD_FAIL;
        }

        /* Vertical 2 */
        if(
            lcd_drawVerticalLine(x1, y0 + 1, y1 - 1, red, green, blue) < LCD_OK
        ) {
            return LCD_FAIL;
        }
    }

    /* draw other shapes */
    if(lcd_drawFilledRectangle(x0, y0, x1, y1, red, green, blue) < LCD_OK) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_drawFilledRectangle(
    unsigned short int x0,
    unsigned short int y0,
    unsigned short int x1,
    unsigned short int y1,
    unsigned char      red,
    unsigned char      green,
    unsigned char      blue
) {
    unsigned short int ix, iy; /* variables for iteration */
    unsigned short int width;
    unsigned short int height;
    unsigned char buffer[3];      /* three bytes */
    unsigned char buffer_temp[3]; /* three bytes */
    unsigned char buffer_required_size = 0;

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* prepare buffer */
    switch(lcd_settings->interface_pixel_format) {
        /* case LCD_PIXEL_FORMAT_444 is not supported for this function */

        case LCD_PIXEL_FORMAT_565:
            buffer[0] = (red & 0xF8)  |  ((green >> 5) & 0x07);
            buffer[1] = ((green << 3) & 0xE0)  |  ((blue >> 3) & 0x1F);

            buffer_required_size = 2; /* two bytes required */
            break;

        case LCD_PIXEL_FORMAT_666:
            buffer[0] = red;
            buffer[1] = green;
            buffer[2] = blue;

            buffer_required_size = 3; /* three bytes required */
            break;

        /* unknown interface pixel format */
        default:
            return LCD_FAIL;
    }

    if(lcd_setWindowPosition(x0, y0, x1, y1) < LCD_OK) {
        return LCD_FAIL;
    }

    if(lcd_activateMemoryWrite() < LCD_OK) {
        return LCD_FAIL;
    }

    /* if(x0 > x1) or if(y0 > y1) then
     * lcd_setWindowPosition will return LCD_FAIL
     */
    width  = x1 - x0 + 1;
    height = y1 - y0 + 1;

    /* write data */
    for(iy = 0; iy < height; iy++) for(ix = 0; ix < width; ix++) {
        /* copy buffer to buffer_temp */
        memcpy(buffer_temp, buffer, buffer_required_size);

        /* send a copy of the buffer; copy is overwritten with incoming data */
        if(lcd_writeData(buffer_temp, buffer_required_size) < LCD_OK) {
            return LCD_FAIL;
        }
    }

    return LCD_OK;
}

lcd_status_t lcd_clearScreen(
    unsigned char red,
    unsigned char green,
    unsigned char blue
) {
    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* clear screen */
    if(
        lcd_drawFilledRectangle(
            0, 0, lcd_settings->width - 1, lcd_settings->height - 1,
            red, green, blue
        ) < LCD_OK
    ) {
        return LCD_FAIL;
    }

    return LCD_OK;
}

lcd_status_t lcd_framebuffer_send(
    unsigned char * buffer,
    const    size_t length_buffer,
    const    size_t length_chunk
) {
    unsigned int i;
    unsigned int chunk_amount;
    unsigned int remainder;

    /* check inputs */
    if(buffer == NULL) {
        return LCD_FAIL;
    }
    if(length_buffer <= 0) {
        return LCD_FAIL;
    }
    if(length_chunk <= 0) {
        return LCD_FAIL;
    }

    chunk_amount = length_buffer / length_chunk; /* integer division */
    remainder    = length_buffer % length_chunk;

    /* send chunks */
    for(i = 0; i < chunk_amount; i++) {
        if(
            lcd_writeData(
                buffer + i * length_chunk,
                length_chunk
            ) < LCD_OK
        ) {
            return LCD_FAIL;
        }
    }

    /* send the remainder, if any */
    if(remainder > 0) {
        if(
            lcd_writeData(
                buffer + chunk_amount * length_chunk,
                remainder
            ) < LCD_OK
        ) {
            return LCD_FAIL;
        }
    }

    return LCD_OK;
}

lcd_status_t set_background_color(unsigned char red, unsigned char green, unsigned char blue) {
    unsigned short int width, height;
    unsigned char pixel_size;
    unsigned char *framebuffer;
    size_t length_framebuffer;
    size_t address;
    unsigned short int x, y;

    /* Prevent a bug of NULL pointer. 'lcd_settings' is required. */
    if(lcd_settings == NULL) {
        return LCD_FAIL;
    }

    /* get width and height */
    width  = lcd_settings->width;
    height = lcd_settings->height;

    /* get pixel size */
    switch(lcd_settings->interface_pixel_format) {
        /* case LCD_PIXEL_FORMAT_444 is not supported for this function */

        case LCD_PIXEL_FORMAT_565:
            pixel_size = 2; /* two bytes required */
            break;

        case LCD_PIXEL_FORMAT_666:
            pixel_size = 3; /* three bytes required */
            break;

        /* unknown interface pixel format */
        default:
            return LCD_FAIL;
    }
    /* allocate memory */
    length_framebuffer = width * height * pixel_size;
    framebuffer = malloc(length_framebuffer);

    /* if out of RAM memory */
    if(framebuffer == NULL) {
        return LCD_FAIL;
    }

    /* set window position */
    if(lcd_setWindowPosition(0, 0, width - 1, height - 1) < LCD_OK) {
        free(framebuffer);
        return LCD_FAIL;
    }

    /* activate memory write */
    if(lcd_activateMemoryWrite() < LCD_OK) {
        free(framebuffer);
        return LCD_FAIL;
    }

    /* Fill framebuffer with the specified color */
    for(x = 0; x < width; x++) {
        for(y = 0; y < height; y++) {
            /* calculate pixel address */
            address = (y * width + x) * pixel_size;

            /* write pixel to framebuffer */
            switch(lcd_settings->interface_pixel_format) {
                case LCD_PIXEL_FORMAT_565:
                    framebuffer[address] = (red & 0xF8) | ((green >> 5) & 0x07);
                    framebuffer[address + 1] = ((green << 3) & 0xE0) | ((blue >> 3) & 0x1F);
                    break;

                case LCD_PIXEL_FORMAT_666:
                    framebuffer[address]     = red;
                    framebuffer[address + 1] = green;
                    framebuffer[address + 2] = blue;
                    break;

                /* unknown interface pixel format */
                default:
                    free(framebuffer);
                    return LCD_FAIL;
            }
        }
    }

    /* send framebuffer */
    if(lcd_framebuffer_send(framebuffer, length_framebuffer, WPI_SPI_BUFFER_SIZE) < LCD_OK) {
        free(framebuffer);
        return LCD_FAIL;
    }

    /* free memory */
    free(framebuffer);

    return LCD_OK;
}

/* fail checking is optional but very useful */
void fail_handler() {
    printf("[!] FAILURE OCCURRED WHILE PROCESSING [!]\n");
    exit(EXIT_FAILURE);
}

void lcd_delay(unsigned long int milliseconds) {
    delay(milliseconds);
}

void lcd_digitalWrite(unsigned short int pin, unsigned char value) {
    digitalWrite(pin, value);
}

void lcd_spiWrite(unsigned char* buffer, size_t length) {
    /* ALREADY TESTED INSIDE THE LIBRARY:
     * 1. if(buffer == NULL)
     * 2. if(length <= 0)
     * You do not need to test this.
     */
    wiringPiSPIDataRW(WPI_SPI_CHANNEL, buffer, length);
}

lcd_ptr_t setup(){
    /* Pointer for display module related settings. */
    lcd_ptr_t my_lcd_settings = NULL;

    /* Initialize the Wiring Pi library */
    wiringPiSetup();

    /* Initialize SPI interface; http://wiringpi.com/reference/spi-library/
     *
     * Based on Timing Chart for 4-line SPI. (pdf v1.4 p36)
     *
     * Serial Clock Cycle (Write) is 66 [ns].
     *
     * 1 [s] / 66 [ns] = 1 / 0.000000066 = 15151515.151515152 [Hz]
     *
     * Based on this information, I conclude that the maximum frequency
     * is 15 MHz = 15 000 000 Hz.
     */
    if(wiringPiSPISetup(WPI_SPI_CHANNEL, 15000000) == -1) {
        fail_handler();
    }
    /* PIN_RESET HAVE TO be stable when the display module is powered on.
     * If not, perform a hardware reset before communicating with the display
     * driver. If you do not do this, the driver will not work properly!
     * The software reset is not suitable.
     */

    /* You HAVE TO manually set the correct pins as outputs! */
    pinMode(PIN_COMMUNICATION_MODE, OUTPUT);
    pinMode(PIN_RESET,              OUTPUT);
    pinMode(PIN_CHIP_SELECT,        OUTPUT);

    /* In order to communicate with the display module, you HAVE TO select
     * the correct one using the Chip Select line.
     * When using multiple display modules, you have to manually manage their
     * selection through the Chip Select lines.
     *
     * ST7735S driver is selected with LOW state. (pdf v1.4 p23)
     */

    /* select display module; activation of communication */
    digitalWrite(PIN_CHIP_SELECT, LOW);

    /* create settings for a specific display module */
    my_lcd_settings = lcd_createSettings(
        128, /* width */
        160, /* height */
        0,   /* width_offset */
        0,   /* height_offset */
        PIN_COMMUNICATION_MODE, /* Wiring Pi pin numbering */
        PIN_RESET
    );

    /* lcd_createSettings will return NULL if memory runs out */
    if(my_lcd_settings == NULL) {
        fail_handler();
    }

    /* This is where you set the settings as active for the library.
     * The library will use them to handle the driver.
     * You can swap it at any time for a different display module.
     * If set to NULL, library functions will return LCD_FAIL.
     */
    lcd_setSettingsActive(my_lcd_settings);

    /* Display initialization. It HAVE TO be done for each display separately.
     * The library will do this for the appropriate display module,
     * based on the active settings.
     */
    if(lcd_initialize() < LCD_OK) {
        fail_handler();
    }

    /* To start drawing, you HAVE TO:
     * step 1: turn off sleep mode
     * step 2: set Memory Access Control  - required by lcd_createSettings()
     * step 3: set Interface Pixel Format - required by lcd_createSettings()
     * step 4: turn on the display
     * After that, you can draw.
     *
     * It is best to make the optional settings between steps 1 and 4.
     */

    /* turn off sleep mode; required to draw */
    if(lcd_setSleepMode(LCD_SLEEP_OUT) < LCD_OK) {
        fail_handler();
    }

    /* set Memory Access Control; refresh - required by lcd_createSettings() */
    if(lcd_setMemoryAccessControl(LCD_MADCTL_DEFAULT) < LCD_OK) {
        fail_handler();
    }

    /* set Interface Pixel Format; refresh - required by lcd_createSettings() */
    if(lcd_setInterfacePixelFormat(LCD_PIXEL_FORMAT_666) < LCD_OK) {
        fail_handler();
    }

    /* set Predefined Gamma; optional reset */
    if(lcd_setGammaPredefined(LCD_GAMMA_PREDEFINED_3) < LCD_OK) {
        fail_handler();
    }

    /* set Display Inversion; optional reset */
    if(lcd_setDisplayInversion(LCD_INVERSION_OFF) < LCD_OK) {
        fail_handler();
    }

    /* set Tearing Effect Line; optional reset */
    if(lcd_setTearingEffectLine(LCD_TEARING_OFF) < LCD_OK) {
        fail_handler();
    }

    /* turn on the display; required to draw */
    if(lcd_setDisplayMode(LCD_DISPLAY_ON) < LCD_OK) {
        fail_handler();
    }

    return my_lcd_settings;
}