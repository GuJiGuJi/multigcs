
#include <userdata.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <main.h>
#include <model.h>
#include <screen_mavlink_menu.h>
#include <screen_keyboard.h>
#include <screen_filesystem.h>
#include <my_mavlink.h>
#include <geomag70.h>

float sel = 0.0;
float set_sel = 0.0;
float sel2 = 0.0;
float set_sel2 = 0.0;

static char select_section[100];
static int option_menu = -1;
static int bits_menu = -1;
static int param_menu = -1;
static float param_last = 0.0;

#define SLIDER_START	0.35
#define SLIDER_LEN	0.7

#define SLIDER2_START	-1.2
#define SLIDER2_LEN	2.4



void mavlink_meta_get_option (int id, char *name, char *entry) {
	char tmp_str3[1024];
	int n = 0;
	int n2 = 0;
	int n3 = 0;
	int n4 = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name) == 0) {
			selected = n;
			break;
		}
	}
	if (selected == -1) {
		return;
	}
	MavLinkVars[n].min = atof(MavLinkVars[n].values);
	for (n2 = 0; n2 < strlen(MavLinkVars[n].values); n2++) {
		if (MavLinkVars[n].values[n2] == ',') {
			strcpy(tmp_str3, MavLinkVars[n].values + n3);
			for (n4 = 0; n4 < strlen(tmp_str3); n4++) {
				if (tmp_str3[n4] == ',') {
					tmp_str3[n4] = 0;
					break;
				}
			}
			if (atoi(tmp_str3) == id) {
				strcpy(entry, tmp_str3);
//				break;
			}
			n3 = n2 + 1;
		}
	}
	strcpy(tmp_str3, MavLinkVars[n].values + n3);
	MavLinkVars[n].max = atof(tmp_str3);
	if (atoi(tmp_str3) == id) {
		strcpy(entry, tmp_str3);
	}
}

void mavlink_meta_get_bits (int id, char *name, char *entry) {
	char tmp_str3[1024];
	int n = 0;
	int n2 = 0;
	int n3 = 0;
	int n4 = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name) == 0) {
			selected = n;
			break;
		}
	}
	if (selected == -1) {
		return;
	}
	MavLinkVars[n].min = atof(MavLinkVars[n].bits);
	for (n2 = 0; n2 < strlen(MavLinkVars[n].bits); n2++) {
		if (MavLinkVars[n].bits[n2] == ',') {
			strcpy(tmp_str3, MavLinkVars[n].bits + n3);
			for (n4 = 0; n4 < strlen(tmp_str3); n4++) {
				if (tmp_str3[n4] == ',') {
					tmp_str3[n4] = 0;
					break;
				}
			}
			if (atoi(tmp_str3) == id) {
				strcpy(entry, tmp_str3);
//				break;
			}
			n3 = n2 + 1;
		}
	}
	strcpy(tmp_str3, MavLinkVars[n].bits + n3);
	MavLinkVars[n].max = atof(tmp_str3);
	if (atoi(tmp_str3) == id) {
		strcpy(entry, tmp_str3);
	}
}

