#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"


extern uint8_t code;
extern int hook_kbd;
extern int mouse_hook;
extern int hook_timer;
extern int time_elapsed;
extern char *video_mem;
extern unsigned h_res;
extern unsigned v_res;
extern unsigned bits_pp;
extern unsigned bytes_pp;
extern vbe_mode_info_t info;



int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
    *lsb = (uint8_t) val;
    return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
    val >>= 8;
    *msb = (uint8_t) val;
    return 0;
}

int new_sys_inb(port_t port, uint8_t *byte) {
	uint32_t byte32;
	if(sys_inb(port, &byte32)) {
        printf("Couldn't read from port %x!\n",port);
        return 1;
    }
	*byte = (uint8_t)byte32;
	return 0;
}

int (kbd_subscribe)(uint8_t *bit_no) {
    hook_kbd = KBD_IRQ;
    *bit_no = hook_kbd;
    if(sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_kbd) == 0){
        return 0;
    }
    return 1;
}

int (unsubscribe_kbd)() {

    if (sys_irqrmpolicy(&hook_kbd) != 0) {
        return 1;
    }

    return 0;
}

void (kbc_ih)(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);
    if  (status & OBF) {
        if ((status & (PAR_ERR | TO_ERR))){
            return;
        }
        new_sys_inb(OUT_BUF, &code);
    }
}

int kbd_send_cmd_to_read(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if (sys_outb(KBC_CMD_REG, R_CMD)) {
        return 1;
    }
    return 0;
}

int kbd_read_cmd(uint8_t *cmd){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(!(status & OBF)){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (new_sys_inb(OUT_BUF, cmd)) {
        return 1;
    }
    return 0;
}

int kbd_send_cmd_to_write(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (sys_outb(KBC_CMD_REG, W_CMD)) {
        return 1;
    }
    return 0;
}

int kbd_write_cmd(port_t port, uint8_t cmd){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (sys_outb(port, cmd)) {
        return 1;
    }
    return 0;
}

int enable_kbd(){
    uint8_t cmd = 0;
    if (kbd_send_cmd_to_read()) return 1;    // sends command to read command byte
    if (kbd_read_cmd(&cmd)) return 1;     // reads command
    cmd |= BIT(0);
    if (kbd_send_cmd_to_write()) return 1;    // sends command to write command byte
    if (kbd_write_cmd(OUT_BUF, cmd)) return 1;      // writes command
    return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    hook_timer = TIMER0_IRQ;
    *bit_no = BIT(hook_timer);
    if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_timer) == 0){
        return 0;
    }
    return 1;
}

int (timer_unsubscribe_int)() {
    if(sys_irqrmpolicy(&hook_timer) == 0) return 0;
    return 1;
}

void (timer_int_handler)() {
    time_elapsed++;
}

int (mouse_subscribe_int)(uint16_t *bit_no) {
    mouse_hook=MOUSE_IRQ;
    *bit_no = BIT(mouse_hook);
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook)) return 1;
    return 0;
}

int (mouse_unsubscribe_int)() {
	if (sys_irqrmpolicy(&mouse_hook)) return 1;
    return 0;
}

void (mouse_ih)(){
    uint8_t status;
    new_sys_inb(STAT_REG, &status);
    if ((status & PAR_ERR) || (status & TO_ERR) || !(status & AUX_ERR)) {
		return;
	}
    if (status & OBF) {
        if (new_sys_inb(OUT_BUF, &code)) {
            code=0;
            return;
        }
	}
}

void parse_mouse_packet(struct packet *pp){
    if(pp->bytes[0]&MOUSE_X_B){
        pp->delta_x=((-(pp->bytes[1]))^0xFF) + 1;
    }
    else{
        pp->delta_x=pp->bytes[1];
    }

    if(pp->bytes[0]&MOUSE_Y_B){
        pp->delta_y=((-(pp->bytes[2]))^0xFF) + 1;
    }
    else{
        pp->delta_y=pp->bytes[2];
    }
    pp->x_ov=(pp->bytes[0] & MOUSE_XOVF);

    pp->y_ov=(pp->bytes[0] & MOUSE_YOVF);

    pp->lb=(pp->bytes[0] & MOUSE_LB);

    pp->rb=(pp->bytes[0] & MOUSE_RB);

    pp->mb=(pp->bytes[0] & MOUSE_MB);

}

