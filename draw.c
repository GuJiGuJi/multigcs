
#ifdef SDLGL
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <png.h>
#ifndef OSX
#define NO_SDL_GLEXT
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_events.h>
#include <SDL_image.h>

#include <model.h>
#include <userdata.h>

#define NO_SDL_GLEXT
#include <SDL_opengl.h>
#include <gl_draw.h>

#else

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <png.h>
#include <ctype.h>
#include <termios.h>

#include <model.h>
#include <userdata.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "esUtil.h"
#include <gles_draw.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_image.h>

#endif

#include <draw.h>
#include <main.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
Uint32 rmask = 0xff000000;
Uint32 gmask = 0x00ff0000;
Uint32 bmask = 0x0000ff00;
Uint32 amask = 0x000000ff;
#else
Uint32 rmask = 0x000000ff;
Uint32 gmask = 0x0000ff00;
Uint32 bmask = 0x00ff0000;
Uint32 amask = 0xff000000;
#endif

SrtmCache AltCache[MAX_ALTCACHE];
const GLfloat DEG2RAD = 3.14159 / 180.0;

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#define GL_RGBA	0x1908
#define GL_BGR 0x80E0
#define GL_RGB 0x1907
#endif

void draw_scrollbar (ESContext *esContext, uint16_t page, uint16_t page_max, uint8_t (*callback) (char *, float, float, int8_t, float)) {
#ifdef CONSOLE_ONLY
	return;
#endif
	if (page_max > 0) {
		draw_box_f3(esContext, 1.3, -0.77, 0.002, 1.35, 0.77, 0.002, 255, 255, 255, 128);
		if (page > 0) {
			set_button("down", view_mode, 1.3, -0.77, 1.35, -0.67, callback, -1.0, 0);
			set_button("down2", view_mode, 1.3, -0.77, 1.35, 0.0, callback, -3.0, 0);
		}
		draw_box_f3(esContext, 1.3, -0.77, 0.002, 1.35, -0.67, 0.002, 255, 255, 255, 128);
		draw_box_f3(esContext, 1.3, -0.77, 0.002, 1.35, -0.67, 0.002, 255, 255, 255, 128);
		draw_rect_f3(esContext, 1.3, -0.77, 0.002, 1.35, -0.67, 0.002, 255, 255, 255, 128);
		if (page < page_max) {
			set_button("up", view_mode, 1.3, 0.67, 1.35, 0.77, callback, 1.0, 0);
			set_button("up2", view_mode, 1.3, 0.0, 1.35, 0.77, callback, 3.0, 0);
		}
		if (page < 0) {
			page = 0;
		}
		if (page > page_max) {
			page = page_max;
		}
		draw_box_f3(esContext, 1.3, 0.67, 0.002, 1.35, 0.77, 0.002, 255, 255, 255, 128);
		draw_box_f3(esContext, 1.3, 0.77, 0.002, 1.35, 0.67, 0.002, 255, 255, 255, 128);
		draw_rect_f3(esContext, 1.3, 0.77, 0.002, 1.35, 0.67, 0.002, 255, 255, 255, 128);
		draw_box_f3(esContext, 1.3 - 0.01, -0.67 + (1.34) * (float)(page) / (float)(page_max) - 0.02, 0.002, 1.35 + 0.01, -0.67 + (1.34) * (float)(page) / (float)(page_max), 0.002, 255, 255, 255, 200);
		draw_rect_f3(esContext, 1.3 - 0.01, -0.67 + (1.34) * (float)(page) / (float)(page_max) - 0.02, 0.002, 1.35 + 0.01, -0.67 + (1.34) * (float)(page) / (float)(page_max), 0.002, 100, 100, 100, 128);
	}
}

void draw_title (ESContext *esContext, char *text) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_text_f3(esContext, 0.0 - strlen(text) * 0.06 * 0.6 / 2.0 - 0.012, -0.95, 0.02, 0.06, 0.06, FONT_GREEN, text);
}

