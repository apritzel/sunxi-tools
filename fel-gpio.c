// SPDX-License-Identifier:   GPL-2.0+
/*
 * (C) Copyright 2025 Andre Przywara
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fel_lib.h"
#include "soc_info.h"

uint32_t fel_readl(feldev_handle *dev, uint32_t addr);
void fel_writel(feldev_handle *dev, uint32_t addr, uint32_t val);

static uint32_t get_port_base(const soc_info_t *soc_info, int port_num)
{
	uint32_t port_base = soc_info->gpio_base;

	if (soc_info->flags & GPIO_NCAT2)
		port_base += port_num * 0x30;
	else
		port_base += port_num * 0x24;

	return port_base;
}

static void gpio_set_cfgpin(feldev_handle *dev, int port_num, int pin_num,
			    int val)
{
	uint32_t cfg_reg = get_port_base(dev->soc_info, port_num);
	uint32_t pin_idx = pin_num % 8;
	uint32_t reg;

	cfg_reg += 4 * (pin_num / 8);

	reg = fel_readl(dev, cfg_reg);
	reg &= ~(0xf << (pin_idx * 4));
	reg |= val << (pin_idx * 4);
	fel_writel(dev, cfg_reg, reg);
}

enum gpio_action {
	GPIO_NULL,
	GPIO_SET,
	GPIO_CLEAR,
	GPIO_TOGGLE
};

static void gpio_set_pin_level(feldev_handle *dev, int port_num, int pin_num,
			       enum gpio_action action)
{
	uint32_t data_reg = get_port_base(dev->soc_info, port_num) + 0x10;
	uint32_t reg;

	reg = fel_readl(dev, data_reg);
	switch (action) {
	case GPIO_NULL:
		break;
	case GPIO_SET:
		reg |= 1U << pin_num;
		break;
	case GPIO_CLEAR:
		reg &= ~(1U << pin_num);
		break;
	case GPIO_TOGGLE:
		reg ^= 1U << pin_num;
		break;
	}
	fel_writel(dev, data_reg, reg);
}

static bool gpio_get_pin_level(feldev_handle *dev, int port_num, int pin_num)
{
	uint32_t data_reg = get_port_base(dev->soc_info, port_num) + 0x10;
	uint32_t reg = fel_readl(dev, data_reg);

	return !!(reg & 1U << pin_num);
}

static int parse_pin(const char *str)
{
	int port, pin;
	char *endptr;

	if (*str == 'p' || *str == 'P')
		str++;
	port = *str - 'a';
	if (port < 0)
		port = *str - 'A';
	if (port < 0 || port >= 16)
		return -1;
	pin = strtoul(str + 1, &endptr, 10);
	if (endptr <= str + 1 || *endptr)
		return -1;

	return port * 32 + pin;
}

int aw_fel_handle_gpio(feldev_handle *dev, const char *verb,
		       const char *pin_str)
{
	int pin = parse_pin(pin_str);
	enum gpio_action action;

	if (pin < 0 || pin >= ('K' - 'A') * 32)
		return -2;

	if (!strcmp(verb, "input")) {
		gpio_set_cfgpin(dev, pin / 32, pin % 32, 0);
		return gpio_get_pin_level(dev, pin / 32, pin % 32) + 1;
	}

	if (!strcmp(verb, "set"))
		action = GPIO_SET;
	else if (!strcmp(verb, "clear"))
		action = GPIO_CLEAR;
	else if (!strcmp(verb, "toggle"))
		action = GPIO_TOGGLE;
	else
		return -1;

	gpio_set_cfgpin(dev, pin / 32, pin % 32, 1);
	gpio_set_pin_level(dev, pin / 32, pin % 32, action);

	return 0;
}

void aw_fel_gpio_help(void)
{
        printf("        gpio input PXyy                 print GPIO pin level\n"
               "        gpio set PXyy                   set GPIO pin to HIGH\n"
               "        gpio clear PXyy                 set GPIO pin to LOW\n"
               "        gpio toggle PXyy                change GPIO pin level\n"
	);
}
