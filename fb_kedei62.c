/*
 * 2019 Tong Zhang <ztong@vt.edu>
 * FBTFT Driver for KeDei 6.2 Display
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <video/mipi_display.h>
#include <linux/gpio.h>

#include "fbtft.h"

#define DRVNAME		"kedei62"
#define WIDTH		480
#define HEIGHT		320
#define FPS		1


static uint8_t lcd_rotations[4] = {
	0b11101010,	//   0
	0b01001010,	//  90
	0b00101010,	// 180
	0b00001010	// 270
};

static uint16_t lcd_h;
static uint16_t lcd_w;

static int write(struct fbtft_par *par, void *buf, size_t len)
{
    gpio_set_value(8, 1);
    fbtft_write_spi(par, buf, len);
    gpio_set_value(8, 0);
    return 0;
}

static void lcd_cmd(struct fbtft_par* par, uint8_t cmd)
{
    uint8_t b1[3];
    b1[0] = 0x11;
	b1[1] = 0x00;
	b1[2] = cmd;
	write(par, &b1[0], sizeof(b1));
}

static void lcd_data(struct fbtft_par* par, uint8_t dat)
{
    uint8_t b1[3];
    b1[0] = 0x15;
    b1[1] = 0x00;
    b1[2] = dat;
	write(par, &b1[0], sizeof(b1));
}

static void lcd_setrotation(struct fbtft_par* par, uint8_t m)
{
	lcd_cmd(par,0x36);
    lcd_data(par,lcd_rotations[m]);
	if (m&1) {
		lcd_h = WIDTH;
		lcd_w = HEIGHT;
	} else {
		lcd_h = HEIGHT;
		lcd_w = WIDTH;
	}
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys,
			 int xe, int ye)
{
    
	lcd_cmd(par,0x2A);
	    lcd_data(par,xs>>8);
        lcd_data(par,xs&0xFF);
	    lcd_data(par,xe>>8);
        lcd_data(par,xe&0xFF);
	lcd_cmd(par,0x2B);
	    lcd_data(par,ys>>8);
        lcd_data(par,ys&0xFF);
	    lcd_data(par,ye>>8);
        lcd_data(par,ye&0xFF);
	lcd_cmd(par,0x2C);
}

static int write_vmem(struct fbtft_par *par, size_t offset, size_t len)
{
	u8 *vmem8 = (u8 *)(par->info->screen_buffer + offset);
	uint8_t b1[3];
    b1[0] = 0x15;
    int i;
    for (i=0;i<len;i+=2)
    {
        b1[1] = vmem8[i+1];
        b1[2] = vmem8[i];
        write(par, b1, sizeof(b1));
    }
	return 0;
}

static void reset(struct fbtft_par *par)
{
	uint8_t buff[4];
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x00;
    buff[3] = 0x00;
    write(par, &buff[0], sizeof(buff));
	mdelay(50);

	buff[0] = 0x00;
	buff[1] = 0x00;
	buff[2] = 0x00;
	buff[3] = 0x00;
	write(par, &buff[0], sizeof(buff));
	mdelay(100);
    
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x00;
	buff[3] = 0x00;
	write(par, &buff[0], sizeof(buff));
	mdelay(50);
}


static int init_display(struct fbtft_par *par)
{
	reset(par);

    lcd_cmd(par,0x00);
    mdelay(10);
    lcd_cmd(par,0xFF); lcd_cmd(par,0xFF);
    mdelay(10);
    lcd_cmd(par,0xFF); lcd_cmd(par,0xFF); lcd_cmd(par,0xFF); lcd_cmd(par,0xFF);
    mdelay(15);
    lcd_cmd(par,0x11);
    mdelay(150);

    lcd_cmd(par,0xB0); lcd_data(par,0x00);
    lcd_cmd(par,0xB3); lcd_data(par,0x02); lcd_data(par,0x00); lcd_data(par,0x00); lcd_data(par,0x00);
    lcd_cmd(par,0xB9); lcd_data(par,0x01); lcd_data(par,0x00); lcd_data(par,0x0F); lcd_data(par,0x0F);
    lcd_cmd(par,0xC0); lcd_data(par,0x13); lcd_data(par,0x3B); lcd_data(par,0x00); lcd_data(par,0x02);
                   lcd_data(par,0x00); lcd_data(par,0x01); lcd_data(par,0x00); lcd_data(par,0x43);
    lcd_cmd(par,0xC1); lcd_data(par,0x08); lcd_data(par,0x0F); lcd_data(par,0x08); lcd_data(par,0x08);
    lcd_cmd(par,0xC4); lcd_data(par,0x11); lcd_data(par,0x07); lcd_data(par,0x03); lcd_data(par,0x04);
    lcd_cmd(par,0xC6); lcd_data(par,0x00);
    lcd_cmd(par,0xC8); lcd_data(par,0x03); lcd_data(par,0x03); lcd_data(par,0x13); lcd_data(par,0x5C);
                   lcd_data(par,0x03); lcd_data(par,0x07); lcd_data(par,0x14); lcd_data(par,0x08);
                   lcd_data(par,0x00); lcd_data(par,0x21); lcd_data(par,0x08); lcd_data(par,0x14);
                   lcd_data(par,0x07); lcd_data(par,0x53); lcd_data(par,0x0C); lcd_data(par,0x13);
                   lcd_data(par,0x03); lcd_data(par,0x03); lcd_data(par,0x21); lcd_data(par,0x00);
    lcd_cmd(par,0x35); lcd_data(par,0x00);
    lcd_cmd(par,0x36); lcd_data(par,0x60);
    lcd_cmd(par,0x3A); lcd_data(par,0x55);
    lcd_cmd(par,0x44); lcd_data(par,0x00); lcd_data(par,0x01);
    lcd_cmd(par,0xD0); lcd_data(par,0x07); lcd_data(par,0x07); lcd_data(par,0x1D); lcd_data(par,0x03);
    lcd_cmd(par,0xD1); lcd_data(par,0x03); lcd_data(par,0x30); lcd_data(par,0x10);
    lcd_cmd(par,0xD2); lcd_data(par,0x03); lcd_data(par,0x14); lcd_data(par,0x04);
    lcd_cmd(par,0x29);

    mdelay(30);

    lcd_cmd(par,0x2A); lcd_data(par,0x00); lcd_data(par,0x00); lcd_data(par,0x01); lcd_data(par,0x3F);
    lcd_cmd(par,0x2B); lcd_data(par,0x00); lcd_data(par,0x00); lcd_data(par,0x01); lcd_data(par,0xE0);
    lcd_cmd(par,0xB4); lcd_data(par,0x00);

    lcd_cmd(par,0x2C);

    mdelay(10);
    lcd_setrotation(par,3);

    //lcd_fill(par,0x0); //black out the screen.
    printk("kedei62 initialized\n");
    return 0;
}

static int request_gpios(struct fbtft_par *par)
{
    return 0;
}

static int verify_gpios(struct fbtft_par *par)
{
    return 0;
}

static struct fbtft_display display = {
	.regwidth = 8,
	.width = HEIGHT,
	.height = WIDTH,
	.fps = FPS,
	.fbtftops = {
        .write = write,
        .write_vmem = write_vmem,
		.set_addr_win = set_addr_win,
        .reset = reset,
		.init_display = init_display,
        .request_gpios = request_gpios,
        .verify_gpios = verify_gpios,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "kedei62", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:kedei62");
MODULE_ALIAS("platform:kedei62");

MODULE_DESCRIPTION("FB driver for KeDei6.2 display");
MODULE_AUTHOR("Tong Zhang<ztong@vt.edu>");
MODULE_LICENSE("GPL");