uint8_t draw_button (ESContext *esContext, char *name, uint8_t view_mode, char *text, char *font, float x, float y, float z, float h, uint8_t align_x, uint8_t align_y, uint8_t (*callback) (char *, float, float, int8_t, float), float data) {
#ifdef CONSOLE_ONLY
	return 0;
#endif
	uint16_t n = 0;
	float x1 = x - strlen(text) * h * 0.6 / 2.0 - 0.012;
	float y1 = y;
	float x2 = x + strlen(text) * h * 0.6 / 2.0 + 0.012;
	float y2 = y + h;
	float z1 = z;
	if (align_x == ALIGN_CENTER) {
		x1 = x - strlen(text) * h * 0.6 / 2.0 - 0.012;
		x2 = x + strlen(text) * h * 0.6 / 2.0 + 0.012;
	} else if (align_x == ALIGN_RIGHT) {
		x1 = x - strlen(text) * h * 0.6 - 0.012;
		x2 = x;
	} else if (align_x == ALIGN_LEFT) {
		x1 = x;
		x2 = x + strlen(text) * h * 0.6 + 0.02;
	}
	if (align_y == ALIGN_CENTER) {
		y1 = y - h / 2;
		y2 = y + h / 2;
	}
	draw_text_f3(esContext, x1, y1, z1, h, h, font, text);
//	draw_rect_f3(esContext, x1 - 0.01, y1 - 0.01, z1, x2 + 0.01, y2 + 0.01, z1, 255, 255, 255, 64);
	for (n = 0; n < MAX_BUTTONS; n++) {
		if (strcmp(Buttons[n].name, name) == 0) {
			Buttons[n].view_mode = view_mode;
			Buttons[n].x1 = x1 - 0.01;
			Buttons[n].y1 = y1 - 0.01;
			Buttons[n].x2 = x2 + 0.01;
			Buttons[n].y2 = y2 + 0.01;
			Buttons[n].data = data;
			Buttons[n].callback = callback;
			Buttons[n].type = 0;
			return 0;
		} else if (Buttons[n].name[0] == 0) {
			strcpy(Buttons[n].name, name);
			Buttons[n].view_mode = view_mode;
			Buttons[n].x1 = x1 - 0.01;
			Buttons[n].y1 = y1 - 0.01;
			Buttons[n].x2 = x2 + 0.01;
			Buttons[n].y2 = y2 + 0.01;
			Buttons[n].data = data;
			Buttons[n].callback = callback;
			Buttons[n].type = 0;
			return 1;
		}
	}
	return 2;
}

uint8_t draw_image_button (ESContext *esContext, char *name, uint8_t view_mode, char *image, float x, float y, float z, float w, float h, uint8_t align_x, uint8_t align_y, uint8_t (*callback) (char *, float, float, int8_t, float), float data) {
#ifdef CONSOLE_ONLY
	return 0;
#endif

char text[1024];
char font[1024];
strcpy(text, "img");
strcpy(font, FONT_GREEN);

	uint16_t n = 0;
	float x1 = x - w / 2.0;
	float y1 = y;
	float x2 = x + w / 2.0;
	float y2 = y + h;
	if (align_x == ALIGN_CENTER) {
		x1 = x - w / 2.0;
		x2 = x + w / 2.0;
	} else if (align_x == ALIGN_RIGHT) {
		x1 = x - w;
		x2 = x;
	} else if (align_x == ALIGN_LEFT) {
		x1 = x;
		x2 = x + w;
	}
	if (align_y == ALIGN_CENTER) {
		y1 = y - h / 2;
		y2 = y + h / 2;
	}

	draw_image_f3(esContext, x1, y1, x2, y2, 0.0, image);
//	draw_rect_f3(esContext, x1 - 0.01, y1 - 0.01, z1, x2 + 0.01, y2 + 0.01, z1, 255, 0, 0, 255);
	for (n = 0; n < MAX_BUTTONS; n++) {
		if (strcmp(Buttons[n].name, name) == 0) {
			Buttons[n].view_mode = view_mode;
			Buttons[n].x1 = x1 - 0.01;
			Buttons[n].y1 = y1 - 0.01;
			Buttons[n].x2 = x2 + 0.01;
			Buttons[n].y2 = y2 + 0.01;
			Buttons[n].data = data;
			Buttons[n].callback = callback;
			Buttons[n].type = 0;
			return 0;
		} else if (Buttons[n].name[0] == 0) {
			strcpy(Buttons[n].name, name);
			Buttons[n].view_mode = view_mode;
			Buttons[n].x1 = x1 - 0.01;
			Buttons[n].y1 = y1 - 0.01;
			Buttons[n].x2 = x2 + 0.01;
			Buttons[n].y2 = y2 + 0.01;
			Buttons[n].data = data;
			Buttons[n].callback = callback;
			Buttons[n].type = 0;
			return 1;
		}
	}
	return 2;
}

