#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"
#include "video.h"

extern char *video_mem;
extern unsigned h_res;
extern unsigned v_res;
extern unsigned bits_pp;
extern unsigned bytes_pp;
extern vbe_mode_info_t info;
extern char *buffer;

void *(vg_init)(uint16_t mode){
    uint32_t size;
    uint32_t base;
    int r;
    struct minix_mem_range mr;
    reg86_t reg86;


    //get info about mode vg get mode info()
    vbe_get_mode_info(mode, &info);

    //w*l*b
    h_res = info.XResolution;
    v_res = info.YResolution;
    bits_pp = info.BitsPerPixel;

    if(bits_pp % 8 == 0){
        bytes_pp = bits_pp / 8;
    } else {
        bytes_pp = bits_pp / 8 +1;
    }
    size = h_res * v_res * bytes_pp;
    base = info.PhysBasePtr;

    /* Allow memory mapping */
    mr.mr_base = (phys_bytes)base;
    mr.mr_limit = mr.mr_base + size;

    if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr))) {
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);
        return NULL;
    }

    /* Map memory */

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, size);

    if(video_mem == MAP_FAILED) {
        panic("couldn't map video memory");
        return NULL;
    }

    // initialize reg86 to 0

    memset(&reg86, 0, sizeof(reg86));
    reg86.intno = 0x10;
    reg86.ah = 0x4F;
    reg86.al = 0x02;
    reg86.bx = BIT(14) | mode;

    if( sys_int86(&reg86) != OK ) {
        printf("\tsys_int86() failed \n");
        return NULL;
    }

    return (void *)video_mem;
}

int draw_pixel(uint16_t x, uint16_t y, uint32_t color){
    if(x >= h_res || y >= v_res){
        return 1;
    }

    uint8_t col;

    for(unsigned int j = 0; j < bytes_pp; j++){
        if(bits_pp == 8){
            if(color > 255){
                printf("Invalid color!\n");
                return 1;
            } else {
                col = (uint8_t) color;
                *(buffer + (y*h_res + x) * bytes_pp + j) = col;
            }
        } else {
            col = (color >> (j * 8));
            *(buffer + (y*h_res + x) * bytes_pp + j) = col;
        }
    }

    return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
    for (int i = 0; i < len; i++){
        if (draw_pixel(x+i, y, color) == 1){
            printf("Couldn't draw pixel!\n");
            return 1;
        }
    }
    return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){

    if(width > h_res) width = h_res;
    if(height > v_res) height = v_res;

    for (int i = 0; i < height; i++){
        if (vg_draw_hline(x, y+i, width, color) == 1){
            printf("Couldn't draw line!\n");
            return 1;
        }
    }
    return 0;
}

void xmp_draw_line(uint8_t *xpm, uint16_t w, uint16_t x, uint16_t y){
    uint8_t *line = (uint8_t *)(buffer + (x + y*h_res) * bytes_pp);
    uint32_t transparent = xpm_transparency_color(XPM_8_8_8);
    uint32_t col;

    for (int i = 0; i < w; i++){
        col = 0;
        for (unsigned int j = 0; j < bytes_pp; j++) {
            col += ((uint32_t)*xpm) << (8*j);
            xpm++;
        }
        if (col != transparent){
            xpm -= bytes_pp;
            for (unsigned int k = 0; k < bytes_pp; k++){
                *line = *xpm;
                line++;
                xpm++;
            }
        } else {
            line += bytes_pp;
        }
    }
}

void xpm_draw(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t x, uint16_t y){
    for(int i = 0; i < h; i++){
        xmp_draw_line(xpm, w, x, y + i);
        xpm += w*bytes_pp;
    }
}


void xmp_draw_subs_line(uint8_t *xpm, uint16_t w, uint16_t wsubs, uint16_t x, uint16_t y){
    uint8_t *line = (uint8_t *)(buffer + (x + y*h_res) * bytes_pp);
    uint32_t transparent = xpm_transparency_color(XPM_8_8_8);
    uint32_t col;

    for (int i = x; i < x+w; i++){
        col = 0;
        for (unsigned int j = 0; j < bytes_pp; j++) {
            col += ((uint32_t)*xpm) << (8*j);
            xpm++;
        }
        if (col != transparent){
            xpm -= bytes_pp;
            for (unsigned int k = 0; k < bytes_pp; k++){
                *line = *xpm;
                line++;
                xpm++;
            }
        } else {
            line += bytes_pp;
        }
    }
}

void xpm_subs(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t wsubs, uint16_t hsubs, uint16_t x, uint16_t y){
    xpm+=(x+y*h_res)*bytes_pp;
    for(int i = 0; i < hsubs; i++){
        xmp_draw_subs_line(xpm, w, wsubs, x, y + i);
        xpm += w*bytes_pp;
    }
}
