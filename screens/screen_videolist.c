
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include <userdata.h>
#include <main.h>
#include <screen_videolist.h>
#include <screen_filesystem.h>

uint8_t videolist_startup = 0;
char videolist_path[401];
char videolist_lastfile[401];

uint8_t videolist_null (char *name, float x, float y, int8_t button, float data) {
	return 0;
}

uint8_t videolist_file_play (char *name, float x, float y, int8_t button, float data) {
	char cmd[401];
	strcpy(videolist_lastfile, name);
	sprintf(cmd, "echo \"%s\" > /tmp/gcs.playfile", name);
	system(cmd);
	SDL_Event user_event;
	user_event.type=SDL_QUIT;
	SDL_PushEvent(&user_event);
	return 0;
}

void screen_videolist (ESContext *esContext) {

	if (videolist_startup == 0) {
		if (videolist_lastfile[0] != 0) {
			strcpy(videolist_path, videolist_lastfile);
			dirname(videolist_path);
		} else {
			sprintf(videolist_path, "/media");
		}
		filesystem_set_dir(videolist_path);
		videolist_startup = 1;
	}

	filesystem_set_callback(videolist_file_play);
	filesystem_reset_filter();
	filesystem_add_filter(".avi\0");
	filesystem_add_filter(".AVI\0");
	filesystem_add_filter(".mp4\0");
	filesystem_add_filter(".MP4\0");
	filesystem_add_filter(".mpeg\0");
	filesystem_add_filter(".mpg\0");
	filesystem_set_mode(VIEW_MODE_VIDEOLIST);

	screen_filesystem(esContext);

	draw_title(esContext, "Video-Player");

}

