/*
 * pwcview - application to view video, create jpeg snapshots and alter
 * settings of a webcam controlled by the pwc driver
 *
 * Copyright (C) 2006-2007 Raaf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <jpeglib.h>
#ifndef NOGUI
#include <SDL.h>
#endif
#include "pwc-ioctl.h"
#include "pixels.h"

#include <libv4l1.h>
#include <linux/videodev.h>

const char *motioncmd;
long cmdinterval = 60;
int fullscreen = 0;
int motionrecord = 0;
int motionfs   = 0;
int motionbeep = 0;
int showmotion = 0;
int showtime   = 0;
int recmargin = 15;
uint32_t threshold = 1;
uint32_t sensitivity = 1000;
struct video_window vw;

#define SCALE_NONE 0
#define SCALE_DOUBLE 1
#define SCALE_FULL 2
int scale = SCALE_FULL;

#ifndef NOGUI
SDL_Surface *screen;

int default_handler(int fd, int dir, char *buf)
{
	if(dir != 0)
		return -1;
	
	snprintf(buf,80,"pwcview");
	return 0;
}

int framerate_handler(int fd, int dir, char *buf)
{
	int fps;
	struct video_window vw;

	if(v4l1_ioctl(fd, VIDIOCGWIN,&vw) == -1) {
		perror("Failed to get current framerate");
		return -1;
	}

	fps = vw.flags  >> PWC_FPS_SHIFT;
	
	if((dir == -1 && fps >= 9) ||(dir == 1 && fps <= 25)) {
		fps += dir == -1 ? -5 : 5;
		vw.flags = fps << PWC_FPS_SHIFT;
		if(v4l1_ioctl(fd, VIDIOCSWIN,&vw) == -1)
			fprintf(stderr,"Failed to set framerate to %d fps: %s\n",fps,strerror(errno));
		if(v4l1_ioctl(fd, VIDIOCGWIN,&vw) == -1) {
			perror("Failed to get new framerate");
			return -1;
		}
		fps = vw.flags  >> PWC_FPS_SHIFT;
	}
	snprintf(buf,80,"framerate: %d fps",fps);
	return 0;
}

int compression_handler(int fd, int dir, char *buf)
{
	int qual;

	if(v4l1_ioctl(fd, VIDIOCPWCGCQUAL,&qual) == -1) {
		perror("Failed to get current compression");
		return -1;
	}

	if((dir == -1 && qual > 0) || (dir == 1 && qual < 3)) {
		qual += dir == -1 ? -1 : 1;
		if(v4l1_ioctl(fd, VIDIOCPWCSCQUAL,&qual) == -1)
			perror("Failed to set compression");
		if(v4l1_ioctl(fd, VIDIOCPWCGCQUAL,&qual) == -1) {
			perror("Failed to get new compression");
			return -1;
		}
	}
	snprintf(buf,80,"compression: %d",qual);
	return 0;
}

int brightness_handler(int fd, int dir, char *buf)
{
	struct video_picture pict;
	
	if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
		perror("Failed to get current brightness");
		return -1;
	}

	if((dir == -1) || (dir == 1)) {
		pict.brightness += dir == -1 ? -512 : 512;
		if(v4l1_ioctl(fd, VIDIOCSPICT,&pict) == -1)
			perror("Failed to set brightness");
		if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
			perror("Failed to get new brightness");
			return -1;
		}
	}
	snprintf(buf,80,"brightness: %d",pict.brightness >> 9);
	return 0;
}

int contrast_handler(int fd, int dir, char *buf)
{
	struct video_picture pict;
	
	if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
		perror("Failed to get current contrast");
		return -1;
	}

	if((dir == -1) || (dir == 1)) {
		pict.contrast += dir == -1 ? -1024 : 1024;
		if(v4l1_ioctl(fd, VIDIOCSPICT,&pict) == -1)
			perror("Failed to set contrast");
		if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
			perror("Failed to get new contrast");
			return -1;
		}
	}
	snprintf(buf,80,"contrast: %d",pict.contrast >> 10);
	return 0;
}

int saturation_handler(int fd, int dir, char *buf)
{
	struct video_picture pict;
	
	if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
		perror("Failed to get current saturation");
		return -1;
	}

	if((dir == -1) || (dir == 1)) {
		pict.colour += dir == -1 ? -327 : 327;
		if(v4l1_ioctl(fd, VIDIOCSPICT,&pict) == -1)
			perror("Failed to set saturation");
		if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
			perror("Failed to get new saturation");
			return -1;
		}
	}
	snprintf(buf,80,"saturation: %d",(pict.colour - 32768) / 327);
	return 0;
}

int gamma_handler(int fd, int dir, char *buf)
{
	struct video_picture pict;
	
	if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
		perror("Failed to get current gamma");
		return -1;
	}

	if((dir == -1) ||(dir == 1)) {
		pict.whiteness += dir == -1 ? -2048 : 2048;
		if(v4l1_ioctl(fd, VIDIOCSPICT,&pict) == -1)
			perror("Failed to set gamma");
		if(v4l1_ioctl(fd, VIDIOCGPICT,&pict) == -1) {
			perror("Failed to get new gamma");
			return -1;
		}
	}
	snprintf(buf,80,"gamma: %d",pict.whiteness >> 11);
	return 0;
}

int agc_handler(int fd, int dir, char *buf)
{
	static u_int16_t agc = 32768;
 	static int agcmode = 1;
	int val;
	
	if(dir == 2) {
		if(++agcmode == 2)
			agcmode = 0;
	}
	else if(dir == -1 && agcmode == 0)
		agc -= 1024;
	else if(dir == 1 && agcmode == 0)
		agc += 1024;
	
	if(agcmode == 1) {
		val = -1;
		snprintf(buf,80,"gain control: auto");
	}
	else {
		val = agc;
		snprintf(buf,80,"gain control: %d",agc >> 10);
	}
	
	v4l1_ioctl(fd, VIDIOCPWCSAGC,&val);
	return 0;
}

int shutter_handler(int fd, int dir, char *buf)
{
	static u_int16_t shutter = 32768;
	static int shuttermode = 1;
	int val;
	
	if(dir == 2) {
		if(++shuttermode == 2)
			shuttermode = 0;
	}
	else if(dir == -1 && shuttermode == 0)
		shutter -= 256;
	else if(dir == 1 && shuttermode == 0)
		shutter += 256;
	
	if(shuttermode == 1) {
		val = -1;
		snprintf(buf,80,"shutter speed: auto");
	}
	else {
		val = shutter;
		snprintf(buf,80,"shutter speed: %d",shutter >> 8);
	}
	v4l1_ioctl(fd, VIDIOCPWCSSHUTTER,&val);
	return 0;
}

int whitebalance_handler(int fd, int dir, char *buf)
{
	static int skip = 0;
	struct pwc_whitebalance wb;
	char *names[] = { "indoor", "outdoor", "fluorescent","manual","auto" };
	int *val = NULL;
	
	if(v4l1_ioctl(fd, VIDIOCPWCGAWB,&wb) == -1) {
		perror("Failed to get white balance");
		return -1;
	}
		
	if(dir == 2 && !skip) {
		if(--wb.mode < PWC_WB_INDOOR)
			wb.mode = PWC_WB_AUTO;
	}
	if(wb.mode == PWC_WB_MANUAL) {
		if(dir == 2) {
			skip = !skip;
			if(skip) {
				wb.manual_red  = wb.read_red;
				wb.manual_blue  = wb.read_blue;
			}
		}
		val = skip ? &wb.manual_red : &wb.manual_blue;
		if(dir == -1)
			*val -= 256;
		else if(dir == 1)
			*val += 256;
	}
	
	if(v4l1_ioctl(fd, VIDIOCPWCSAWB,&wb) == -1)
		perror("Failed to set white balance");
	
	if(v4l1_ioctl(fd, VIDIOCPWCGAWB,&wb) == -1) {
		perror("Failed to get white balance");
		return -1;
	}

	if(wb.mode == PWC_WB_MANUAL)
		snprintf(buf,80,"white balance %s gain: %d",skip ? "red" : "blue",*val >> 8);
	else
		snprintf(buf,80,"white balance: %s",names[wb.mode]);
	return 0;
}

int whitebalancespeed_handler(int fd, int dir, char *buf)
{
	struct pwc_wb_speed speed;

	if(v4l1_ioctl(fd, VIDIOCPWCGAWBSPEED,&speed) == -1) {
		perror("Failed to get current awb speed");
		return -1;
	}

	if((dir == -1) || (dir == 1)) {
		speed.control_speed += dir == -1 ? -2032 : 2032;
		if(v4l1_ioctl(fd, VIDIOCPWCSAWBSPEED,&speed) == -1)
			perror("Failed to set awb speed");
		if(v4l1_ioctl(fd, VIDIOCPWCGAWBSPEED,&speed) == -1) {
			perror("Failed to get new awb speed");
			return -1;
		}
	}
	snprintf(buf,80,"white balance speed: %d",speed.control_speed / 2032);
	return 0;
}

int whitebalancedelay_handler(int fd, int dir, char *buf)
{
	struct pwc_wb_speed speed;

	if(v4l1_ioctl(fd, VIDIOCPWCGAWBSPEED,&speed) == -1) {
		perror("Failed to get current awb delay");
		return -1;
	}

	if((dir == -1) || (dir == 1)) {
		speed.control_delay += dir == -1 ? -1024 : 1024;
		if(v4l1_ioctl(fd, VIDIOCPWCSAWBSPEED,&speed) == -1)
			perror("Failed to set awb delay");
		if(v4l1_ioctl(fd, VIDIOCPWCGAWBSPEED,&speed) == -1) {
			perror("Failed to get new awb delay");
			return -1;
		}
	}
	snprintf(buf,80,"white balance delay: %d",speed.control_delay >> 10);
	return 0;
}

int contour_handler(int fd, int dir,char *buf)
{
	static u_int16_t contour = 32768;
	static int contourmode = 1;
	int val;
	
	if(dir == 2) {
		if(++contourmode == 2)
			contourmode = 0;
	}
	else if(dir == -1 && contourmode == 0)
		contour -= 1024;
	else if(dir == 1 && contourmode == 0)
		contour += 1024;
	
	if(contourmode == 1)
		val = -1;
	else 
		val = contour;
	
	if(v4l1_ioctl(fd, VIDIOCPWCSCONTOUR,&val) == -1)
		perror("Failed to set contour");
	
	if(contourmode == 1)
		snprintf(buf,80,"contour: auto");
	else {
		if(v4l1_ioctl(fd, VIDIOCPWCGCONTOUR,&contour) == -1) {
			perror("Failed to get contour");
			return -1;
		}
		snprintf(buf,80,"contour: %d",contour >> 10);
	}
	return 0;
}

int dynamicnoise_handler(int fd, int dir, char *buf)
{
	int dynnoise;
	 
	if(v4l1_ioctl(fd, VIDIOCPWCGDYNNOISE,&dynnoise) == -1) {
		perror("Failed to get current dynamic noise reduction mode");
		return -1;
	}
	if(dir == 2) {
		if(++dynnoise == 4)
			dynnoise = 0;
		if(v4l1_ioctl(fd, VIDIOCPWCSDYNNOISE,&dynnoise) == -1)
			perror("Failed to set dynamic noise reduction mode");
		 
		if(v4l1_ioctl(fd, VIDIOCPWCGDYNNOISE,&dynnoise) == -1) {
			perror("Failed to get new dynamic noise reduction mode");
			return -1;
		 }
	}
	snprintf(buf,80,"dnr mode: %d",dynnoise);
	return 0;
}

int backlight_handler(int fd, int dir, char *buf)
{
	int backlight;

	if(v4l1_ioctl(fd, VIDIOCPWCGBACKLIGHT,&backlight) == -1) {
		perror("Failed to get backlight mode");
		return -1;
	}
	if(dir == 2) {
		backlight = !backlight;
		if(v4l1_ioctl(fd, VIDIOCPWCSBACKLIGHT,&backlight) == -1)
			perror("Failed to set backlight mode");
		
		if(v4l1_ioctl(fd, VIDIOCPWCGBACKLIGHT,&backlight) == -1) {
			perror("Failed to get new backlight mode");
			return -1;
		}
	}
	snprintf(buf,80,"backlight compensation: %s",backlight ? "on" : "off");
	return 0;
}

int flicker_handler(int fd, int dir, char *buf)
{
	int flicker;

	if(v4l1_ioctl(fd, VIDIOCPWCGFLICKER,&flicker) == -1) {
		perror("Failed to get flicker mode");
		return -1;
	}
	if(dir == 2) {
		flicker = !flicker;
		if(v4l1_ioctl(fd, VIDIOCPWCSFLICKER,&flicker) == -1)
			perror("Failed to set flicker mode");
		
		if(v4l1_ioctl(fd, VIDIOCPWCGFLICKER,&flicker) == -1) {
			perror("Failed to get new flicker mode");
			return -1;
		}
	}
	snprintf(buf,80,"anti flicker mode: %s",flicker ? "on" : "off");
	return 0;
}
#ifdef __FreeBSD__
int colour_handler(int fd, int dir, char *buf)
{
	int colour;

	if(v4l1_ioctl(fd, VIDIOCPWCGCOLOUR,&colour) == -1) {
		perror("Failed to get colour mode");
		return -1;
	}
	if(dir == 2) {
		colour = !colour;
		if(v4l1_ioctl(fd, VIDIOCPWCSCOLOUR,&colour) == -1)
			perror("Failed to set colour mode");
		
		if(v4l1_ioctl(fd, VIDIOCPWCGCOLOUR,&colour) == -1) {
			perror("Failed to get new colour mode");
			return -1;
		}
	}
	snprintf(buf,80,"colour mode: %s",colour ? "color" : "black & white");
	return 0;
}
#endif
int saveuser_handler(int fd, int dir, char *buf)
{
	if(dir == 0) {
		snprintf(buf,80,"save user settings");
	}
	else if(dir == 2) {
		if(v4l1_ioctl(fd, VIDIOCPWCSUSER,NULL) == -1)
			snprintf(buf,80,"Error: %s",strerror(errno));
		else 
			snprintf(buf,80,"User settings saved");
		return 0;
		
	}
	else {
		return -1;
	}
	return 0;
}

int restoreuser_handler(int fd, int dir, char *buf)
{
	if(dir == 0) {
		snprintf(buf,80,"restore user settings");
	}
	else if(dir == 2) {
	  if(v4l1_ioctl(fd, VIDIOCPWCRUSER,NULL) == -1)
			snprintf(buf,80,"Error: %s",strerror(errno));
		else
			snprintf(buf,80,"User settings restored");
	}
	else {
		return -1;
	}
	return 0;
}

int restorefactory_handler(int fd, int dir, char *buf)
{
	if(dir == 0) {
		snprintf(buf,80,"restore factory settings");
	}
	else if(dir == 2) {
	  if(v4l1_ioctl(fd, VIDIOCPWCFACTORY,NULL) == -1)
			snprintf(buf,80,"Error: %s",strerror(errno));
		else
			snprintf(buf,80,"Factory settings restored");
	}
	else {
		return -1;
	}
	return 0;
}

struct pwc_leds led;
int ledon_handler(int fd, int dir, char *buf)
{
	v4l1_ioctl(fd, VIDIOCPWCGLED,&led);
	if((dir == -1) || (dir == 1)) {
		led.led_on += (dir == -1) ? -100 : 100;
		if(led.led_on < 0)
			led.led_on = 0;
		if(v4l1_ioctl(fd, VIDIOCPWCSLED,&led) == -1)
			perror("Failed to set leds");
	}
	snprintf(buf,80,"led on: %d", led.led_on);
	return 0;
}

int ledoff_handler(int fd, int dir, char *buf)
{
	v4l1_ioctl(fd, VIDIOCPWCGLED,&led);
	if((dir == -1) || (dir == 1)) {
		led.led_off += (dir == -1) ? -100 : 100;
		if(led.led_off < 0)
			led.led_off = 0;
		if(v4l1_ioctl(fd, VIDIOCPWCSLED,&led) == -1)
			perror("Failed to set leds");
	}
	snprintf(buf,80,"led off: %d", led.led_off);
	return 0;
}

int sdlflags = SDL_RESIZABLE;
int bpp = 0;
SDL_Rect rect, window;

int scale_handler(int fd, int dir, char *buf)
{
	const char *scalestr[] = { "none", "double", "full"};
	if(dir == 2) {
		if(++scale > SCALE_FULL)
			scale = SCALE_NONE;
		switch(scale) {
		case SCALE_NONE:
			rect.w = vw.width;
			rect.h = vw.height;
			break;
		case SCALE_DOUBLE:
			if(window.w >= (2 * vw.width) && window.h >= (2 * vw.height)) {
				rect.w = 2 * vw.width;
				rect.h = 2 * vw.height;
			}
			else {
				rect.w = vw.width;
				rect.h = vw.height;
			}
			break;
		case SCALE_FULL:
			rect.w = window.w;
			rect.h = window.h;
			break;
		}
		/* Force Redraw */			
		if((screen = SDL_SetVideoMode(window.w, window.h, bpp, sdlflags)) == NULL) {
			fprintf(stderr,"SDL Failed to set videomode: %s\n", SDL_GetError());
			exit(1);
		}
	}
	snprintf(buf,80,"scaling: %s",scalestr[scale]);
	return 0;
}