int next_power_of_two (int n) {
	double logbase2 = log((double) n) / log(2.0);
	return (int)(pow(2, ceil(logbase2)) + 0.5);
}

SDL_Surface* convert_to_power_of_two (SDL_Surface* surface) {
	int width = next_power_of_two(surface->w);
	int height = next_power_of_two(surface->h);
	SDL_Surface* pot_surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_Rect dstrect;
	dstrect.w = surface->w;
	dstrect.h = surface->h;
	dstrect.x = 0;
	dstrect.y = 0;
	SDL_BlitSurface(surface, NULL, pot_surface, &dstrect);
	SDL_FreeSurface(surface);
	return pot_surface;
}

int imagefile_exists (char *fileName) {
	struct stat buf;
	int i = stat ( fileName, &buf );
	if ( i == 0 ) {
		return 1;
	}
	return 0;
}

int loadImage(const char *filename) {
	if (strstr(filename, ".png\0") > 0) {
		return loadPNG(filename);
	}
#ifdef RPI_NO_X
	char tmp_file[1024];
	sprintf(tmp_file, "%s.png", filename);
	if (imagefile_exists(tmp_file) == 0) {
		char tmp_cmd[2048];
		sprintf(tmp_cmd, "convert \"%s\" \"%s\"", filename, tmp_file);
		system(tmp_cmd);
	}
	return loadPNG(tmp_file);
#endif
	SDL_Surface *imageSurface1 = IMG_Load(filename);
	if (! imageSurface1) {
		printf("Error: loading image: %s\n", filename);
		return 0;
	}
	SDL_Surface *imageSurface = convert_to_power_of_two(imageSurface1);
	GLuint texture;
	GLenum texture_format;
	GLint  nOfColors;
        nOfColors = imageSurface->format->BytesPerPixel;
        if (nOfColors == 4) {
                if (imageSurface->format->Rmask == 0x000000ff) {
                        texture_format = GL_RGBA;
                } else {
                        texture_format = GL_BGRA;
		}
        } else if (nOfColors == 3) {
                if (imageSurface->format->Rmask == 0x000000ff) {
                        texture_format = GL_RGB;
                } else {
                        texture_format = GL_BGR;
                }
        } else {
                printf("warning: the image is not truecolor..  this will probably break\n");
		return 0;
        }
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

#ifdef SDLGL
	glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, imageSurface->w, imageSurface->h, 0, texture_format, GL_UNSIGNED_BYTE, imageSurface->pixels);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, texture_format, imageSurface->w, imageSurface->h, 0, texture_format, GL_UNSIGNED_BYTE, imageSurface->pixels);
#endif

	SDL_FreeSurface(imageSurface);
	imageSurface = NULL;
	return texture;
}