uint8_t mavlink_param_set (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 1) == 0) {
			selected = n;
			break;
		}
	}
	MavLinkVars[selected].value = data;
	if (MavLinkVars[selected].value < MavLinkVars[selected].min) {
		MavLinkVars[selected].value = MavLinkVars[selected].min;
	} else if (MavLinkVars[selected].value > MavLinkVars[selected].max) {
		MavLinkVars[selected].value = MavLinkVars[selected].max;
	}
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_param_diff (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 6) == 0) {
			selected = n;
			break;
		}
	}
	MavLinkVars[selected].value += data;
	if (MavLinkVars[selected].value < MavLinkVars[selected].min) {
		MavLinkVars[selected].value = MavLinkVars[selected].min;
	} else if (MavLinkVars[selected].value > MavLinkVars[selected].max) {
		MavLinkVars[selected].value = MavLinkVars[selected].max;
	}
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_slider_move (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 1) == 0) {
			selected = n;
			break;
		}
	}
	if (button == 4) {
		if (MavLinkVars[selected].type != 0) {
			MavLinkVars[selected].value += 1;
		} else {
			MavLinkVars[selected].value += 0.01;
		}
	} else if (button == 5) {
		if (MavLinkVars[selected].type != 0) {
			MavLinkVars[selected].value -= 1;
		} else {
			MavLinkVars[selected].value -= 0.01;
		}
	} else {
		float percent = (x - SLIDER_START) * 100.0 / SLIDER_LEN;
		if (percent > 100.0) {
			percent = 100.0;
		} else if (percent < 0.0) {
			percent = 0.0;
		}
		float diff = MavLinkVars[selected].max - MavLinkVars[selected].min;
		float new = percent * diff / 100.0 + MavLinkVars[selected].min;
		if (MavLinkVars[selected].type != 0) {
			int conv = (int)(new);
			new = (float)conv;
		}
		printf("slider: %s %f %f %f %f\n", name + 1, x, percent, new, data);
		if (strstr(MavLinkVars[selected].name, "baud") > 0) {
			float bauds[] = {1200.0, 2400.0, 9600.0, 38400.0, 57600.0, 115200.0, 200000.0};
			for (n = 0; n < 6; n++) {
				if (new <= bauds[n] + (bauds[n + 1] - bauds[n]) / 2) {
					new = bauds[n];
					break;
				}
			}
		}
		MavLinkVars[selected].value = new;
	}
	if (MavLinkVars[selected].value < MavLinkVars[selected].min) {
		MavLinkVars[selected].value = MavLinkVars[selected].min;
	} else if (MavLinkVars[selected].value > MavLinkVars[selected].max) {
		MavLinkVars[selected].value = MavLinkVars[selected].max;
	}
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_slider2_move (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 1) == 0) {
			selected = n;
			break;
		}
	}
	if (button == 4) {
		if (MavLinkVars[selected].type != 0) {
			MavLinkVars[selected].value += 1;
		} else {
			MavLinkVars[selected].value += 0.01;
		}
	} else if (button == 5) {
		if (MavLinkVars[selected].type != 0) {
			MavLinkVars[selected].value -= 1;
		} else {
			MavLinkVars[selected].value -= 0.01;
		}
	} else {
		float percent = (x - SLIDER2_START) * 100.0 / SLIDER2_LEN;
		if (percent > 100.0) {
			percent = 100.0;
		} else if (percent < 0.0) {
			percent = 0.0;
		}
		float diff = MavLinkVars[selected].max - MavLinkVars[selected].min;
		float new = percent * diff / 100.0 + MavLinkVars[selected].min;
		if (MavLinkVars[selected].type != 0) {
			int conv = (int)(new);
			new = (float)conv;
		}
		printf("slider: %s %f %f %f %f\n", name + 1, x, percent, new, data);
		if (strstr(MavLinkVars[selected].name, "baud") > 0) {
			float bauds[] = {1200.0, 2400.0, 9600.0, 38400.0, 57600.0, 115200.0, 200000.0};
			for (n = 0; n < 6; n++) {
				if (new <= bauds[n] + (bauds[n + 1] - bauds[n]) / 2) {
					new = bauds[n];
					break;
				}
			}
		}
		MavLinkVars[selected].value = new;
	}
	if (MavLinkVars[selected].value < MavLinkVars[selected].min) {
		MavLinkVars[selected].value = MavLinkVars[selected].min;
	} else if (MavLinkVars[selected].value > MavLinkVars[selected].max) {
		MavLinkVars[selected].value = MavLinkVars[selected].max;
	}
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_set_magdecl (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 1) == 0) {
			selected = n;
			break;
		}
	}
	if (selected == -1) {
		return 0;
	}
	char tmp_str[100];
	int ret_dd = 0;
	int ret_dm = 0;
	get_declination(ModelData.p_lat, ModelData.p_long, ModelData.p_alt, &ret_dd, &ret_dm);
	sprintf(tmp_str, "%i%2.0i", ret_dd, ret_dm);
	MavLinkVars[selected].value = atof(tmp_str);
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_options_menu (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	option_menu = (int)data;
	return 0;
}

uint8_t mavlink_option_sel (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 3) == 0) {
			selected = n;
			break;
		}
	}
	if (selected == -1) {
		return 0;
	}
	MavLinkVars[selected].value = data;
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	option_menu = -1;
	return 0;
}

uint8_t mavlink_param_menu (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	param_menu = (int)data;
	if (param_menu != -1) {
		param_last = MavLinkVars[param_menu].value;
	}
	return 0;
}

uint8_t mavlink_bits_menu (char *name, float x, float y, int8_t button, float data) {
	reset_buttons();
	bits_menu = (int)data;
	return 0;
}

uint8_t mavlink_bits_sel (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	int selected = -1;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 3) == 0) {
			selected = n;
			break;
		}
	}
	if (selected == -1) {
		return 0;
	}
	int new = (int)MavLinkVars[selected].value;
	if (new & (1<<(int)data)) {
		new &= ~(1<<(int)data);
	} else {
		new |= (1<<(int)data);
	}
	MavLinkVars[selected].value = (float)new;
	mavlink_send_value(MavLinkVars[selected].name, MavLinkVars[selected].value, MavLinkVars[selected].type);
	return 0;
}