int motionrecord_handler(int fd, int dir, char *buf)
{
	if(dir == 2) {
		if(isatty(STDOUT_FILENO)) {
			fprintf(stderr,"Error: You must redirect stdout to a file or pipe stdout to another\n"
				       "       command when motion detection recording is enabled\n");
		}
		else
			motionrecord = !motionrecord;
	}
	snprintf(buf,80,"motion record: %s", motionrecord ? "yes" : "no");
	return 0;
}

int cmdinterval_handler(int fd, int dir, char *buf)
{
	if((dir == -1) || (dir == 1)) {
		cmdinterval += dir;
		if(cmdinterval < 0)
			cmdinterval = 0;
	}
	snprintf(buf,80,"motion command interval: %ld",cmdinterval);
	return 0;
}

int sensitivity_handler(int fd, int dir, char *buf)
{
	if((dir == -1) || (dir == 1)) {
		sensitivity += (dir == -1) ? -10 : 10;
		if(sensitivity & 0x80000000)
			sensitivity = 0;
		if(sensitivity > 16320)
			sensitivity = 16320;
	}
	snprintf(buf,80,"motion sensitivity: %u",sensitivity);
	return 0;
}

int threshold_handler(int fd, int dir, char *buf)
{
	if((dir == -1) || (dir == 1)) {
		threshold += (dir == -1) ? -1 : 1;
		if(threshold & 0x80000000)
			threshold = 0;
		if(threshold > 4800)
			threshold = 4800;
	}
	snprintf(buf,80,"motion threshold: %u",threshold);
	return 0;
}