int loadPNG(const char *filename) {
    GLuint texture;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep *row_pointers = NULL;
    int bitDepth, colourType;
    FILE *pngFile = fopen(filename, "rb");
    if (!pngFile)
        return 0;
    png_byte sig[8];
    fread(&sig, 8, sizeof(png_byte), pngFile);
    rewind(pngFile);
    if (!png_check_sig(sig, 8)) {
        //printf("png sig failure\n");
        return 0;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        //printf("png ptr not created\n");
        return 0;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        //printf("set jmp failed\n");
        return 0;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        //printf("cant get png info ptr\n");
        return 0;
    }
    png_init_io(png_ptr, pngFile);
    png_read_info(png_ptr, info_ptr);
    bitDepth = png_get_bit_depth(png_ptr, info_ptr);
    colourType = png_get_color_type(png_ptr, info_ptr);
    if (colourType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (colourType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        //png_set_gray_1_2_4_to_8(png_ptr);
        png_set_expand_gray_1_2_4_to_8(png_ptr);  // thanks to Jesse Jaara for bug fix for newer libpng...
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (bitDepth == 16)
        png_set_strip_16(png_ptr);
    else if (bitDepth < 8)
        png_set_packing(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colourType, NULL, NULL, NULL);

    int components;		// = GetTextureInfo(colourType);
    switch (colourType) {
    case PNG_COLOR_TYPE_GRAY:
        components = 1;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        components = 2;
        break;
    case PNG_COLOR_TYPE_RGB:
        components = 3;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        components = 4;
        break;
    default:
        components = -1;
    }

    if (components == -1) {
        if (png_ptr)
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        //printf("%s broken?\n", filename);
        return 0;
    }

    GLubyte *pixels =
        (GLubyte *) malloc(sizeof(GLubyte) * (width * height * components));
    row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);

    int i = 0;
    for (i = 0; i < height; ++i) {
        row_pointers[i] = (png_bytep) (pixels + (i * width * components));
    }
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLuint glcolours;
    (components == 4) ? (glcolours = GL_RGBA) : (0);
    (components == 3) ? (glcolours = GL_RGB) : (0);
    (components == 2) ? (glcolours = GL_LUMINANCE_ALPHA) : (0);
    (components == 1) ? (glcolours = GL_LUMINANCE) : (0);

    //printf("%s has %i colour components\n",filename,components);
    //glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, glcolours, GL_UNSIGNED_BYTE, pixels);
    glTexImage2D(GL_TEXTURE_2D, 0, glcolours, width, height, 0, glcolours, GL_UNSIGNED_BYTE, pixels);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    free(row_pointers);
    free(pixels);
    return texture;
}

void draw_image_uncache (char *file) {
#ifdef CONSOLE_ONLY
	return;
#endif
	int16_t n = 0;
	int16_t tex_num = -1;
	for (n = 0; n < MAX_TEXCACHE; n++) {
		if (strcmp(TexCache[n].name, file) == 0) {
			tex_num = n;
			printf("remove image %s from cache %i (%i)\n", TexCache[tex_num].name, tex_num, TexCache[tex_num].atime);
			glDeleteTextures( 1, &TexCache[tex_num].texture );
			TexCache[tex_num].name[0] = 0;
			TexCache[tex_num].texture = -1;
			break;
		}
	}
}



void draw_line_f (ESContext *esContext, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_line_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}

void draw_line (ESContext *esContext, int16_t px1, int16_t py1, int16_t px2, int16_t py2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	GLfloat x1 = (float)px1 / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	GLfloat y1 = (float)py1 / (float)esContext->height * 2.0 - 1.0;
	GLfloat x2 = (float)px2 / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	GLfloat y2 = (float)py2 / (float)esContext->height * 2.0 - 1.0;
	draw_line_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}


void draw_circle_f (ESContext *esContext, float x1, float y1, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_circle_f3(esContext, x1, y1, 0.0, radius, r, g, b, a);
}

void draw_circle (ESContext *esContext, int16_t x, int16_t y, int16_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	GLfloat x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	GLfloat y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	GLfloat radius1 = (float)radius / (float)esContext->width * 2.0 * aspect;
	y1 = y1 * -1;
	draw_circle_f3(esContext, x1, y1, 0.0, radius1, r, g, b, a);
}

void draw_circleFilled_f (ESContext *esContext, float x1, float y1, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_circleFilled_f3(esContext, x1, y1, 0.0, radius, r, g, b, a);
}

void draw_circleFilled (ESContext *esContext, int16_t x, int16_t y, int16_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	GLfloat x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	GLfloat y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	GLfloat radius1 = (float)radius / (float)esContext->width * 2.0 * aspect;
	y1 = y1 * -1;
	draw_circleFilled_f3(esContext, x1, y1, 0.0, radius1, r, g, b, a);
}

void draw_rect_f (ESContext *esContext, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_rect_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}

void draw_box_f (ESContext *esContext, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_box_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}

void draw_rect (ESContext *esContext, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	float y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	float x2 = x1 + (float)w / (float)esContext->width * 2.0 * aspect;
	float y2 = y1 + (float)h / (float)esContext->height * 2.0;
	draw_rect_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}

void draw_box (ESContext *esContext, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	float y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	float x2 = x1 + (float)w / (float)esContext->width * 2.0 * aspect;
	float y2 = y1 + (float)h / (float)esContext->height * 2.0;
	draw_box_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, r, g, b, a);
}

void draw_tria_f (ESContext *esContext, float x1, float y1, float x2, float y2, float x3, float y3, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_tria_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, x3, y3, 0.0, r, g, b, a);
}