uint8_t mavlink_param_file_save (char *name, float x, float y, int8_t button, float data) {
	char filename[1024];
	FILE *fr;
	int n = 0;
	sprintf(filename, "%s/PARAMS/%s", BASE_DIR, name);
	if ((fr = fopen(filename, "w")) != 0) {
		fprintf(fr, "# Onboard parameters\n");
		fprintf(fr, "#\n");
		fprintf(fr, "# MAV ID  COMPONENT ID  PARAM NAME  VALUE (FLOAT)\n");
		for (n = 0; n < 500 - 1; n++) {
			if (MavLinkVars[n].name[0] != 0) {
				fprintf(fr, "%i	%i	%s	%f\n", ModelData.sysid, ModelData.compid, MavLinkVars[n].name, MavLinkVars[n].value);
			}
		}
		fprintf(fr, "\n");
		fclose(fr);
		printf("failed to save file: %s\n", filename);
	} else {
		printf("saved file: %s\n", filename);
	}
	return 0;
}

uint8_t mavlink_param_save (char *name, float x, float y, int8_t button, float data) {
	char filename[1024];
	sprintf(filename, "%s.txt", ModelData.name);
	keyboard_set_callback(mavlink_param_file_save);
	keyboard_set_text(filename);
	keyboard_set_mode(VIEW_MODE_FCMENU);
	return 0;
}

uint8_t mavlink_param_file_load (char *name, float x, float y, int8_t button, float data) {
	mavlink_param_read_file(name);
	return 0;
}

uint8_t mavlink_param_load (char *name, float x, float y, int8_t button, float data) {
	char directory[200];
	sprintf(directory, "%s/PARAMS", BASE_DIR);
	filesystem_set_callback(mavlink_param_file_load);
	filesystem_set_dir(directory);
	filesystem_reset_filter();
	filesystem_add_filter(".txt\0");
	filesystem_add_filter(".param\0");
	filesystem_set_mode(VIEW_MODE_FCMENU);
	return 0;
}

uint8_t mavlink_flash (char *name, float x, float y, int8_t button, float data) {
	save_to_flash();
	return 0;
}

uint8_t mavlink_flashload (char *name, float x, float y, int8_t button, float data) {
	load_from_flash();
	return 0;
}

uint8_t mavlink_aux_toggle (char *name, float x, float y, int8_t button, float data) {
	printf("aux: %s %f\n", name, data);
	int n = 0;
	for (n = 0; n < 500 - 1; n++) {
		if (strcmp(MavLinkVars[n].name, name + 3) == 0) {
			break;
		}
	}
	int new = (int)MavLinkVars[n].value;
	if (name[0] == 'S') {
		new |= (1<<(int)data);
	} else if (name[0] == 'R') {
		new &= ~(1<<(int)data);
	}
	MavLinkVars[n].value = (float)new;
	mavlink_send_value(MavLinkVars[n].name, MavLinkVars[n].value, MavLinkVars[n].type);
	reset_buttons();
	return 0;
}

uint8_t mavlink_select_main (char *name, float x, float y, int8_t button, float data) {
	strcpy(select_section, name + 8);
	set_sel = 0;
	reset_buttons();
	return 0;
}

uint8_t mavlink_select_sel (char *name, float x, float y, int8_t button, float data) {
	select_section[0] = 0;
	set_sel = 0;
	reset_buttons();
	return 0;
}

uint8_t mavlink_select_sel_scroll (char *name, float x, float y, int8_t button, float data) {
	if ((int)data > 0) {
		set_sel++;
	} else if (set_sel > 0) {
		set_sel--;
	}
	return 0;
}

