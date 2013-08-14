
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <userdata.h>
#include <serial.h>
#include <model.h>
#include <main.h>
#include <SDL.h>

static SDL_Thread *sdl_thread_serial_frsky = NULL;
static uint8_t buffer[102];
static uint8_t buffer_ptr = 0;
static uint8_t buffer_user[102];
static uint8_t buffer_user_ptr = 0;
static uint8_t last = 0;
static uint8_t new = 0;
static uint8_t type = 0;
static uint8_t mode = 0;
static int serial_fd_frsky = -1;
static uint8_t frsky_thread_running = 0;
static uint32_t last_connection = 1;

static uint8_t GPS_TIME_Day = 0;
static uint8_t GPS_TIME_Month = 0;
static uint8_t GPS_TIME_Year = 0;
static uint8_t GPS_TIME_Houre = 0;
static uint8_t GPS_TIME_Minute = 0;
static uint8_t GPS_TIME_Second = 0;

static int8_t GPS_found = 0;
static uint16_t GPS_lat_d = 0;
static uint16_t GPS_lat_m = 0;
static uint16_t GPS_long_d = 0;
static uint16_t GPS_long_m = 0;
static uint16_t GPS_speed = 0;
static uint16_t GPS_dir = 0;
static int16_t GPS_alt = 0;
static int16_t BARO_alt = 0;

uint8_t frsky_connection_status (void) {
	if (serial_fd_frsky == -1) {
		return 0;
	}
	return last_connection;
}