void draw_triaFilled_f (ESContext *esContext, float x1, float y1, float x2, float y2, float x3, float y3, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_triaFilled_f3(esContext, x1, y1, 0.0, x2, y2, 0.0, x3, y3, 0.0, r, g, b, a);
}

void draw_circleFilled_f3_part_end (ESContext *esContext, float x1, float y1, float z1, float radius, float radius_inner, float start, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef CONSOLE_ONLY
	return;
#endif
	y1 = y1 * -1;
	x1 = x1 - cos(start * DEG2RAD) * (radius_inner + (radius - radius_inner) / 2.0);
	y1 = y1 + sin(start * DEG2RAD) * (radius_inner + (radius - radius_inner) / 2.0);
	if (start > 180.0) {
		draw_circleFilled_f3_part(esContext, x1, -y1, 0.0, (radius - radius_inner) / 2.0, 0.0, 0.0, 360.0, r, g, b, a);
	} else {
		draw_circleFilled_f3_part(esContext, x1, -y1, 0.0, (radius - radius_inner) / 2.0, 0.0, 180.0, 360.0, r, g, b, a);
	}
}


void draw_circleMeter_f3 (ESContext *esContext, float x, float y, float z, float radius, float start1, float start2, float start3, float start4, float value, char *text, char *text2, uint8_t type) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float th = radius / 3.5;
	float width = radius / 15.0;
	if (type == 2) {
		width = 0.015;
	}
	float offset1 = start1;
	float offset2 = start4;
	start2 = (offset2 - offset1) * start2 / 100.0 + offset1;
	start3 = (offset2 - offset1) * start3 / 100.0 + offset1;
	value = (offset2 - offset1) * value / 100.0 + offset1;
	if (type == 2) {
		if (contrast == 1) {
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset1, 255, 255, 255, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, offset1, start2, 255, 255, 255, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start2, start3, 255, 255, 0, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start3, offset2, 255, 0, 0, 255);
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset2, 255, 0, 0, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, 0.0, radius - width, offset1, value, 100, 100, 255, 255);
			draw_circlePointer_f3(esContext, x, y, z + 0.0001, radius - width, radius / 20.0, value, 255, 255, 255, 255);
		} else {
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset1, 227, 227, 227, 200);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, offset1, start2, 227, 227, 227, 200);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start2, start3, 255, 255, 0, 200);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start3, offset2, 255, 0, 0, 200);
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset2, 255, 0, 0, 200);
			draw_circleFilled_f3_part(esContext, x, y, z, 0.0, radius - width, offset1, value, 100, 100, 255, 127);
			draw_circlePointer_f3(esContext, x, y, z + 0.0001, radius - width, radius / 20.0, value, 255, 255, 255, 255);
		}
	} else {
		if (contrast == 1) {
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset1, 127, 127, 127, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, offset1, start2, 127, 127, 127, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start2, start3, 200, 200, 200, 255);
			draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start3, offset2, 255, 255, 255, 255);
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset2, 10, 200, 10, 255);
			draw_circlePointer_f3(esContext, x, y, z + 0.0001, radius - width, radius / 20.0, value, 255, 255, 255, 255);
		} else {
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset1, 200, 10, 10, 255);
			if (type == 3) {
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, offset1, start2, 10, 220, 10, 255);
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start2, start3, 220, 220, 10, 255);
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start3, offset2, 220, 10, 10, 255);
			} else {
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, offset1, start2, 220, 10, 10, 255);
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start2, start3, 220, 220, 10, 255);
				draw_circleFilled_f3_part(esContext, x, y, z, radius, radius - width, start3, offset2, 10, 220, 10, 255);
			}
	//		draw_circleFilled_f3_part_end(esContext, x, y, z, radius, radius - width, offset2, 10, 200, 10, 255);
			draw_circlePointer_f3(esContext, x, y, z + 0.0001, radius - width, radius / 20.0, value, 255, 255, 255, 255);
		}
	}
	if (strlen(text) > 0 || strlen(text2) > 0) {
		if (type == 1) {
			draw_text_f3(esContext, x + radius - width - strlen(text) * th * 0.6, y + (radius - width) / 2.0, z + 0.001, th, th, FONT_WHITE, text);
			draw_text_f3(esContext, x + radius - width - strlen(text2) * th * 0.6, y + th + (radius - width) / 2.0, z + 0.001, th, th, FONT_WHITE, text2);
		} else if (type == 2) {
			draw_box_f3(esContext, x - radius / 5 * 3, y - th / 2.0, z + 0.0011, x + radius / 5 * 3, y + th * 3.0 / 2.0, z + 0.0011, 0, 0, 0, 127);
			draw_rect_f3(esContext, x - radius / 5 * 3, y - th / 2.0, z + 0.0015, x + radius / 5 * 3, y + th * 3.0 / 2.0, z + 0.0015, 255, 255, 255, 255);
			draw_text_f3(esContext, x - strlen(text) * th * 0.6 / 2.0, y - th / 2.0, z + 0.0015, th, th, FONT_WHITE, text);
			draw_text_f3(esContext, x - strlen(text2) * th * 0.6 / 2.0, y + th / 2.0, z + 0.0015, th, th, FONT_WHITE, text2);
		} else {
			draw_text_f3(esContext, x - strlen(text) * th * 0.6 / 2, y + th / 2.0, z + 0.001, th, th, FONT_WHITE, text);
			draw_text_f3(esContext, x - strlen(text2) * th * 0.6 / 2, y + th / 2.0 + th, z + 0.001, th, th, FONT_WHITE, text2);
		}
	}
}