void mavlink_param_read_file (char *param_file) {
        FILE *fr;
        char line[1024];
        int tmp_int1;
        int tmp_int2;
        char var1[101];
        char val[101];
        fr = fopen (param_file, "r");
        while(fgets(line, 100, fr) != NULL) {
                var1[0] = 0;
                val[0] = 0;
		if (line[0] != '#' && line[0] != '\n') {
	                sscanf (line, "%i %i %s %s", &tmp_int1, &tmp_int2, (char *)&var1, (char *)&val);
			float new_val = atof(val);
			uint16_t n = 0;
			uint8_t flag = 0;
			for (n = 0; n < 500; n++) {
				if (strcmp(MavLinkVars[n].name, var1) == 0) {
					float old_val = MavLinkVars[n].value;
					if (old_val != new_val) {
						printf ("CHANGED: %s = %f (OLD: %f)\n", var1, new_val, MavLinkVars[n].value);
						MavLinkVars[n].value = atof(val);
					}
					flag = 1;
					break;
				}
			}
			if (flag == 0) {
				for (n = 0; n < 500; n++) {
					if (MavLinkVars[n].name[0] == 0) {
						strcpy(MavLinkVars[n].name, var1);
						MavLinkVars[n].value = atof(val);
						MavLinkVars[n].id = -1;
				                printf ("NEW: %s = %f\n", var1, atof(val));
						break;
					}
				}
			}

		}
        }
        fclose(fr);
	reset_buttons();
}

uint8_t mavlink_param_upload_all (char *name, float x, float y, int8_t button, float data) {
	int n = 0;
	for (n = 0; n < 500 - 1; n++) {
		if (MavLinkVars[n].name[0] != 0) {
			mavlink_send_value(MavLinkVars[n].name, MavLinkVars[n].value, MavLinkVars[n].type);
			SDL_Delay(20);
		}
	}
	reset_buttons();
	return 0;
}

