
#include <math.h>
#include <userdata.h>
#include <main.h>
#include <screen_fms.h>
#include <my_mavlink.h>
#include <screen_wpedit.h>
#include <screen_map.h>

int16_t fms_pos = 0;
uint16_t fms_sel = 0;

uint8_t fms_null (char *name, float x, float y, int8_t button, float data) {
	return 0;
}

uint8_t fms_add (char *name, float x, float y, int8_t button, float data) {
	return 0;
}

uint8_t fms_del (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	uint16_t n = 0;
	uint16_t n2 = waypoint_active + 1;
	for (n = waypoint_active; n < MAX_WAYPOINTS; n++) {
		if (WayPoints[n].p_lat != 0.0) {
			WayPoints[n].p_lat = WayPoints[n2].p_lat;
			WayPoints[n].p_long = WayPoints[n2].p_long;
			WayPoints[n].p_alt = WayPoints[n2].p_alt;
			WayPoints[n].yaw = WayPoints[n2].yaw;
			strcpy(WayPoints[n].name, WayPoints[n2].name);
			strcpy(WayPoints[n].command, WayPoints[n2].command);
			n2++;
		}
	}
	waypoint_active--;
	return 0;
}

uint8_t fms_scroll (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	if ((int)data < 0) {
		if (fms_pos > 0) {
			fms_pos += (int)data;
		}
	} else {
		fms_pos += (int)data;
	}
	return 0;
}

uint8_t fms_select (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	waypoint_active = (int)data;
	return 0;
}