int recmargin_handler(int fd, int dir, char *buf)
{
	if((dir == -1) || (dir == 1)) {
		recmargin += (dir == -1) ? -1 : 1;
		if(recmargin < 0)
			recmargin = 0;
	}
	snprintf(buf,80,"recording margin: %d frame(s)",recmargin);
	return 0;
}
	
int showmotion_handler(int fd, int dir, char *buf)
{
	if(dir == 2)
		showmotion = !showmotion;
	snprintf(buf,80,"motion show : %s", showmotion ? "yes" : "no");
	return 0;
}

int showtime_handler(int fd, int dir, char *buf)
{
	if(dir == 2)
		showtime = !showtime;
	snprintf(buf,80,"time date string: %s", showtime ? "yes" : "no");
	return 0;
}

int fullscreen_handler(int fd, int dir, char *buf)
{
	if(dir == 2)
		motionfs = !motionfs;
	snprintf(buf,80,"motion fullscreen: %s", motionfs ? "yes" : "no");
	return 0;
}

int beep_handler(int fd, int dir, char *buf)
{
	if(dir == 2)
		motionbeep = !motionbeep;
	snprintf(buf,80,"motion beep: %s", motionbeep ? "yes" : "no");
	return 0;
}

int (*handler[])(int fd, int direction, char *buf) = {  /* direction: -1 = down, 0 = init, 1 = up, 2 = special */
	default_handler,
	framerate_handler,
	brightness_handler,
	contrast_handler,
	saturation_handler,
	gamma_handler,
	agc_handler,
	shutter_handler,
	whitebalance_handler,
	whitebalancespeed_handler,
	whitebalancedelay_handler,
	contour_handler,
	dynamicnoise_handler,
	backlight_handler,
	flicker_handler,
#ifdef __FreeBSD__
	colour_handler,
#endif
	compression_handler,
	ledon_handler,
	ledoff_handler,
	saveuser_handler,
	restoreuser_handler,
	restorefactory_handler,
	showtime_handler,
	scale_handler,
	motionrecord_handler,
	fullscreen_handler,
	beep_handler,
	showmotion_handler,
	cmdinterval_handler,
	sensitivity_handler,
	threshold_handler,
	recmargin_handler
};