void screen_mavlink_menu (ESContext *esContext) {
	int16_t row = 0;
	int16_t col = 0;
	int16_t row2 = 0;
	int16_t n = 0;
	int16_t n2 = 0;
	int n3 = 0;
	char tmp_str[1024];
	char tmp_str2[1024];
	int8_t flag = 0;
	int8_t flag2 = 0;

	static char main_groups[500][100];

	draw_title(esContext, "MavLink-Parameter");

	row2 = 0;
	for (row = 0; row < 500 - 1; row++) {
		if (strlen(MavLinkVars[row].name) > 3) {
			strcpy(tmp_str, MavLinkVars[row].name);
			for (n = 0; n < strlen(tmp_str) ; n++) {
				if (tmp_str[n] == '_') {
					tmp_str[n] = 0;
					break;
				}
			}
			flag2 = 0;
			for (row2 = 0; row2 < 500 - 1; row2++) {
				if (strcmp(main_groups[row2], tmp_str) == 0) {
					flag2 = 1;
					break;
				}
			}
			if (flag2 == 0) {
				for (row2 = 0; row2 < 500 - 1; row2++) {
					if (main_groups[row2][0] == 0) {
						strcpy(main_groups[row2], tmp_str);
						break;
					}
				}
			}
			flag = 1;
		}
	}




	if (option_menu != -1) {
		draw_button(esContext, "#", VIEW_MODE_FCMENU, MavLinkVars[option_menu].name, FONT_GREEN, 0.0, -0.8, 2.002, 0.08, 1, 0, mavlink_options_menu, (float)-1);
		for (n2 = (int)MavLinkVars[option_menu].min; n2 <= (int)MavLinkVars[option_menu].max; n2++) {
			tmp_str2[0] = 0;
			mavlink_meta_get_option(n2, MavLinkVars[option_menu].name, tmp_str2);
			if (tmp_str2[0] == 0) {
				break;
			}
			sprintf(tmp_str, "%3.0i%s", n2, MavLinkVars[option_menu].name);
			if ((n2 - (int)MavLinkVars[option_menu].min) > 12) {
				if ((int)MavLinkVars[option_menu].value == n2) {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_GREEN, 0.0, -0.7 + (n2 - (int)MavLinkVars[option_menu].min - 13) * 0.1, 2.002, 0.08, 0, 0, mavlink_option_sel, n2);
				} else {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 0.0, -0.7 + (n2 - (int)MavLinkVars[option_menu].min - 13) * 0.1, 2.002, 0.08, 0, 0, mavlink_option_sel, n2);
				}
			} else {
				if ((int)MavLinkVars[option_menu].value == n2) {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_GREEN, -1.2, -0.7 + (n2 - (int)MavLinkVars[option_menu].min) * 0.1, 2.002, 0.08, 0, 0, mavlink_option_sel, n2);
				} else {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, -1.2, -0.7 + (n2 - (int)MavLinkVars[option_menu].min) * 0.1, 2.002, 0.08, 0, 0, mavlink_option_sel, n2);
				}
			}
		}
		draw_button(esContext, "back", VIEW_MODE_FCMENU, "[BACK]", FONT_WHITE, -1.3, 0.8, 2.002, 0.08, 0, 0, mavlink_options_menu, (float)-1);
	} else if (bits_menu != -1) {
		draw_button(esContext, "#", VIEW_MODE_FCMENU, MavLinkVars[bits_menu].name, FONT_GREEN, 0.0, -0.8, 2.002, 0.08, 1, 0, mavlink_options_menu, (float)-1);
		for (n2 = (int)0; n2 < 32; n2++) {
			tmp_str2[0] = 0;
			mavlink_meta_get_bits(n2, MavLinkVars[bits_menu].name, tmp_str2);
			if (tmp_str2[0] == 0) {
				break;
			}
			sprintf(tmp_str, "%3.0i%s", n2, MavLinkVars[bits_menu].name);
			if ((n2 - 0) > 12) {
				if ((int)MavLinkVars[bits_menu].value & (1<<n2)) {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_GREEN, 0.0, -0.7 + (n2 - 13) * 0.1, 2.002, 0.08, 0, 0, mavlink_bits_sel, n2);
				} else {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 0.0, -0.7 + (n2 - 13) * 0.1, 2.002, 0.08, 0, 0, mavlink_bits_sel, n2);
				}
			} else {
				if ((int)MavLinkVars[bits_menu].value & (1<<n2)) {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_GREEN, -1.2, -0.7 + (n2) * 0.1, 2.002, 0.08, 0, 0, mavlink_bits_sel, n2);
				} else {
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, -1.2, -0.7 + (n2) * 0.1, 2.002, 0.08, 0, 0, mavlink_bits_sel, n2);
				}
			}
		}
		draw_button(esContext, "back", VIEW_MODE_FCMENU, "[BACK]", FONT_WHITE, -1.3, 0.8, 2.002, 0.08, 0, 0, mavlink_bits_menu, (float)-1);
	} else if (param_menu != -1) {
		draw_button(esContext, "#", VIEW_MODE_FCMENU, MavLinkVars[bits_menu].name, FONT_GREEN, 0.0, -0.8, 2.002, 0.08, 1, 0, mavlink_param_menu, (float)-1);

		row = 1;

		sprintf(tmp_str, "mv_sel_%s_%i_t", MavLinkVars[param_menu].name, n);
		draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, MavLinkVars[param_menu].name, FONT_WHITE, 0.0, -0.8, 2.002, 0.15, 1, 0, mavlink_param_menu, -1.0);

		if (strlen(MavLinkVars[param_menu].desc) > 60) {
			strncpy(tmp_str, MavLinkVars[param_menu].desc, 100);
			tmp_str[60] = 0;
			draw_button(esContext, "#1", VIEW_MODE_FCMENU, tmp_str, FONT_WHITE, 0.0, -0.7 + row * 0.14 + 0.07, 0.002, 0.06, 1, 0, mavlink_param_menu, -1.0);
			draw_button(esContext, "#2", VIEW_MODE_FCMENU, MavLinkVars[param_menu].desc + 60, FONT_WHITE, 0.0, -0.7 + row * 0.14 + 0.07 + 0.07, 0.002, 0.06, 1, 0, mavlink_param_menu, -1.0);
		} else {
			draw_button(esContext, "#1", VIEW_MODE_FCMENU, MavLinkVars[param_menu].desc, FONT_WHITE, 0.0, -0.7 + row * 0.14 + 0.07, 0.002, 0.06, 1, 0, mavlink_param_menu, -1.0);
		}

		if (MavLinkVars[param_menu].type == 0) {
			sprintf(tmp_str, "diff1_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[-1.0]", FONT_WHITE, -1.0, 0.3, 2.002, 0.08, 0, 0, mavlink_param_diff, -1.0);

			sprintf(tmp_str, "diff2_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[-0.1]", FONT_WHITE, -1.2, 0.4, 2.002, 0.08, 0, 0, mavlink_param_diff, -0.1);

			sprintf(tmp_str, "diff3_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[-0.01]", FONT_WHITE, -0.8, 0.4, 2.002, 0.08, 0, 0, mavlink_param_diff, -0.01);

			sprintf(tmp_str, "diff4_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[-0.001]", FONT_WHITE, -0.4, 0.4, 2.002, 0.08, 0, 0, mavlink_param_diff, -0.001);

			sprintf(tmp_str, "diff5_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[+0.001]", FONT_WHITE, 0.4, 0.4, 2.002, 0.08, 2, 0, mavlink_param_diff, 0.001);

			sprintf(tmp_str, "diff6_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[+0.01]", FONT_WHITE, 0.8, 0.4, 2.002, 0.08, 2, 0, mavlink_param_diff, 0.01);

			sprintf(tmp_str, "diff7_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[+0.1]", FONT_WHITE, 1.2, 0.4, 2.002, 0.08, 2, 0, mavlink_param_diff, 0.1);

			sprintf(tmp_str, "diff8_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[+1.0]", FONT_WHITE, 1.0, 0.3, 2.002, 0.08, 2, 0, mavlink_param_diff, 1.0);
		} else {
			sprintf(tmp_str, "diff1_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[-1]", FONT_WHITE, -1.0, 0.3, 2.002, 0.08, 0, 0, mavlink_param_diff, -1.0);

			sprintf(tmp_str, "diff8_%s", MavLinkVars[param_menu].name);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "[+1]", FONT_WHITE, 1.0, 0.3, 2.002, 0.08, 2, 0, mavlink_param_diff, 1.0);
		}



		sprintf(tmp_str, "min00_%s", MavLinkVars[param_menu].name);
		if (MavLinkVars[param_menu].type != 0) {
			sprintf(tmp_str2, "%i", (int)MavLinkVars[param_menu].min);
		} else {
			sprintf(tmp_str2, "%0.4f", MavLinkVars[param_menu].min);
		}
		draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, -1.2, -0.1, 2.002, 0.08, 0, 0, mavlink_param_diff, -1.0);

		sprintf(tmp_str, "max00_%s", MavLinkVars[param_menu].name);
		if (MavLinkVars[param_menu].type != 0) {
			sprintf(tmp_str2, "%i", (int)MavLinkVars[param_menu].max);
		} else {
			sprintf(tmp_str2, "%0.4f", MavLinkVars[param_menu].max);
		}
		draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 1.2, -0.1, 2.002, 0.08, 2, 0, mavlink_param_diff, 1.0);

		draw_box_f3c2(esContext, SLIDER2_START, 0.0, 2.002, SLIDER2_START + SLIDER2_LEN, 0.2, 2.002, 55, 55, 55, 220, 75, 45, 85, 100);
		draw_box_f3c2(esContext, SLIDER2_START, 0.0, 2.002, SLIDER2_START + ((MavLinkVars[param_menu].value - MavLinkVars[param_menu].min) * SLIDER2_LEN / (MavLinkVars[param_menu].max - MavLinkVars[param_menu].min)), 0.2, 2.002, 255, 255, 55, 220, 175, 145, 85, 100);

		if (MavLinkVars[param_menu].type != 0) {
			sprintf(tmp_str2, "%i", (int)MavLinkVars[param_menu].value);
		} else {
			sprintf(tmp_str2, "%0.4f", MavLinkVars[param_menu].value);
		}
		sprintf(tmp_str, "value_%s", MavLinkVars[param_menu].name);
		draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 0.0, 0.1, 2.005, 0.09, 1, 1, mavlink_param_diff, 0.0);

		sprintf(tmp_str, "S%s", MavLinkVars[param_menu].name);
		set_button(tmp_str, view_mode, SLIDER2_START, 0.0, SLIDER2_START + SLIDER2_LEN, 0.2, mavlink_slider2_move, (float)param_menu, 1);


		if (strcmp(MavLinkVars[param_menu].name, "mag_declination") == 0) {
			int ret_dd = 0;
			int ret_dm = 0;
			get_declination(ModelData.p_lat, ModelData.p_long, ModelData.p_alt, &ret_dd, &ret_dm);
			sprintf(tmp_str, "M%s", MavLinkVars[param_menu].name);
			sprintf(tmp_str2, "[SET: %id%2.0im]", ret_dd, ret_dm);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 1.2, 0.6, 2.002, 0.08, 2, 0, mavlink_set_magdecl, 0.0);
		}

		sprintf(tmp_str, "V%s", MavLinkVars[param_menu].name);
		if (MavLinkVars[param_menu].type != 0) {
			sprintf(tmp_str2, "[RESET: %i]", (int)param_last);
		} else {
			sprintf(tmp_str2, "[RESET: %0.4f]", param_last);
		}
		draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 0.0, 0.6, 2.002, 0.08, 1, 0, mavlink_param_set, param_last);

		draw_button(esContext, "back", VIEW_MODE_FCMENU, "[BACK]", FONT_WHITE, -1.3, 0.8, 2.002, 0.08, 0, 0, mavlink_param_menu, (float)-1);
	} else if (select_section[0] == 0) {
		col = 0;
		row = 0;
		for (n = 0; n < 500; n++) {
			if (main_groups[n][0] == 0) {
				break;
			}
			sprintf(tmp_str, "mv_main_%s", main_groups[n]);
			draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, main_groups[n], FONT_WHITE, -1.3 + col * 0.5, -0.8 + row * 0.12, 0.002, 0.08, 0, 0, mavlink_select_main, n);
			col++;
			if (col > 4) {
				col = 0;
				row++;
			}
		}
	} else if (select_section[0] != 0) {
		col = 0;
		row = 0;
		int mav_id_sect = 0;
		int mav_id = 0;
		if ((int)set_sel > 0) {
			draw_button(esContext, "scroll_up", VIEW_MODE_FCMENU, "[^]", FONT_WHITE, 0.0, -0.7 - 0.14, 0.002, 0.08, 1, 0, mavlink_select_sel_scroll, -1.0);
		}
		for (mav_id = 0; mav_id < 500; mav_id++) {
			if (MavLinkVars[mav_id].name[0] == 0) {
				continue;
			}
			sprintf(tmp_str, "%s_", select_section);
			if (strncmp(MavLinkVars[mav_id].name, tmp_str, strlen(tmp_str)) == 0 || strcmp(MavLinkVars[mav_id].name, select_section) == 0) {
			} else {
				continue;
			}
			mav_id_sect++;
			if (mav_id_sect < (int)set_sel + 1 || mav_id_sect - (int)set_sel > 10) {
				continue;
			}
			if (strlen(MavLinkVars[mav_id].desc) > 60) {
				strncpy(tmp_str, MavLinkVars[mav_id].desc, 100);
				tmp_str[60] = 0;
				draw_button(esContext, "#", VIEW_MODE_FCMENU, tmp_str, FONT_WHITE, -1.19, -0.7 + row * 0.14 + 0.07, 0.002, 0.04, 0, 0, mavlink_select_sel, 0.0);
				draw_button(esContext, "#", VIEW_MODE_FCMENU, MavLinkVars[mav_id].desc + 60, FONT_WHITE, -1.18, -0.7 + row * 0.14 + 0.07 + 0.04, 0.002, 0.04, 0, 0, mavlink_select_sel, 0.0);
			} else {
				draw_button(esContext, "#", VIEW_MODE_FCMENU, MavLinkVars[mav_id].desc, FONT_WHITE, -1.19, -0.7 + row * 0.14 + 0.07, 0.002, 0.04, 0, 0, mavlink_select_sel, 0.0);
			}
			if (MavLinkVars[mav_id].type != 0) {
				sprintf(tmp_str2, "%i", (int)MavLinkVars[mav_id].value);
			} else {
				sprintf(tmp_str2, "%0.4f", MavLinkVars[mav_id].value);
			}
			if (
				strcmp(MavLinkVars[mav_id].name, "aux_angle") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_horizon") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_baro") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_mag") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_camstab") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_camtrig") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_arm") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_gps_home") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_gps_hold") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_gps_log") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_passthru") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_headfree") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_beeper") == 0
				||
				strcmp(MavLinkVars[mav_id].name, "aux_headadj") == 0
			) {
				sprintf(tmp_str, "mv_sel_%s_%i_t", MavLinkVars[mav_id].name, n);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, MavLinkVars[mav_id].name, FONT_WHITE, -1.2, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_select_sel, 0.0);
				n3 = 0;
				for (n2 = 0; n2 < 12; n2++) {
					if ((int)MavLinkVars[mav_id].value & (1<<n2)) {
						sprintf(tmp_str, "R%2.0i%s", n2, MavLinkVars[mav_id].name);
						draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "X", FONT_WHITE, -0.3 + n3 * 0.1, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_aux_toggle, (float)n2);
					} else {
						sprintf(tmp_str, "S%2.0i%s", n2, MavLinkVars[mav_id].name);
						draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, "O", FONT_WHITE, -0.3 + n3 * 0.1, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_aux_toggle, (float)n2);
					}
					if (n2 == 2 || n2 == 5 || n2 == 8) {
						n3++;
					}
					n3++;
				}
			} else if (MavLinkVars[mav_id].values[0] != 0) {
				sprintf(tmp_str, "mv_sel_%s_%i_t", MavLinkVars[mav_id].name, n);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, MavLinkVars[mav_id].name, FONT_WHITE, -1.2, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_options_menu, (float)(mav_id));
				mavlink_meta_get_option((int)MavLinkVars[mav_id].value, MavLinkVars[mav_id].name, tmp_str2);
				sprintf(tmp_str, "S%s", MavLinkVars[mav_id].name);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, -0.1, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_options_menu, (float)(mav_id));
			} else if (MavLinkVars[mav_id].bits[0] != 0) {
				sprintf(tmp_str, "mv_sel_%s_%i_t", MavLinkVars[mav_id].name, n);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, MavLinkVars[mav_id].name, FONT_WHITE, -1.2, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_bits_menu, (float)(mav_id));
				sprintf(tmp_str, "B%s", MavLinkVars[mav_id].name);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, ".....", FONT_WHITE, -0.1, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_bits_menu, (float)(mav_id));
			} else {
				sprintf(tmp_str, "mv_sel_%s_%i_t", MavLinkVars[mav_id].name, n);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, MavLinkVars[mav_id].name, FONT_WHITE, -1.2, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_param_menu, (float)(mav_id));
				sprintf(tmp_str, "mv_sel_%s_%i_v", MavLinkVars[mav_id].name, n);
				draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 0.05, -0.7 + row * 0.14, 0.002, 0.08, 2, 0, mavlink_param_menu, (float)(mav_id));
				sprintf(tmp_str, "S%s", MavLinkVars[mav_id].name);
				draw_box_f3c2(esContext, SLIDER_START, -0.7 + row * 0.14, 0.001, SLIDER_START + SLIDER_LEN, -0.7 + row * 0.14 + 0.1, 0.001, 55, 55, 55, 220, 75, 45, 85, 100);
				draw_box_f3c2(esContext, SLIDER_START, -0.7 + row * 0.14, 0.001, SLIDER_START + ((MavLinkVars[mav_id].value - MavLinkVars[mav_id].min) * SLIDER_LEN / (MavLinkVars[mav_id].max - MavLinkVars[mav_id].min)), -0.7 + row * 0.14 + 0.1, 0.001, 255, 255, 55, 220, 175, 145, 85, 100);
				set_button(tmp_str, view_mode, SLIDER_START, -0.7 + row * 0.14, SLIDER_START + SLIDER_LEN, -0.7 + row * 0.14 + 0.1, mavlink_slider_move, (float)row, 1);
				if (strcmp(MavLinkVars[mav_id].name, "mag_declination") == 0) {
					int ret_dd = 0;
					int ret_dm = 0;
					get_declination(ModelData.p_lat, ModelData.p_long, ModelData.p_alt, &ret_dd, &ret_dm);
					sprintf(tmp_str, "M%s", MavLinkVars[mav_id].name);
					sprintf(tmp_str2, "%i%2.0i", ret_dd, ret_dm);
					draw_button(esContext, tmp_str, VIEW_MODE_FCMENU, tmp_str2, FONT_WHITE, 1.2, -0.7 + row * 0.14, 0.002, 0.08, 0, 0, mavlink_set_magdecl, 0.0);
				}
			}
			row++;
		}
		if (mav_id_sect >= 10 && row >= 10) {
			draw_button(esContext, "scroll_down", VIEW_MODE_FCMENU, "[v]", FONT_WHITE, 0.0, 0.7, 0.002, 0.08, 1, 0, mavlink_select_sel_scroll, 1.0);
		}
		draw_button(esContext, "back", VIEW_MODE_FCMENU, "[BACK]", FONT_WHITE, -1.3, 0.8, 0.002, 0.08, 0, 0, mavlink_select_sel, 0.0);
	}

	draw_button(esContext, "load", VIEW_MODE_FCMENU, "[LOAD FILE]", FONT_WHITE, -1.0, 0.9, 0.002, 0.06, 1, 0, mavlink_param_load, 1.0);
	draw_button(esContext, "save", VIEW_MODE_FCMENU, "[SAVE FILE]", FONT_WHITE, -0.5, 0.9, 0.002, 0.06, 1, 0, mavlink_param_save, 1.0);
	draw_button(esContext, "upload", VIEW_MODE_FCMENU, "[UPLOAD ALL]", FONT_WHITE, 0.0, 0.9, 0.002, 0.06, 1, 0, mavlink_param_upload_all, 1.0);
	draw_button(esContext, "flash_r", VIEW_MODE_FCMENU, "[LOAD FLASH]", FONT_WHITE, 0.5, 0.9, 0.002, 0.06, 1, 0, mavlink_flashload, 0.0);
	draw_button(esContext, "flash_w", VIEW_MODE_FCMENU, "[WRITE FLASH]", FONT_WHITE, 1.0, 0.9, 0.002, 0.06, 1, 0, mavlink_flash, 0.0);

	if (flag == 0) {
		draw_text_f(esContext, -0.4, 0.0, 0.05, 0.05, FONT_BLACK_BG, "No Mavlink-Parameters found");
	}

	screen_keyboard(esContext);
	screen_filesystem(esContext);
}