void draw_image_f (ESContext *esContext, float x1, float y1, float x2, float y2, char *file) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_image_f3(esContext, x1, y1, x2, y2, 0.0, file);
}

void draw_image (ESContext *esContext, int16_t x, int16_t y, int16_t w, int16_t h, char *file) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	float y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	float x2 = x1 + (float)w / (float)esContext->width * 2.0 * aspect;
	float y2 = y1 + (float)h / (float)esContext->height * 2.0;
	draw_image_f3(esContext, x1, y1, x2, y2, 0.0, file);
}

#ifdef SDLGL
uint32_t getpixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
} 
#endif 

int16_t get_altitude (float lat, float lon) {
	char file[100];
	int8_t n = 0;
	int8_t alt_num = -1;
	int8_t old_num = -1;
	uint32_t atime_min = time(0);
	int16_t px = 0;
	int16_t py = 0;
	int8_t lat_m = (int)lat;
	int8_t lon_m = (int)lon;
	int8_t flag = 0;
	if (lat_m == 0 || lon_m == 0) {
		return 0;
	}
	sprintf(file, "%s/.multigcs/MAPS/N%02iE%03i.hgt", getenv("HOME"), lat_m, lon_m);
	for (n = 0; n < 8; n++) {
		if (strcmp(AltCache[n].name, file) == 0) {
			alt_num = n;
		} else if (AltCache[n].atime < atime_min) {
			old_num = n;
			atime_min = AltCache[n].atime;
		}
	}
	if (alt_num == -1) {
		for (n = 0; n < MAX_ALTCACHE; n++) {
			if (AltCache[n].name[0] == 0) {
				alt_num = n;
				break;
			}
		}
		if (old_num == -1) {
			old_num = 0;
		}
		if (alt_num == -1) {
			alt_num = old_num;
			printf("remove srtm %s from cache %i (%i)\n", AltCache[alt_num].name, old_num, AltCache[alt_num].atime);
//			Delete;
			AltCache[alt_num].name[0] = 0;
		}
		if (alt_num != -1) {
//			printf("loading srtm %s in to alt-cache %i %i\n", file, alt_num, AltCache[alt_num].atime);
			FILE *fr;
			fr = fopen(file, "r");
			if (fr != 0) {
				strcpy(AltCache[alt_num].name, file);
				AltCache[alt_num].atime = time(0);
				for (py = 0; py < 1201; py++) {
					for (px = 0; px < 1201; px++) {
						uint8_t val1 = 0;
						uint8_t val2 = 0;
						int16_t val = 0;
				        	fread(&val1, 1, 1, fr);
				        	fread(&val2, 1, 1, fr);
						val = (val1<<8) + val2;
						AltCache[alt_num].data[1200 - py][px] = val;
					}
				}
				fclose(fr);
			} else {
				flag = 1;
			}
		}
	}

	if (flag == 0) {
		AltCache[alt_num].atime = time(0);
//		printf("# using Alt-Cache: %s\n", AltCache[alt_num].name);
		// geting all field-points
		int16_t y1 = (int)((lat - (float)lat_m) * 1201.0);
		int16_t x1 = (int)((lon - (float)lon_m) * 1201.0);

#ifndef SDLGL
		return AltCache[alt_num].data[y1][x1];
#else
		// geting field
		float fy = ((lat - (float)lat_m) * 1201.0);
		float fx = ((lon - (float)lon_m) * 1201.0);
		int16_t y2 = (int)((lat - (float)lat_m) * 1201.0 + 1.0);
		int16_t x2 = (int)((lon - (float)lon_m) * 1201.0 + 1.0);

		// geting alt-values of field-points
		int16_t vx1a = AltCache[alt_num].data[y1][x1];
		int16_t vx2a = AltCache[alt_num].data[y1][x2];
		int16_t vx1b = AltCache[alt_num].data[y2][x1];
		int16_t vx2b = AltCache[alt_num].data[y2][x2];
		if (vx1a == -32768) {
			uint16_t tn = 0;
			for (tn = 0; tn < 10; tn++) {
				vx1a = AltCache[alt_num].data[y1 + tn][x1 + tn];
				if (vx1a != -32768) {
					break;
				}
				vx1a = AltCache[alt_num].data[y1 - tn][x1 - tn];
				if (vx1a != -32768) {
					break;
				}
			}
		}
		if (vx2a == -32768) {
			uint16_t tn = 0;
			for (tn = 0; tn < 10; tn++) {
				vx2a = AltCache[alt_num].data[y1 + tn][x2 + tn];
				if (vx2a != -32768) {
					break;
				}
				vx2a = AltCache[alt_num].data[y1 - tn][x2 - tn];
				if (vx2a != -32768) {
					break;
				}
			}
		}
		if (vx1b == -32768) {
			uint16_t tn = 0;
			for (tn = 0; tn < 10; tn++) {
				vx1b = AltCache[alt_num].data[y2 + tn][x1 + tn];
				if (vx1b != -32768) {
					break;
				}
				vx1b = AltCache[alt_num].data[y2 - tn][x1 - tn];
				if (vx1b != -32768) {
					break;
				}
			}
		}
		if (vx2b == -32768) {
			uint16_t tn = 0;
			for (tn = 0; tn < 10; tn++) {
				vx2b = AltCache[alt_num].data[y2 + tn][x2 + tn];
				if (vx2b != -32768) {
					break;
				}
				vx2b = AltCache[alt_num].data[y2 - tn][x2 - tn];
				if (vx2b != -32768) {
					break;
				}
			}
		}
		// geting alt-values of avarages (x1,y1->x2,y1 and x1,y2->x2,y2)
		int16_t vxa = (float)(vx2a - vx1a) * (fx - (float)x1) + (float)vx1a;
		int16_t vxb = (float)(vx2b - vx1b) * (fx - (float)x1) + (float)vx1b;
		// geting alt-values avarages (xy1->xy2)
		int16_t alt = (vxb - vxa) * (fy - (float)y1) + vxa;
		return alt;
#endif
	}
	return 0;
}