Uint32 cbtimer(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent(&event);
	return interval;
}
#endif

void sig_chld(int signo)
{
	int stat;
	while(waitpid(-1, &stat, WNOHANG) > 0)
		;
}

void jpeg_init(int width, int height, int quality, struct jpeg_compress_struct *cinfo,
		struct jpeg_error_mgr *jerr, JSAMPIMAGE jimage, JSAMPROW y)
{
	int i;
	JSAMPROW u,v;
	
	cinfo->err = jpeg_std_error(jerr);
	jpeg_create_compress(cinfo);
	
	cinfo->image_width = width;
	cinfo->image_height = height;
	cinfo->input_components = 3;
	cinfo->in_color_space = JCS_YCbCr;
	jpeg_set_defaults(cinfo);
	
	/* cinfo->dct_method = JDCT_FLOAT; */
	cinfo->raw_data_in = TRUE;
	
	cinfo->comp_info[0].h_samp_factor = 2;
	cinfo->comp_info[0].v_samp_factor = 2;
	cinfo->comp_info[1].h_samp_factor = 1;
	cinfo->comp_info[1].v_samp_factor = 1;
	cinfo->comp_info[2].h_samp_factor = 1;
	cinfo->comp_info[2].v_samp_factor = 1;

	jimage[0] = malloc(height * 2 * sizeof(JSAMPROW));
	if(jimage[0] == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		exit(1);
	}
		
	jimage[1] = jimage[0] + height;
	jimage[2] = jimage[1] + (height/2);

	u = y + width * height;
	v = u + width * height / 4;
	
