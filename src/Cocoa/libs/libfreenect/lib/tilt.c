/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "freenect_internal.h"

// The kinect can tilt from +31 to -31 degrees in what looks like 1 degree increments
// The control input looks like 2*desired_degrees
#define MAX_TILT_ANGLE 31
#define MIN_TILT_ANGLE (-31)

#define GRAVITY 9.80665

int freenect_set_tilt_degs(freenect_device *dev, double angle)
{
	int ret;
	uint8_t empty[0x1];

	angle = (angle<MIN_TILT_ANGLE) ? MIN_TILT_ANGLE : ((angle>MAX_TILT_ANGLE) ? MAX_TILT_ANGLE : angle);
	angle = angle * 2;

	ret = fnusb_control(&dev->usb_motor, 0x40, 0x31, (uint16_t)angle, 0x0, empty, 0x0);
	return ret;
}

int freenect_set_led(freenect_device *dev, freenect_led_options option)
{
	int ret;
	uint8_t empty[0x1];
	ret = fnusb_control(&dev->usb_motor, 0x40, 0x06, (uint16_t)option, 0x0, empty, 0x0);
	return ret;
}

int freenect_get_raw_accel(freenect_device *dev, int16_t* x, int16_t* y, int16_t* z)
{
	freenect_context *ctx = dev->parent;
	uint8_t buf[10];
	uint16_t ux, uy, uz;
	int ret = fnusb_control(&dev->usb_motor, 0xC0, 0x32, 0x0, 0x0, buf, 10);
	if (ret != 10) {
		FN_ERROR("Error in accelerometer reading, libusb_control_transfer returned %d\n", ret);
		return ret < 0 ? ret : -1;
	}

	ux = ((uint16_t)buf[2] << 8) | buf[3];
	uy = ((uint16_t)buf[4] << 8) | buf[5];
	uz = ((uint16_t)buf[6] << 8) | buf[7];
	*x = (int16_t)ux;
	*y = (int16_t)uy;
	*z = (int16_t)uz;

	return ret;
}

int freenect_get_mks_accel(freenect_device *dev, double* x, double* y, double* z)
{
	//the documentation for the accelerometer (http://www.kionix.com/Product%20Sheets/KXSD9%20Product%20Brief.pdf)
	//states there are 819 counts/g
	int16_t ix, iy, iz;
	int ret = freenect_get_raw_accel(dev,&ix,&iy,&iz);

	if (ret >= 0) {
		*x = (double)ix/FREENECT_COUNTS_PER_G*GRAVITY;
		*y = (double)iy/FREENECT_COUNTS_PER_G*GRAVITY;
		*z = (double)iz/FREENECT_COUNTS_PER_G*GRAVITY;
	}

	return ret;
}