void draw_pointer (ESContext *esContext, int16_t x, int16_t y, int16_t w, int16_t h, char *file) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	float y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	float x2 = x1 + (float)w / (float)esContext->width * 2.0 * aspect;
	float y2 = y1 + (float)h / (float)esContext->height * 2.0;
	draw_image_f3(esContext, x1, y1, x2, y2, 0.0035, file);
}

void draw_text_f3 (ESContext *esContext, float x1, float y1, float z1, float w, float h, char *file, char *text) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_text_f3_fast(esContext, x1, y1, z1, w, h, file, text);
}

void draw_text_f (ESContext *esContext, float x1, float y1, float w, float h, char *file, char *text) {
#ifdef CONSOLE_ONLY
	return;
#endif
	draw_text_f3(esContext, x1, y1, 0.003, w, h, file, text);
}

void draw_text (ESContext *esContext, int16_t x, int16_t y, int16_t w, int16_t h, char *file, char *text) {
#ifdef CONSOLE_ONLY
	return;
#endif
	float x1 = (float)x / (float)esContext->width * 2.0 * aspect - 1.0 * aspect;
	float y1 = (float)y / (float)esContext->height * 2.0 - 1.0;
	float fw = (float)w / 200;
	float fh = (float)h / 200;
	draw_text_f3(esContext, x1, y1, 0.0, fw, fh, file, text);
}