	for(i = 0; i < height; ++i, y+=width) {
		jimage[0][i] = y;
	}
	for(i = 0; i < height/2; ++i, u+=width/2, v+=width/2) {
		jimage[1][i] = u;
		jimage[2][i] = v;
	}

	jpeg_set_quality(cinfo, quality, TRUE);
}

void jpeg_write(int height, JSAMPIMAGE jimage, struct jpeg_compress_struct *cinfo,
		const char *fmt, const char *cmd)
{
        JSAMPARRAY jdata[3];
	char filename[1024];
	FILE *outfile;
	time_t tt;
	struct tm *tm;
	int i;

	tt = time(NULL);
	if(tt == (time_t)-1) {
		perror("Failed to get time");
		return;
	}

	tm = localtime(&tt);
	
	if(strftime(filename,1024,fmt,tm) == 0) {
		fprintf(stderr,"Error: resulting filename to long\n");
		return;
	}
	
	if ((outfile = fopen(filename, "wb")) == NULL) {
		perror("Error opening output file");
		return;
	}

	jdata[0] = jimage[0];
	jdata[1] = jimage[1];
	jdata[2] = jimage[2];
	
	jpeg_stdio_dest(cinfo, outfile);
	jpeg_start_compress(cinfo, TRUE);

	for (i = 0;i < height;i += 2*DCTSIZE) {
		jpeg_write_raw_data(cinfo, jdata, 2*DCTSIZE);
		jdata[0] += 2*DCTSIZE;
		jdata[1] += DCTSIZE;
		jdata[2] += DCTSIZE;
	}

	jpeg_finish_compress(cinfo);
	fclose(outfile);
	
	if(cmd != NULL) {
		switch(fork()) {
		case 0:
			dup2(STDERR_FILENO,STDOUT_FILENO);
			execlp(cmd,cmd,filename,NULL);
			fprintf(stderr,"Failed to execute %s: %s\n",cmd,strerror(errno));
			_exit(1);
		case -1:
			perror("fork failed");
			/* Fall through */
		default:
			break;
		}
	}
}

uint8_t motionmask[60][80];
int detectmotion(unsigned char *buf, int width, int height)
{
	static int newbuf;
	static int skip = 5;
	static uint32_t motionbuf[2][60][80];
	static int rectime;
	uint32_t diff;
	int line, col, motiondetected = 0;
	unsigned char *dst, *src = buf;
	int oldbuf = !newbuf;

	time_t tt;
	char *tp = NULL, *pt = NULL;

	if(showtime) {
		if((tt = time(NULL)) != (time_t)-1) {
			pt = ctime(&tt);
			pt[25] = '\0';
		}
	}
	
	memset(motionbuf[newbuf],0,60*80*sizeof(uint32_t));

	for(line = 0; line < height; ++line) {
		int y = line / 8; tp = pt;
		if(line < 8)
			dst = buf + line * width;
		for(col = 0; col < width; col += 8) {
			int x = col / 8;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;
			motionbuf[newbuf][y][x] += *src++;

			/* Add time string */
			if(tp != NULL && *tp != '\0' && line < 8) {
				int i;
				unsigned char c = (unsigned char)*tp++;
				
				if(c >= '0' && c <= ':')
					c = c - '0' + 1;
				else if(c >= 'a' && c <= 'z')
					c = c - 'a' + 12;
				else  if(c >= 'A' && c <= 'Z')
					c = c - 'A' + 38;
				else
					c = 0;

				for(i = 0; i < 7 && c != 0; ++i) {
					switch(pixels[c][line][i]) {
					case 1: dst[i] = 0;	break;
					case 2: dst[i] = 0xFF;	break;
					}
				}
				dst += (c == 0) ? 5 : 7;
			}

			if((line % 8) != 7 || motionmask[y][x])
				continue;

			diff = motionbuf[newbuf][y][x] - motionbuf[oldbuf][y][x];
			diff = diff & 0x80000000 ? 0xFFFFFFFF - diff : diff;

			if(diff > sensitivity && !skip) {
				motiondetected++;
				if(showmotion) {
					uint32_t *p = (uint32_t*)(src - 8);
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p -= (width / sizeof(uint32_t));
					p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF;
				}
			}
		}
	}
	if(motiondetected >= threshold) {
		rectime = recmargin + 1;
#ifndef NOGUI
		if(motionfs && !fullscreen) {
			SDL_WM_ToggleFullScreen(screen);
			fullscreen = !fullscreen;
			SDL_WM_GrabInput(fullscreen ? SDL_GRAB_ON : SDL_GRAB_OFF);
		}
#endif
		if(motionbeep)
			fprintf(stderr,"\a");

		if(motioncmd != NULL) {
			static struct timeval start;
			struct timeval cur, diff;
			
			gettimeofday(&cur,NULL);
			timersub(&cur,&start,&diff);

			if(diff.tv_sec > cmdinterval) {
				gettimeofday(&start,NULL);
							
				switch(fork()) {
				case 0:
					dup2(STDERR_FILENO,STDOUT_FILENO);
					execlp(motioncmd,motioncmd,NULL);
					fprintf(stderr,"Failed to execute %s: %s\n",motioncmd,strerror(errno));
					_exit(1);
				case -1:
					perror("fork failed");
					/* Fall through */
				default:
					break;
				}
			}
		}
	}
	if(motionrecord && rectime > 0) {
		write(STDOUT_FILENO,buf,(width*height*3)/2);
		rectime--;
	}
	if(skip > 0)
		--skip;
	newbuf = !newbuf;
	return 0;
}

void displaymask(unsigned char *buf, int width, int height)
{
	int x, y;
	for(y = 0; y < (height/8); ++y) {
		for(x = 0; x < (width/8); ++x) {
			if(motionmask[y][x]) {
				uint32_t *p = (uint32_t*)(buf + (x * 8) + (y * width * 8));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF; p += (width / sizeof(uint32_t));
				p[0] ^= 0xFFFFFFFF; p[1] ^= 0xFFFFFFFF;
			}
		}
	}
}