void screen_fms (ESContext *esContext) {
#ifndef SDLGL
	ESMatrix modelview;
	UserData *userData = esContext->userData;
#endif
	char tmp_str[100];
	char tmp_str2[100];
	int n = 0;
	int n2 = 0;
	float last_lat = 0.0;
	float last_lon = 0.0;
	float last_alt = 0.0;
	float alt = 0.0;
	float ss = -0.7;

#ifndef SDLGL
	esMatrixLoadIdentity(&modelview);
	esMatrixMultiply(&userData->mvpMatrix, &modelview, &userData->perspective);
	esMatrixMultiply(&userData->mvpMatrix2, &modelview, &userData->perspective);
#endif

	draw_title(esContext, "Waypoints");

	for (n = fms_pos * 7; n < MAX_WAYPOINTS && n2 < 7; n++) {
		if (n == 0) {
			draw_box_f(esContext, -1.2, -0.55 + ((n2 - 1) * 0.22), 1.3, -0.55 + (n2 * 0.22), 127, 127, 127, 64);
		} else if (n == waypoint_active) {
			draw_box_f(esContext, -1.2, -0.55 + ((n2 - 1) * 0.22), 1.3, -0.55 + (n2 * 0.22), 255, 255, 255, 64);
			draw_line_f(esContext, ss - 0.01, -0.53 + ((n2 - 1) * 0.22), ss, -0.51 + ((n2 - 1) * 0.22), 0, 255, 0, 255);
			draw_line_f(esContext, ss + 0.01, -0.53 + ((n2 - 1) * 0.22), ss, -0.51 + ((n2 - 1) * 0.22), 0, 255, 0, 255);
		}
		if (WayPoints[n].p_lat != 0.0) {
			float distance1 = 0.0;
			float distance2 = 0.0;
			float winkel_up = 0.0;
			if (last_lat != 0.0) {
				/* Distance - Ground-Level */
				distance1 = acos( 
					cos(toRad(last_lat))
					* cos(toRad(WayPoints[n].p_lat))
					* cos(toRad(last_lon) - toRad(WayPoints[n].p_long))
					+ sin(toRad(last_lat)) 
					* sin(toRad(WayPoints[n].p_lat))
				) * 6378.137 * 1000.0;
				alt = WayPoints[n].p_alt - last_alt;
				/* Distance - Sichtverbindung */
				distance2 = sqrt(((distance1) * (distance1)) + (alt * alt));
				/* Steigung */
				winkel_up = (asin(alt / distance2)) * 180 / PI;
				draw_line_f(esContext, ss, -0.62 + ((n2 - 1) * 0.22), ss, -0.62 + (n2 * 0.22), 0, 255, 0, 255);
				draw_circleFilled_f(esContext, ss, -0.62 + ((n2 - 1) * 0.22), 0.01, 0, 255, 0, 128);
				draw_circleFilled_f(esContext, ss, -0.62 + (n2 * 0.22), 0.01, 0, 255, 0, 128);
				sprintf(tmp_str, "%0.1fm (%0.1fm / %0.1fGrad)", distance1, distance2, winkel_up);
				draw_text_f3(esContext, ss + 0.01, -0.62 + (n2 * 0.22) - 0.125, 0.002, 0.05, 0.05, FONT_GREEN, tmp_str);
			}
			last_lat = WayPoints[n].p_lat;
			last_lon = WayPoints[n].p_long;
			last_alt = WayPoints[n].p_alt;
			sprintf(tmp_str, "%s", WayPoints[n].name);
			sprintf(tmp_str2, "%s-%i-a", WayPoints[n].name, n);
			draw_button(esContext, tmp_str2, VIEW_MODE_FMS, tmp_str, FONT_GREEN, -1.2, -0.7 + (n2 * 0.22), 0.002, 0.1, 0, 0, fms_select, (float)n);

			if (WayPoints[n].command[0] == 0) {
				strcpy(tmp_str, "---");
			} else {
				sprintf(tmp_str, "%s", WayPoints[n].command);
			}
			draw_text_f3(esContext, -1.2 + 0.01, -0.7 + (n2 * 0.22) - 0.04, 0.002, 0.05, 0.05, FONT_GREEN, tmp_str);
			if (WayPoints[n].p_lat == 0.0) {
				strcpy(tmp_str, "---");
			} else {
				sprintf(tmp_str, "%0.6f", WayPoints[n].p_lat);
			}
			sprintf(tmp_str2, "%s-%i-b", WayPoints[n].name, n);
			draw_button(esContext, tmp_str2, VIEW_MODE_FMS, tmp_str, FONT_GREEN, -0.5, -0.7 + (n2 * 0.22), 0.002, 0.1, 0, 0, fms_select, (float)n);

			if (WayPoints[n].p_long == 0.0) {
				strcpy(tmp_str, "---");
			} else {
				sprintf(tmp_str, "%0.6f", WayPoints[n].p_long);
			}
			sprintf(tmp_str2, "%s-%i-c", WayPoints[n].name, n);
			draw_button(esContext, tmp_str2, VIEW_MODE_FMS, tmp_str, FONT_GREEN, 0.1, -0.7 + (n2 * 0.22), 0.002, 0.1, 0, 0, fms_select, (float)n);

			if (WayPoints[n].p_alt == 0.0) {
				strcpy(tmp_str, "---");
			} else {
				sprintf(tmp_str, "%0.1fm", WayPoints[n].p_alt);
			}
			sprintf(tmp_str2, "%s-%i-d", WayPoints[n].name, n);
			draw_button(esContext, tmp_str2, VIEW_MODE_FMS, tmp_str, FONT_GREEN, 1.0, -0.7 + (n2 * 0.22), 0.002, 0.1, 2, 0, fms_select, (float)n);
			n2++;
		}
	}
	n2 = 0;
	for (n = 0; n < MAX_WAYPOINTS; n++) {
		if (WayPoints[n].p_lat != 0.0) {
			n2++;
		}
	}

	if (n2 > 7) {
		draw_scrollbar(esContext, fms_pos, n2 / 7, fms_scroll);
	}
	if (fms_pos < 0) {
		fms_pos = 0;
	} else if (fms_pos > n2 / 7) {
		fms_pos = n2 / 7;
	}



	draw_button(esContext, "fms_edit", VIEW_MODE_FMS, "EDIT", FONT_GREEN_BG, -0.7, 0.9, 0.002, 0.06, ALIGN_CENTER, ALIGN_TOP, wpedit_waypoint_edit, (float)VIEW_MODE_FMS);
	draw_button(esContext, "fms_add", VIEW_MODE_FMS, "ADD", FONT_GREEN_BG, -0.5, 0.9, 0.002, 0.06, 1, 0, fms_add, 0.0);
	draw_button(esContext, "fms_del", VIEW_MODE_FMS, "DEL", FONT_GREEN_BG, 0.5, 0.9, 0.002, 0.06, 1, 0, fms_del, 0.0);
	draw_button(esContext, "fms_read_wp", VIEW_MODE_FMS, "READ_WP", FONT_GREEN_BG, -0.2, 0.9, 0.002, 0.06, 1, 0, read_wp, 0.0);
	draw_button(esContext, "fms_write_wp", VIEW_MODE_FMS, "WRITE_WP", FONT_GREEN_BG, 0.2, 0.9, 0.002, 0.06, 1, 0, write_wp, 0.0);
	draw_button(esContext, "fms_goto", VIEW_MODE_FMS, "GOTO", FONT_GREEN_BG, 0.7, 0.9, 0.002, 0.06, 1, 0, map_goto, (float)waypoint_active);

}

