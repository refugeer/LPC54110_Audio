#include <rtthread.h>
#include "board.h"

#define LED_NUM 3

struct led_ctrl
{
	uint32_t num;
	uint32_t port;
};

struct lpc_led
{
	/* inherit from rt_device */
	struct rt_device parent;
	struct led_ctrl ctrl[LED_NUM];
};

static struct lpc_led led;

static rt_err_t rt_led_init(rt_device_t dev)
{
	/*led2 Blue:P0.31 ,led1 Green:P0.30 ,led0 Red:P0_29  P38,P32*/
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 29, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 10, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1,  9, (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));
	
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 29);
  Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, false);
	
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 10);
  Chip_GPIO_SetPinState(LPC_GPIO, 1, 10, true);

  Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 9);
  Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
	
	led.ctrl[0].num = 29;
	led.ctrl[0].port = 0;
	
	led.ctrl[1].num = 10;
	led.ctrl[1].port = 1;
	
	led.ctrl[2].num =  9;
	led.ctrl[2].port = 1;

	return RT_EOK;
	
}

static rt_err_t rt_led_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_led_close(rt_device_t dev)
{
	return RT_EOK;
}


static rt_size_t rt_led_read(rt_device_t dev, rt_off_t pos, void *buffer,
                             rt_size_t size)
{
	rt_ubase_t index = 0;
	rt_ubase_t nr = size;
	rt_uint8_t *value = buffer;

	RT_ASSERT(dev == &led.parent);
	RT_ASSERT((pos + size) <= LED_NUM);

	for (index = 0; index < nr; index++) {
		//if ((LPC_GPIO->PIN[led.ctrl[pos + index].port]) & 1 << led.ctrl[pos + index].num)
		if ((LPC_GPIO->B[0][led.ctrl[pos + index].num]))	 {
			*value = 0;
		}
		else {
			*value = 1;
		}
		value++;
	}
	return index;
}



static rt_size_t rt_led_write(rt_device_t dev, rt_off_t pos,
                              const void *buffer, rt_size_t size)
{
    rt_ubase_t index = 0;
    rt_ubase_t nw = size;
    const rt_uint8_t *value = buffer;

    RT_ASSERT(dev == &led.parent);
    RT_ASSERT((pos + size) <= LED_NUM);
    for (index = 0; index < nw; index++)
    {
			if (*value > 0)
			{
				 //LPC_GPIO->CLR[led.ctrl[pos + index].port] |= (1 << led.ctrl[pos + index].num);
				 //LPC_GPIO->CLR[0] |= (1 << led.ctrl[pos + index].num);
				Chip_GPIO_SetPinState(LPC_GPIO, led.ctrl[pos + index].port, led.ctrl[pos + index].num, 0UL);
			}
			else
			{
				//LPC_GPIO->SET[led.ctrl[pos + index].port] |= (1 << led.ctrl[pos + index].num);
				//LPC_GPIO->SET[0] |= (1 << led.ctrl[pos + index].num);
				Chip_GPIO_SetPinState(LPC_GPIO, led.ctrl[pos + index].port, led.ctrl[pos + index].num, 1UL);
			}
		}
    
		return index;
}


static rt_err_t rt_led_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    return RT_EOK;
}

int rt_led_hw_init(void)
{
    led.parent.type         = RT_Device_Class_Char;
    led.parent.rx_indicate  = RT_NULL;
    led.parent.tx_complete  = RT_NULL;
    led.parent.init         = rt_led_init;
    led.parent.open         = rt_led_open;
    led.parent.close        = rt_led_close;
    led.parent.read         = rt_led_read;
    led.parent.write        = rt_led_write;
    led.parent.control      = rt_led_control;
    led.parent.user_data    = RT_NULL;

    /* register a character device */
    rt_device_register(&led.parent, "led", RT_DEVICE_FLAG_RDWR);
    /* init led device */
    rt_led_init(&led.parent);
    return 0;
}



void Led_Control(rt_uint32_t Set_led, rt_uint32_t value)
{
		/* set led status */
		switch (value)
		{
		case 0:
				/* Light off */
				//PC_GPIO->B[0][led.ctrl[Set_led].num] = 1UL;
				Chip_GPIO_SetPinState(LPC_GPIO, led.ctrl[Set_led].port, led.ctrl[Set_led].num, 1UL);
				break;
		case 1:
				/* Lights on */
				//LPC_GPIO->B[0][led.ctrl[Set_led].num] = 0UL;
				Chip_GPIO_SetPinState(LPC_GPIO, led.ctrl[Set_led].port, led.ctrl[Set_led].num, 0UL);
				break;
		default:
				break;
		}
}



INIT_DEVICE_EXPORT(rt_led_hw_init);
#ifdef RT_USING_FINSH
#include <finsh.h>
void led_test(rt_uint32_t led_num, rt_uint32_t value)
{
    rt_uint8_t led_value = value;
    rt_led_write(&led.parent, led_num, &led_value, 1);
}
FINSH_FUNCTION_EXPORT(led_test, e.g: led_test(0, 100).)
#endif