#ifndef NOGUI
void createmask(SDL_Event *event, SDL_Rect *rect, int w, int h)
{
	static int xstart = -1, ystart = -1;
	double divx = ((double)rect->w / w) * 8;
	double divy = ((double)rect->h / h) * 8;

	if(xstart == -1) {
		xstart = (int)(event->button.x / divx);
		ystart = (int)(event->button.y / divy);
	}
	else {
		int xlast = (int)(event->button.x / divx);
		int ylast = (int)(event->button.y / divy);
		int x, y;

		for(y = ystart ;y >= 0 && y < 60 && y <= ylast; y++)
			for(x = xstart; x >= 0 && x < 80 && x <= xlast; x++)
				motionmask[y][x] = (event->button.button == SDL_BUTTON_LEFT) ? 1 : 0;
		xstart = ystart = -1;
	}
}
#endif

#define PSZ_MAX 6
struct {
	char *name;
	int width;
	int height;
} sizes[PSZ_MAX] = {
	{ "sqcif", 128, 96 },
	{ "qsif", 160, 120 },
	{ "qcif", 176, 144 },
	{ "sif", 320, 240 },
	{ "cif", 352, 288 },
	{ "vga", 640, 480 }
};
	
int usage()
{
	fprintf(stderr,
#ifndef NOGUI
		"Usage: pwcview [ options ]\n\n"
#else
		"Usage: pwcsnap [ options ]\n\n"
#endif
		"Options:\n"
		" -?               Display this help message\n"
#ifndef NOGUI
		" -h               Run in headless mode\n"
#ifdef __FreeBSD__
		" -p               Enable webcam snapshot button\n"
#endif
		" -k               Add time date string to picture\n\n"
		" -x               Create window without frame\n"
		" -y               Use IYUV overlay instead of YV12 overlay\n"
		" -z               Create the video surface in system memory (SDL_SWSURFACE)\n"
		" -a               Always use video surface (SDL_ANYFORMAT)\n"
		" -b <bpp>         Bits per pixel to setup SDL video surface with (default: 0)\n\n"
#endif
		" -r               Enable motion detection recording\n"
		" -m               Show detected motion\n"
		" -j <command>     Command to execute when motion is detected (default: none)\n"
		" -g <interval>    Minimum time in seconds between motion commands (default: 60)\n"
		" -u <sensitivity> Motion detection sensitivity (default: 1000)\n"
		" -t <threshold>   Motion detection threshold (default: 1)\n"
		" -l <frames>      Recording margin (default: 15)\n\n"
		" -c <count>       Number of jpeg snapshots to take (default: 0 (-1=unlimited))\n"
		" -i <interval>    Jpeg snapshot interval in milliseconds (default: 3000)\n"
		" -q <quality>     Quality of jpeg output image (range: 0-100, default: 75)\n"
		" -o <outfile>     Filename for jpeg output (default: /tmp/%%Y%%m%%d%%H%%M%%S.jpg)\n"
		" -e <command>     Command to execute after each snaphot (default: none)\n\n"
		" -d <device>      Video device to open (default: /dev/video0)\n"
		" -s <size>        Video size to use (default: sif)\n"
		" -f <fps>         Video framerate to use (default: 5)\n\n"
		" See the pwcview(1) man page for details\n\n");

	return 1;
}

