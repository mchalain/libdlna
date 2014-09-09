/*
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
 *
 * This file is part of uplaymusic.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __SOUND_MODULE_H__
#define __SOUND_MODULE_H__

#define SOUND	0x0002

struct sound_module
{
	int (*open)(int channels, int encoding, long rate);
	int (*write)(char *buffer, ssize_t buffsize);
	int (*close)(void);
	int (*get_volume)(float *);
	int (*set_volume)(float);
	int (*get_mute)(int *);
	int (*set_mute)(int);
};

extern struct sound_module *
sound_module_set(const char *shortname);
extern struct sound_module *
sound_module_get();

#endif