int thread_serial_frsky (void *unused) {
	uint8_t n = 0;
	uint8_t nn = 0;
	uint8_t start = 0;
	uint8_t read_buffer[201];
	uint8_t read_num = 0;
	uint8_t byte_stuffing = 0;
	uint8_t byte_stuffing_user = 0;
	while (gui_running == 1 && frsky_thread_running == 1) {
		while ((read_num = read(serial_fd_frsky, read_buffer, 200)) > 0) {
			for (nn = 0; nn < read_num; nn++) {
				new = read_buffer[nn];
//				printf("##%i: %i (0x%x)\n", buffer_ptr, new, new);
				if (start == 1) {
					type = new;
					start = 0;
				} else if (last == 0x7e && new == 0x7e) {
					buffer_ptr = 0;
					start = 1;
				} else if (type != 0 && new == 0x7e && buffer_ptr > 0) {
					ModelData.heartbeat_rc = 100;
					ModelData.found_rc = 1;
					last_connection = time(0);
					if (type == 0xfe) {
//						printf("# remote voltage and link quality (len=%i) #\n", buffer_ptr);
//						printf("	RX-Battery   %0.1f V\n", (float)buffer[0] * 3.3 / 255.0 * 4.0);
//						printf("	Analog-IN    %0.2f V\n", (float)buffer[1] * 3.3 / 255.0);
//						printf("	Strom        %0.1f A\n", ((float)buffer[1] - 129.0) * 100.0 / 126.0);
//						printf("	LQI-UpLink   %i %%\n", (buffer[2] - 40) * 100 / 70);
//						printf("	LQI-DownLink %i %%\n", buffer[3] / 2);
						ModelData.rssi_rx = (buffer[2] - 40) * 100 / 70;
						ModelData.rssi_tx = buffer[3] / 2;
						ModelData.voltage_rx = (float)buffer[0] * 3.3 / 255.0 * 4.0;
						ModelData.ampere = ((float)buffer[1] - 129.0) * 100.0 / 126.0;
						redraw_flag = 1;
					} else if (type == 0xfd) {
//						printf("# user data (len=%i) #\n", buffer[0]);
						for (n = 0; n < buffer[0]; n++) {
							if (buffer[2 + n] == 0x5e && buffer_user_ptr < 4) {
								buffer_user_ptr = 0;
							} else if (buffer[2 + n] == 0x5e) {
/*
								printf("test %i\n", buffer_user_ptr);
								printf(" >> 0x%x\n", buffer_user[2]);
								printf(" >> 0x%x\n", buffer_user[3]);
*/
								uint8_t i = 0;
								if (buffer_user[1] == 0x06) {
									float volt = ((((buffer_user[2] & 0x0f)<<8) + (buffer_user[3] & 0xff)) & 0x0fff) * 0.002;
									uint16_t cell = buffer_user[2]>>4;
									ModelData.voltage_zell[cell] = volt;
									redraw_flag = 1;
//									printf("############## Voltage: %i = %0.2f Volt\n", cell, volt);
								} else if (buffer_user[1] == 0x03) {
									uint16_t rpm = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									printf("############## RPM: %i\n", rpm);
								} else if (buffer_user[1] == 0x02) {
									int16_t temp1 = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									//printf("############## Temp1: %i\n", temp1 * 280 / 0xffff);
									ModelData.temperature[0] = temp1;
									redraw_flag = 1;
								} else if (buffer_user[1] == 0x05) {
									int16_t temp2 = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									//printf("############## Temp2: %i\n", temp2 * 280 / 0xffff);
									ModelData.temperature[1] = temp2;
									redraw_flag = 1;
								} else if (buffer_user[1] == 0x04) {
//									printf("############## Fuel: \n");
								} else if (buffer_user[1] == 0x10) {
									BARO_alt = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									printf("############## Altx.0: %i\n", BARO_alt);
								} else if (buffer_user[1] == 0x21) {
									uint16_t alt = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1) {
										ModelData.baro = (float)BARO_alt + (float)alt / 100;
										if (GPS_found == 0) {
											ModelData.p_alt = (float)BARO_alt + (float)alt / 100;
										}
										redraw_flag = 1;
									}
									printf("############## Altx.0: %i\n", alt);
								} else if (buffer_user[1] == 0x24) {
									int16_t acc_x = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									printf("############## Acc-x: %f\n", (float)acc_x / 100.0);
								} else if (buffer_user[1] == 0x25) {
									int16_t acc_y = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									printf("############## Acc-y: %f\n", (float)acc_y / 100.0);
								} else if (buffer_user[1] == 0x26) {
									int16_t acc_z = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									printf("############## Acc-z: %f\n", (float)acc_z / 100.0);
								} else if (buffer_user[1] == 0x01) {
									GPS_alt = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
								} else if (buffer_user[1] == 0x01 + 8) {
									uint16_t gps_alt = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1) {
										ModelData.p_alt = (float)GPS_alt + (float)gps_alt / 100;
										redraw_flag = 1;
									}
								} else if (buffer_user[1] == 0x11) {
									GPS_speed = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
								} else if (buffer_user[1] == 0x11 + 8) {
									uint16_t gps_speed = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1) {
										ModelData.speed = (float)GPS_speed + (float)gps_speed / 100;
										redraw_flag = 1;
									}
								} else if (buffer_user[1] == 0x12) {
									uint16_t gps_long = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									GPS_long_d = gps_long / 100;
									GPS_long_m = (gps_long % 100);
								} else if (buffer_user[1] == 0x12 + 8) {
									uint16_t gps_long = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1 && GPS_long_m != 0) {
										ModelData.p_long = ((float)GPS_long_m + (float)gps_long / 10000.0) / 60.0 + (float)GPS_long_d;
										GPS_found = 1;
										redraw_flag = 1;
									}
								} else if (buffer_user[1] == 0x13) {
									uint16_t gps_lat = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									GPS_lat_d = gps_lat / 100;
									GPS_lat_m = (gps_lat % 100);
								} else if (buffer_user[1] == 0x13 + 8) {
									uint16_t gps_lat = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1 && GPS_lat_m != 0) {
										ModelData.p_lat = ((float)GPS_lat_m + (float)gps_lat / 10000.0) / 60.0 + (float)GPS_lat_d;
										GPS_found = 1;
										redraw_flag = 1;
									}
								} else if (buffer_user[1] == 0x14) {
									GPS_dir = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
								} else if (buffer_user[1] == 0x14 + 8) {
									uint16_t gps_curse = (buffer_user[3]<<8) + (buffer_user[2] & 0xff);
									if (mode == 1) {
										ModelData.yaw = (float)GPS_dir + (float)gps_curse / 100;
										redraw_flag = 1;
									}
								} else if (buffer_user[1] == 0x15) {
									GPS_TIME_Day = buffer_user[2];
									GPS_TIME_Month = buffer_user[3];
								} else if (buffer_user[1] == 0x16) {
									GPS_TIME_Year = buffer_user[2];
								} else if (buffer_user[1] == 0x17) {
									GPS_TIME_Houre = buffer_user[2];
									GPS_TIME_Minute = buffer_user[3];
								} else if (buffer_user[1] == 0x18) {
									GPS_TIME_Second = buffer_user[2];
									if (GPS_TIME_Year > 11 && GPS_TIME_Second > 0 && GPS_TIME_Day > 0) {
										char tmp_str[100];
										sprintf(tmp_str, "date -s \"%02i.%02i.20%02i %02i:%02i:%02i\"", GPS_TIME_Day, GPS_TIME_Month, GPS_TIME_Year, GPS_TIME_Houre, GPS_TIME_Minute, GPS_TIME_Second);
										printf("############## GPS_Time: %02i.%02i.20%02i %02i:%02i:%02i\n", GPS_TIME_Day, GPS_TIME_Month, GPS_TIME_Year, GPS_TIME_Houre, GPS_TIME_Minute, GPS_TIME_Second);
									}
								} else if (buffer_user[1] == 0x22) {
									// East/West
								} else if (buffer_user[1] == 0x23) {
									// North/South
								} else {
									printf("(0x%x - %i)\n", buffer_user[1], buffer_user[1]);
									for (i = 2; i < buffer_user_ptr; i++) {
										printf("	(0x%x - %i - %c) --- %i\n", buffer_user[i], buffer_user[i], buffer_user[i], i);
									}
								}
								buffer_user_ptr = 0;
							}
							if (byte_stuffing_user == 1) {
								byte_stuffing_user = 0;
								buffer_user[buffer_user_ptr++] = buffer[2 + n] ^ 0x60;
							} else if (buffer[2 + n] == 0x5d) {
								byte_stuffing_user = 1;
							} else {
								buffer_user[buffer_user_ptr++] = buffer[2 + n];
							}
							if (buffer_user_ptr > 30) {
								printf("# user-buffer overflow / reset #\n");
								buffer_user_ptr = 0;
							}
						}
					} else {
						printf("# unknown data (type=%x; len=%i) #\n", type, buffer_ptr);
					}
					type = 0;
					buffer_ptr = 0;
				} else if (type != 0) {
					if (buffer_ptr < 100) {
						if (byte_stuffing == 1) {
							byte_stuffing = 0;
							buffer[buffer_ptr++] = new ^ 0x20;
						} else if (new == 0x7D) {
							byte_stuffing = 1;
						} else {
							buffer[buffer_ptr++] = new;
						}
					}
				}
				last = new;
			}
		}
		usleep(1000000);
	}
	printf("** exit thread frsky\n");
	return 0;
}

uint8_t frsky_mode_get (void) {
	return mode;
}

void frsky_mode (uint8_t new_mode) {
	mode = new_mode;
	if (mode == 1) {
		printf("** frsky extented telemtry\n");
	} else {
		printf("** frsky normal telemtry\n");
	}
}

uint8_t frsky_init (char *port, uint32_t baud) {
	frsky_thread_running = 1;
	printf("init frsky serial port...\n");
	serial_fd_frsky = serial_open(port, baud);
	if (serial_fd_frsky != -1) {
#ifdef SDL2
		sdl_thread_serial_frsky = SDL_CreateThread(thread_serial_frsky, NULL, NULL);
#else
		sdl_thread_serial_frsky = SDL_CreateThread(thread_serial_frsky, NULL);
#endif
		if ( sdl_thread_serial_frsky == NULL ) {
			fprintf(stderr, "Unable to create thread_serial_frsky: %s\n", SDL_GetError());
			return 1;
		}
	}
	return 0;
}

void frsky_exit (void) {
	if ( sdl_thread_serial_frsky != NULL ) {
		SDL_WaitThread(sdl_thread_serial_frsky, NULL);
		sdl_thread_serial_frsky = NULL;
	}
	serial_close(serial_fd_frsky);
}