int
main(int argc, char **argv)
{
#ifndef NOGUI
	Uint8 *keylist;
	SDL_Overlay *overlay;
	SDL_Event event;
	SDL_TimerID timerid;
	int initflags = SDL_INIT_VIDEO;
	int format = SDL_YV12_OVERLAY;
	int swsurface = 0;
	int ysize, uvsize, rv;
	unsigned char *u, *v;
	int mode = 0, skipupdate = 0;
	char buf[80];
	Uint32 interval = 3000;
	int headless = 0;
#else
	unsigned int interval = 3000;
	int headless = 1;
#endif
	struct timespec tv;
	struct video_picture vp;
	struct pwc_probe probe;
	const char *device = "/dev/video0";
	unsigned int fps = 5;

	int fdmask;
	int showmask = 0;
	int snapbtn = 0;
	int snapcnt = 0;
	int frozen = 0;
	int i = 3; /* sizeidx (sif) */
	
        JSAMPARRAY jdata[3];
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int quality = 75;
	const char *outfile = "/tmp/%Y%m%d%H%M%S.jpg";
	const char *command = NULL;
	
	int fd;
	int imgsize;
	int ch, size;
	unsigned char *y;

	while((ch = getopt(argc,argv,"yaxhrmkpzl:b:q:o:d:e:f:i:t:u:j:g:c:s:?")) != -1) {
		switch(ch) {
#ifndef NOGUI
		case 'y': format = SDL_IYUV_OVERLAY;		break;
		case 'a': sdlflags |= SDL_ANYFORMAT;		break;
		case 'x': sdlflags |= SDL_NOFRAME;		break;
		case 'h': headless = 1;				break;
#ifdef __FreeBSD__
		case 'p': snapbtn = 1;				break;
#endif
		case 'z': swsurface = 1;			break;
		case 'b': bpp = atoi(optarg);			break;
#endif
		case 'q': quality = atoi(optarg);		break;
		case 'o': outfile = optarg;			break;
		case 'd': device = optarg;			break;
		case 'e': command = optarg;			break;
		case 'r': motionrecord = 1;			break;
		case 'm': showmotion = 1;			break;
		case 'k': showtime = 1;				break;
		case 'l': recmargin = atoi(optarg);		break;
		case 'f': fps = atoi(optarg);			break;
		case 'i': interval = strtoul(optarg,NULL,10);	break;
		case 't': threshold = strtoul(optarg,NULL,10);	break;
		case 'u': sensitivity = strtoul(optarg,NULL,10);break;
		case 'j': motioncmd = optarg;			break;
		case 'g': cmdinterval = strtol(optarg,NULL,10);	break;
		case 'c':
			snapcnt  = atoi(optarg);
			if(snapcnt < -1) {
				fprintf(stderr,"Invalid snapshot count: %d\n",snapcnt);
				return 1;
			}
			break;

		case 's':
			for(i = 0; i < PSZ_MAX; ++i)
				if(strcmp(sizes[i].name,optarg) == 0)
					  break;

			if(i == PSZ_MAX) {
				fprintf(stderr,"Invalid size, valid sizes: sqcif, qsif, qcif, sif, cif, vga\n");
				return 1;
			}
			break;
			
		case '?':
		default:
			return usage();
		}
	}
	if(headless && motionrecord == 1 && snapcnt != 0) {
		fprintf(stderr,"Error: You cannot use both motion detection recording\n"
			       "       and jpeg snapshots in headless mode\n");
		return 1;
	}
	if(motionrecord == 1 && isatty(STDOUT_FILENO)) {
		fprintf(stderr,"Error: You must redirect stdout to a file or pipe stdout to another\n"
			       "       command when motion detection recording is enabled\n");
		return 1;
	}

	if(fps < 5 || fps > 30) {
		fprintf(stderr,"Invalid framerate, framerate must be in the range 5-30\n");
		return 1;
	}

	if(!headless && interval < (((1000 / fps)/10)*10))
		interval = (((1000 / fps)/10)*10);

	vw.width = sizes[i].width;
	vw.height= sizes[i].height;
	vw.flags = fps << PWC_FPS_SHIFT;
	imgsize = (vw.width * vw.height * 3)/2;

	fd = 0;

	if ((fd = v4l1_open(device,O_RDWR,0)) < 0) {
		if(errno == EBUSY)
			fprintf(stderr,"Failed to access webcam: Device in use\n");
		else {
			perror("Failed to access webcam");
			fprintf(stderr,"***********************************************************\n"
#ifdef __FreeBSD__
				       "Make sure you have connected your webcam to the root hub\n"
				       "or to a USB 1.1 hub, also check your dmesg for any errors.\n"
#else
				       "Please check your dmesg for any errors.\n"
#endif
				       "***********************************************************\n");
		}
		exit(1);
    	}
	//fcntl(fd,F_SETFD,FD_CLOEXEC);

	if(v4l1_ioctl(fd, VIDIOCGPICT,&vp) == -1) {
		perror("Failed to get current picture info");
		exit(1);
	}
	vp.palette = VIDEO_PALETTE_YUV420P;
	if(v4l1_ioctl(fd, VIDIOCSPICT,&vp) == -1) {
		perror("Failed to set palette to YUV420P");
		exit(1);
	}
	
	if(v4l1_ioctl(fd, VIDIOCSWIN,&vw) == -1) {
		fprintf(stderr,"Failed to set webcam to: %dx%d (%s) at %d fps (%s)\n",
				vw.width,vw.height,sizes[i].name,fps,strerror(errno));
		exit(1);
	}
	fprintf(stderr,"Webcam set to: %dx%d (%s) at %d fps\n",vw.width,vw.height,sizes[i].name,fps);

	if(headless && snapcnt == 0 && motionrecord == 0) { /* Done */
		close(fd);
		exit(0);
	}
	if(snapbtn) {
		snapbtn = 0;
		if(v4l1_ioctl(fd, VIDIOCPWCPROBE,&probe) != -1 &&
		   probe.type >= 720 && probe.type <= 740)
				snapbtn = 1;
	}
	
	if((fdmask = open("pwcview.msk",O_RDONLY)) != -1) {
		read(fdmask,motionmask,60*80*sizeof(uint8_t));
		close(fdmask);
		fprintf(stderr,"motion mask loaded\n");
	}
	
	y = malloc(imgsize);
	if(y == NULL) {
		perror("Out of memory");
		exit(1);
	}
	jpeg_init(vw.width,vw.height,quality,&cinfo,&jerr,jdata,(JSAMPROW)y);

	if(command != NULL ||motioncmd != NULL)
		signal(SIGCHLD,sig_chld);

#ifndef NOGUI
	if(!headless) {
		
		window.w = vw.width;
		window.h = vw.height;
		rect.w = vw.width;
		rect.h = vw.height;
		ysize = vw.width * vw.height;
		uvsize = ysize / 4;
	
		if(format == SDL_IYUV_OVERLAY) {
			u = y + ysize;
			v = u + uvsize;
		}
		else {
			v = y + ysize;
			u = v + uvsize;
		}
		
		if(snapcnt != 0)
			initflags |= SDL_INIT_TIMER;
		
		if (SDL_Init(initflags) < 0) {
			fprintf(stderr,"Failed to init sdl: %s\n", SDL_GetError());
			exit(1);
		}

		atexit(SDL_Quit);

		sdlflags |= swsurface ? SDL_SWSURFACE : SDL_HWSURFACE;
		if((screen = SDL_SetVideoMode(window.w, window.h, bpp, sdlflags)) == NULL) {
			fprintf(stderr,"SDL Failed to set videomode: %s\n", SDL_GetError());
			exit(1);
		}
		if((overlay = SDL_CreateYUVOverlay(vw.width, vw.height, format, screen)) == NULL) {
			fprintf(stderr,"Failed to create yuvoverlay: %s\n", SDL_GetError());
			exit(1);
		}
		SDL_DisplayYUVOverlay(overlay, &rect);
	
		snprintf(buf,80,"pwcview");
		keylist = SDL_GetKeyState(NULL);

		if(snapcnt != 0)
			timerid = SDL_AddTimer(interval,cbtimer,NULL);
	}
#endif
	while (frozen || ((size = v4l1_read(fd,y,imgsize)) > 0) || (size == -1 && errno == EINTR)) {
		int snap = y[0] & 0x01;

		if(!frozen && size != imgsize) {
			if(size != -1) {
				fprintf(stderr,"Warning short read, got only %d of %d bytes\n",size,imgsize);
			}
			continue;
		}
		if(!frozen && (motionrecord||showmotion||showtime||motionfs||motionbeep||motioncmd))
			detectmotion(y,vw.width,vw.height);
		if(showmask)
			displaymask(y, vw.width, vw.height);

		if(headless) {
			if(motionrecord)
				continue;

			jpeg_write(vw.height,jdata,&cinfo,outfile,command);
			if(snapcnt > 0)
				snapcnt--;
			if(snapcnt == 0)
				exit(0);

			tv.tv_sec = interval / 1000;
			tv.tv_nsec = (interval % 1000) * 1000000;

			while(nanosleep(&tv,&tv) == -1 && errno == EINTR)
				;

			continue;
		}
		if(snapbtn && snap)
			jpeg_write(vw.height,jdata,&cinfo,outfile,command);
			
#ifndef NOGUI

		SDL_LockYUVOverlay(overlay);
		memcpy(overlay->pixels[0],y,ysize);
		memcpy(overlay->pixels[1],u,uvsize);
		memcpy(overlay->pixels[2],v,uvsize);
		SDL_UnlockYUVOverlay(overlay);
		SDL_DisplayYUVOverlay(overlay, &rect);
		
		SDL_PumpEvents();
		
		if(skipupdate == 0) {
			if(showmask)
				SDL_WM_SetCaption("edit motion mask:", "edit motion mask:");
			else
				SDL_WM_SetCaption(buf,buf);
		}
		skipupdate = 1;

		if(keylist[SDLK_RIGHT]) {
			skipupdate = handler[mode](fd,1,buf);
			continue;
		}
		else if(keylist[SDLK_LEFT]) {
			skipupdate = handler[mode](fd,-1,buf);
			continue;
		}
				
		if(frozen)
			rv = SDL_WaitEvent(&event);
		else
			rv = SDL_PollEvent(&event);

		if(rv) {
			do {
				if(event.type == SDL_KEYDOWN) {
					switch(event.key.keysym.sym) {
					case SDLK_DOWN:
						if(mode != sizeof(handler)/sizeof(handler[0]) - 1)
							mode++;
						else
							mode = 0;
						skipupdate = handler[mode](fd,0,buf);
						break;
					case SDLK_UP:
						if(mode != 0)
							mode--;
						else
							mode =  sizeof(handler)/sizeof(handler[0]) - 1;
						skipupdate = handler[mode](fd,0,buf);
						break;
					case SDLK_RETURN:
						skipupdate = handler[mode](fd,2,buf);
						break;
					case SDLK_f:
						SDL_WM_ToggleFullScreen(screen);
						fullscreen = !fullscreen;
						SDL_WM_GrabInput(fullscreen ? SDL_GRAB_ON : SDL_GRAB_OFF);
						break;
					case SDLK_m:
						showmask = !showmask;
						skipupdate = 0;
						break;
					case SDLK_w:
						if((fdmask = open("pwcview.msk",O_CREAT|O_TRUNC|O_WRONLY,0644)) != -1) {
							write(fdmask,motionmask,60*80*sizeof(uint8_t));
							close(fdmask);
							fprintf(stderr,"motion mask saved\n");
						}
						break;
					case SDLK_p:
						jpeg_write(vw.height,jdata,&cinfo,outfile,command);
						break;
					case SDLK_SPACE:
						frozen = !frozen;
						break;
					case SDLK_q:
						exit(0);
					default:
						break;
				
					}
				}
				else if(event.type == SDL_VIDEORESIZE) {
					window.w = event.resize.w;
					window.h = event.resize.h;
					if(scale == SCALE_FULL) {
						rect.w = window.w;
						rect.h = window.h;
					}
					else if(scale == SCALE_DOUBLE) {
						if(window.w >= (2 * vw.width) && window.h >= (2 * vw.height)) {
							rect.w = 2 * vw.width;
							rect.h = 2 * vw.height;
						}
						else {
							rect.w = vw.width;
							rect.h = vw.height;
						}
					}
					if((screen = SDL_SetVideoMode(window.w, window.h, bpp, sdlflags)) == NULL) {
						fprintf(stderr,"SDL Failed to set videomode: %s\n", SDL_GetError());
						exit(1);
					}
				}
				else if(event.type == SDL_USEREVENT) {
					jpeg_write(vw.height,jdata,&cinfo,outfile,command);
					
					if(snapcnt > 0)
						snapcnt--;
					if(snapcnt == 0)
						SDL_RemoveTimer(timerid);
				}
				else if(event.type == SDL_MOUSEBUTTONDOWN) {
					if(showmask) {
						if(event.button.button == SDL_BUTTON_MIDDLE) 
							memset(motionmask,0,60*80*sizeof(uint8_t));
						else
							createmask(&event,&rect,vw.width,vw.height);
					}
					else {
						SDL_WM_ToggleFullScreen(screen);
						fullscreen = !fullscreen;
						SDL_WM_GrabInput(fullscreen ? SDL_GRAB_ON : SDL_GRAB_OFF);
					}
				}
				else if(event.type == SDL_QUIT) {
					exit(0);
				}
			}while(!frozen && SDL_PollEvent(&event));
		}
#endif
	}
	
	if(size != 0)
		perror("Error reading from webcam");

	v4l1_close(fd);
	jpeg_destroy_compress(&cinfo);
	return 0;
}