int mouse_write_cmd(uint8_t cmd){
    uint8_t res;

    kbd_write_cmd(KBC_CMD_REG, MOUSE_W_CMD);
    kbd_write_cmd(OUT_BUF, cmd);
    tickdelay(micros_to_ticks(DELAY_US));
    kbd_read_cmd(&res);

    if(res != ACK){
        kbd_write_cmd(KBC_CMD_REG, MOUSE_W_CMD);
        kbd_write_cmd(OUT_BUF, cmd);
        tickdelay(micros_to_ticks(DELAY_US));
        kbd_read_cmd(&res);

        if(res == ERROR){
            return 1;
        }
    }

    return 0;
}

enum event get_new_event(struct packet *pp){
    static bool lb_was_pressed=false, rb_was_pressed=false;
    if(pp->lb && !lb_was_pressed){
        lb_was_pressed=true;
        return PRESSED_LB;
    }
    else if(!pp->lb && lb_was_pressed){
        lb_was_pressed=false;
        return RELEASED_LB;
    }
    else if(pp->rb && !rb_was_pressed){
        rb_was_pressed=true;
        return PRESSED_RB;
    }
    else if(!pp->rb && rb_was_pressed){
        rb_was_pressed=false;
        return RELEASED_RB;
    }
    
    else if((pp->delta_x!=0) || (pp->delta_y!=0)){
        return MOVING;
    }
    else {
        return ANOTHER;
    }
    
}

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
                *(video_mem + (y*h_res + x) * bytes_pp + j) = col;
            }
        } else {
            col = (color >> (j * 8));
            *(video_mem + (y*h_res + x) * bytes_pp + j) = col;
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

uint32_t get_color(uint8_t no_rectangles, uint8_t col, uint8_t row, uint32_t first, uint8_t step){
    if(bits_pp==8){
        return (first + (row * no_rectangles + col) * step) % (1 << bits_pp);
    }
    else{
        uint32_t first_red, first_green, first_blue, trash;
        trash = 32 - info.RedMaskSize - info.GreenMaskSize - info.BlueMaskSize;

        first_red = first << (32 -trash);
        first_red = first_red >> (info.GreenMaskSize + info.BlueMaskSize);
        first_green = first << (trash + info.RedMaskSize);
        first_green = first_green >> (trash + info.RedMaskSize + info.BlueMaskSize);
        first_blue = first << (trash + info.RedMaskSize + info.GreenMaskSize);
        first_blue = (first_blue >> (trash + info.RedMaskSize + info.GreenMaskSize));

        first_red = (first_red + col * step) % (1 << info.RedMaskSize);
        first_green = (first_green + row * step) % (1 << info.GreenMaskSize);
        first_blue = (first_blue + (col+row) * step) % (1 << info.BlueMaskSize);

        return (first_red<<info.RedFieldPosition | first_green<<info.GreenFieldPosition | first_blue<<info.BlueFieldPosition);
    }
}

void xmp_draw_line(uint8_t *xpm, uint16_t w, uint16_t x, uint16_t y){
    uint8_t *line = (uint8_t *)(video_mem + (x + y*h_res) * bytes_pp);
    for (int i = 0; i < w; i++){
        *line = *xpm;
        line++;
        xpm++;
    }
}

void xpm_draw(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t x, uint16_t y){
    for(int i = 0; i < h; i++){
        xmp_draw_line(xpm, w, x, y + i);
        xpm += w;
    }
}

void clear_VRAM(){
    for (unsigned int i = 0; i < h_res * v_res; i++) {
        *(video_mem + i) = 0;
    }
}

int get_info_controller(vg_vbe_contr_info_t *info_holder){
    
    mmap_t memory;
    struct reg86 regist86;

    //lm_init(true);
    lm_alloc(512, &memory);

    memset(&regist86, 0, sizeof(regist86));

    info_holder = (vg_vbe_contr_info_t *)memory.virt;

    info_holder->VBESignature[0] = 'V' ;
    info_holder->VBESignature[1] = 'B' ;
    info_holder->VBESignature[2] = 'E' ;
    info_holder->VBESignature[3] = '2' ;

    regist86.ax = 0x4F00;
    regist86.intno = 0x10;


    regist86.es = PB2BASE(memory.phys);
    regist86.di = PB2OFF(memory.phys);

    if( sys_int86(&regist86) != OK ) {
        printf("sys_int86() failed \n");
        return 1;
    }

    vg_display_vbe_contr_info(info_holder);
    lm_free(&memory);
    return 0;
}
