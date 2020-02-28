#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"
#include "game.h"
#include "timer.h"
#include "kbd.h"
#include "video.h"
#include "mouse.h"


extern uint8_t code;
extern uint32_t irq_set;
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
extern bool first;
extern enum xpm_image_type type;
extern char *buffer;
int16_t deslocation;
uint16_t player_x, player_w;
bool grid[5][8];
uint8_t blocks;
uint16_t ball_x;
uint16_t ball_y;
float ball_vx;
float ball_vy;
uint16_t bonus_x;
uint16_t bonus_y;


unsigned main_menu(){
    xpm_image_t img_bg;
    xpm_image_t img_arrow;

    xpm_load(MENU_xpm, type, &img_bg);

    xpm_draw(img_bg.bytes, img_bg.width, img_bg.height, 0, 0);

    xpm_load(ARROW_xpm, type, &img_arrow);

    xpm_draw(img_arrow.bytes, img_arrow.width, img_arrow.height, 270, 280);

    unsigned out = 1;

    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    message msg;
    int r;
    time_elapsed = 0;

    bool Up=false, Down=false;

    while(code != ENTER_BC) {
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(0)) {
                        copybuffer();
                    }
                    if (msg.m_notify.interrupts & BIT(KBD_IRQ)) {
                        kbc_ih();
                        if (first) {
                            code = 0;
                            first = false;
                            continue;
                        }
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                            if(!Up && !Down){
                                if(code==UPBREAK)Up=true;
                                if(code==DOWNBREAK)Down=true;
                            }
                            if(Up || Down){
                                if(Up){
                                    xpm_subs(img_bg.bytes, img_bg.width, img_bg.height, img_arrow.width, img_arrow.height, 270, 280+(75*(out-1)));
                                    out--;
                                    if(out==0)out=3;
                                    xpm_draw(img_arrow.bytes, img_arrow.width, img_arrow.height, 270, 280+(75*(out-1)));
                                    Up=false;

                                }
                                else{
                                    xpm_subs(img_bg.bytes, img_bg.width, img_bg.height, img_arrow.width, img_arrow.height, 270, 280+(75*(out-1)));
                                    out++;
                                    if(out==4)out=1;
                                    xpm_draw(img_arrow.bytes, img_arrow.width, img_arrow.height, 270, 280+(75*(out-1)));
                                    Down=false;
                                }
                            }

                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                    }
                     if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)){
                         mouse_ih();
                     }
                    break;
                
                default:
                    break;
            }
        }
    }
    return out;
}

void play(){
    int ipc_status;
    message msg;
    int r;
    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    xpm_image_t player, ball, tbonus, pbonus;
    unsigned level=0, byte=0, time_count, score = 0, bonus_count;
    time_elapsed = 0;
    struct packet pp;
    enum event action = STANDING;
    deslocation=0;
    int ball_col, bonus;
    bool bonus_type = true, bonus_active = false, bonus_drop = false;
    struct Date start_date;
    struct Date end_date;

    clear_buffer();

    xpm_load(PLAYER_xpm, type, &player);
    player_w = 256;
    xpm_load(BALL_xpm, type, &ball);
    xpm_load(TBONUS_xpm, type, &tbonus);
    xpm_load(PBONUS_xpm, type, &pbonus);

    next_level(&level, player.bytes, ball.bytes, &time_count, score);
    bonus_x = 0;
    bonus_y = 0;
    copybuffer();

    rtc_time(&start_date);

    while(level <= 9) {
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(0)) {
                        copybuffer();
                        timer_int_handler();
                        if (time_elapsed%60 == 0){
                            time_count--;
                            draw_number(0, time_count);
                            if (time_count == 0){
                                clear_buffer();
                                return;
                            }
                            if (bonus_active){
                                bonus_count--;
                                if(bonus_count == 0){
                                    bonus_active = false;
                                    xpm_load(PLAYER_xpm, type, &player);
                                    vg_draw_rectangle(player_x,688,player_w,player.height,0);
                                    xpm_draw(player.bytes,player.width, player.height, player_x, 688);
                                    player_w = 256;
                                }
                            }
                        }
                        ball_col = handle_ball_collision(level);
                        if(ball_col==1){
                            draw_grid();
                            score += 5;
                            draw_number(1024-47*3, score);
                            if (blocks != 0 && !bonus_active && !bonus_drop){
                                bonus = rand()%(20) + 1;
                                if(bonus <= 3){
                                    bonus_drop = true;
                                    if (bonus_type){
                                        bonus_type = false;
                                    } else {
                                        bonus_type = true;
                                    }
                                } else {
                                    bonus_x = 0;
                                    bonus_y = 0;
                                }
                            } else if (!bonus_drop){
                                bonus_x = 0;
                                bonus_y = 0;
                            }
                        }
                        else if(ball_col==2){
                            clear_buffer();
                            return;
                        }
                        if(bonus_drop){
                            if(bonus_y <= 112 + 5*64 + 5){
                                draw_grid();
                            }
                            if(!bonus_type){
                                vg_draw_rectangle(bonus_x, bonus_y, 32, 32, 0);
                                bonus_y += 4;
                                xpm_draw(tbonus.bytes, 32, 32, bonus_x, bonus_y);
                                if(bonus_y == 688-32){
                                    if((bonus_x <= player_x + player_w && bonus_x >= player_x) || (bonus_x + 32  <= player_x && bonus_x + 32 >= player_x)){
                                        time_count += 10;
                                    }
                                    vg_draw_rectangle(bonus_x, bonus_y, 32, 32, 0);
                                    bonus_x = 0;
                                    bonus_y = 0;
                                    bonus_drop = false;
                                }
                            } else {
                                vg_draw_rectangle(bonus_x, bonus_y, 32, 8, 0);
                                bonus_y += 4;
                                xpm_draw(pbonus.bytes, 32, 8, bonus_x, bonus_y);
                                if(bonus_y == 688-8){
                                    if((bonus_x <= player_x + player_w && bonus_x >= player_x) || (bonus_x + 32  <= player_x && bonus_x + 32 >= player_x)){
                                        bonus_active = true;
                                        xpm_load(BPLAYER_xpm, type, &player);
                                        vg_draw_rectangle(player_x,688,player.width,player.height,0);
                                        if(player_x > 1024-384) player_x=1024-384;
                                        xpm_draw(player.bytes,player.width, player.height, player_x, 688);
                                        player_w = 384;
                                        bonus_count = 10;
                                    }
                                    vg_draw_rectangle(bonus_x, bonus_y, 32, 8, 0);
                                    bonus_x = 0;
                                    bonus_y = 0;
                                    bonus_drop = false;
                                }
                            }
                        }
                        vg_draw_rectangle(ball_x, ball_y, 32, 32, 0);
                        ball_x += ball_vx;
                        ball_y += ball_vy;
                        if(blocks == 0){
                            score += 5*time_count;
                            time_count = 0;
                            draw_number(0, time_count);
                            draw_number(1023-47*3, score);
                            time_elapsed=0;
                            if(level==9){
                                level++;
                                break;
                            }
                            next_level(&level, player.bytes, ball.bytes, &time_count, score);
                            bonus_drop = false;
                            bonus_x = 0;
                            bonus_y = 0;
                        }
                        xpm_draw(ball.bytes, ball.width, ball.height, ball_x, ball_y);
                        copybuffer();
                    }
                    if (msg.m_notify.interrupts & BIT(KBD_IRQ)) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                        if(code==ESC_BC){
                            if(pause_game()){
                                time_elapsed=0;
                                clear_buffer();
                                return;
                            }
                            else{
                                draw_grid();
                                xpm_draw(player.bytes, player_w, 30, player_x, 688);
                                draw_number(1024-47*3, score);
                                draw_number(0, time_count);
                            }
                        }
                    }
                    if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)){
                        mouse_ih();
                        if ((byte == 0 && (BIT(3) & code)) || byte == 1){
                            pp.bytes[byte] = code;
                            byte++;
                        }
                        else if (byte == 2){
                            pp.bytes[byte] = code;
                            byte = 0;
                            parse_mouse_packet(&pp);
                            action=get_new_event(&pp);
                            switch(action){
                                case PRESSED_MB:
                                    score += 5*time_count;
                                    time_count = 0;
                                    draw_number(0, time_count);
                                    draw_number(1023-47*3, score);
                                    time_elapsed=0;
                                    if(level==9){
                                        level++;
                                        break;
                                    }
                                    next_level(&level, player.bytes, ball.bytes, &time_count, score);
                                    bonus_drop = false;
                                    bonus_x = 0;
                                    bonus_y = 0;
                                    break;
                                case MOVING_TO_RIGHT:
                                    if(player_x+player_w==1024){
                                        break;
                                    }
                                    else if(player_x+player_w+deslocation>=1024){
                                        vg_draw_rectangle(player_x,688,player.width,player.height,0);
                                        player_x=1024-player_w;
                                        xpm_draw(player.bytes,player.width, player.height, player_x,688);
                                    }
                                    else{
                                        vg_draw_rectangle(player_x,688,player.width,player.height,0);
                                        player_x+=deslocation;
                                        xpm_draw(player.bytes,player.width, player.height, player_x,688);
                                    }
                                    break;
                                case MOVING_TO_LEFT:
                                    if(player_x==0){
                                        break;
                                    }
                                    else if(player_x+deslocation<=0){
                                        vg_draw_rectangle(player_x,688,player.width,player.height,0);
                                        player_x=0;
                                        xpm_draw(player.bytes,player.width, player.height, player_x,688);
                                    }
                                    else{
                                        vg_draw_rectangle(player_x,688,player.width,player.height,0);
                                        player_x+=deslocation;
                                        xpm_draw(player.bytes,player.width, player.height, player_x,688);
                                    }
                                    break;
                                default:
                                    break;
                            }
                                    
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    rtc_time(&end_date);

    struct Date real_date;
    xpm_image_t img_end;

    xpm_load(GEND_xpm, type, &img_end);

    xpm_draw(img_end.bytes, img_end.width, img_end.height, 0, 0);
    rtc_time(&real_date);
    draw_real_date(&start_date,0,0);
    draw_number(1024-47*3, score);
    draw_real_date(&end_date,0,100);

    copybuffer();

    while(code!=ENTER_BC) {
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(KBD_IRQ)) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                    }
                    if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)){
                        mouse_ih();
                    }
                    break;
                default:
                    break;
            }
        }
    }


    time_elapsed=0;
}

void credits(){
    xpm_image_t img_credits;

    xpm_load(CREDITS_xpm, type, &img_credits);

    xpm_draw(img_credits.bytes, img_credits.width, img_credits.height, 0, 0);

    copybuffer();

    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    message msg;
    int r;

    while(code != ENTER_BC) {
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(KBD_IRQ)) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                    }
                    if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)){
                        mouse_ih();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    clear_buffer();
}

void draw_player(uint8_t *xpm){
    xpm_draw(xpm, player_w, 30, 384, 688);
    player_x=384;
}

void draw_ball(uint8_t *ball, unsigned level){
    ball_x = 512;//512
    ball_y = 624;//624
    xpm_draw(ball, 32, 32, ball_x, ball_y);
    if (level < 5){
        ball_vy = -4;
        ball_vx = 4;
    } else {
        ball_vy = -8;
        ball_vx = 8;
    }
}

void draw_grid(){
    xpm_image_t img_block;
    xpm_load(BLOCK_xpm, XPM_8_8_8, &img_block);
    vg_draw_rectangle(0, 47, 1024, 1, 0xFFF);
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 8; j++){
            if(grid[i][j]){
                xpm_draw(img_block.bytes, 128, 64, j*128, 112 + i*64);
            } else {
                vg_draw_rectangle(j*128, 112 + i*64, 128, 64, 0);
            }
        }
    }
}

void draw_number(uint16_t xi, unsigned n){
    unsigned digits[3];
    digits[2] = n%10;
    digits[1] = ((n-digits[2])/10)%10;
    digits[0] = ((n-digits[2]-digits[1]*10)/100)%10;

    xpm_image_t digit;

    vg_draw_rectangle(xi, 0, 47*3, 47, 0);

    for(int i = 0; i < 3; i++){
        if (digits[i] == 0){
            xpm_load(a0_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 1){
            xpm_load(a1_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 2){
            xpm_load(a2_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 3){
            xpm_load(a3_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 4){
            xpm_load(a4_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 5){
            xpm_load(a5_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 6){
            xpm_load(a6_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 7){
            xpm_load(a7_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 8){
            xpm_load(a8_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        } else if (digits[i] == 9){
            xpm_load(a9_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi +i*47, 0);
        }
    }
}

void next_level(unsigned *level, uint8_t *player, uint8_t *ball, unsigned *time_count, unsigned score){
    next_level_card(*level);
    blocks = 0;
    if(*level==0){
        for(int i=0; i<5; i++){
            if (i % 2 == 0) {
                for (int j = 0; j < 8; j++) {
                    grid[i][j] = true;
                    blocks++;
                }
            } else {
                for (int j = 0; j < 8; j++) {
                    grid[i][j] = false;
                }
            }
        }
    } else if(*level==1){
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 8; j++){
                if (i == 3-j || i == j -4 || i == 4-j || i == j-3){
                    grid[i][j] = true;
                    blocks++;
                } else {
                    grid[i][j] = false;
                }
            }
        }
    } else if(*level==2) {
        for(int i=0; i<5; i++) {
            if(i < 3) {
                for (int j = 0; j < 8; j++) {
                    grid[i][j] = true;
                    blocks++;
                }
            } else {
                for (int j = 0; j < 8; j++) {
                    grid[i][j] = false;
                }
            }
        }
    } else if (*level == 3){
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 8; j++) {
                grid[i][j] = true;
                blocks++;
            }
        }
    } else {
        int aux;
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 8; j++){
                aux = rand()%(10) + 1;
                if(aux<=7){
                    grid[i][j] = true;
                    blocks++;
                } else {
                    grid[i][j] = false;
                }
            }
        }
    }
    (*level)++;
    if (*level < 5){
        *time_count = floor(blocks*3);
    } else {
        *time_count = floor(blocks*2);
    }
    clear_buffer();
    draw_number(0, *time_count);
    draw_number(1023-47*3, score);
    draw_grid();
    draw_player(player);
    draw_ball(ball, *level);
}

void next_level_card(unsigned level){
    code = 0;
    int ipc_status;
    message msg;
    int r;
    xpm_image_t card;
    uint16_t yi;
    struct packet pp;
    enum event action = STANDING;
    unsigned byte = 0;

    if(level == 0) {
        xpm_load(GBEGIN_xpm, type, &card);
        yi = 0;
    } else {
        xpm_load(NEXTLEVEL_xpm, type, &card);
        yi = 48;
    }

    xpm_draw(card.bytes, card.width, card.height, 0, yi);
    copybuffer();
    while(action != PRESSED_RB){
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)) {
                        mouse_ih();
                        if ((byte == 0 && (BIT(3) & code)) || byte == 1) {
                            pp.bytes[byte] = code;
                            byte++;
                        } else if (byte == 2) {
                            pp.bytes[byte] = code;
                            byte = 0;
                            parse_mouse_packet(&pp);
                            action = get_new_event(&pp);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

int handle_ball_collision(unsigned level){
    bool block_destroy_v = false;
    bool block_destroy_h = false;
    bool at_border = false;
    if((ball_x==0||ball_x==1024-32)){
        at_border = true;
    }
    if(ball_y == 48){
        ball_vy=-ball_vy;
    }
    if((ball_x+32>=player_x && ball_x<=player_x+player_w) && ball_y==688-32){
        if(ball_x+16==player_x+player_w/2){ // meio
            ball_vy=-4 *pow(2, floor(level/5) +1);
            ball_vx=0;
        }
        else if(ball_x<=player_x+player_w/2 && ball_x>=player_x+player_w/2-40){ // meio -40
            ball_vy=-ball_vy;
        }
        else if(ball_x<=player_x+player_w/2 && ball_x>=player_x+player_w/2-108){ // meio -108
            ball_vx=-2*pow(2, floor(level/5) +1);
            ball_vy=-2*pow(2, floor(level/5) +1);
        }
        else if(ball_x<=player_x+player_w/2 && ball_x >=player_x - 32){ // ponta da esquerda
            ball_vx=-4*pow(2, floor(level/5) +1);
            ball_vy=-1*pow(2, floor(level/5) +1);
        }
        else if(ball_x>=player_x+player_w/2 && ball_x<=player_x+player_w/2+40){ // meio +40
            ball_vy=-ball_vy;
        }
        else if(ball_x>=player_x+player_w/2 && ball_x<=player_x+player_w/2+108){ // meio +108
            ball_vx=2*pow(2, floor(level/5) +1);
            ball_vy=-2*pow(2, floor(level/5) +1);
        }
        else if(ball_x>=player_x+player_w/2 && ball_x<=player_x+player_w){ // ponta da direita
            ball_vx=4*pow(2, floor(level/5) +1);
            ball_vy=-1*pow(2, floor(level/5) +1);
        }
    }
    else if( ball_y > 688-32 ){
        return 2;
    }
    else {
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 8; j++){
                if(grid[i][j]){
                    if(!block_destroy_v && (ball_y == 112 + (i+1)*64 || ball_y + 32 == 112 + i*64) && ((ball_x > j*128 && ball_x < (j+1)*128) || (ball_x + 32 > j*128 && ball_x + 32 < (j+1)*128))){
                        ball_vy = -ball_vy;
                        grid[i][j] = false;
                        if(bonus_x == 0 && !block_destroy_h) {
                            bonus_x = 48 + j*128;
                            bonus_y = i*64 + 112;
                        }
                        blocks--;
                        block_destroy_v = true;
                    }
                    if (!block_destroy_h && (ball_x == (j+1)*128 || ball_x + 32 == j*128) && ((ball_y > 112 + i*64 && ball_y < 112 + (i+1)*64) || (ball_y + 32 > 112 + i*64 && ball_y + 32 < 112 + (i+1)*64))){
                        ball_vx = - ball_vx;
                        grid[i][j] = false;
                        if(bonus_x == 0 && !block_destroy_v) {
                            bonus_x = 48 + j * 128;
                            bonus_y = i* 64 + 112;
                        }
                        blocks--;
                        block_destroy_h = true;
                    }
                    if (!block_destroy_h && !block_destroy_v && ((ball_x == (j+1)*128 && ball_y == 112 + (i+1)*64 && !grid[i][j+1] && (i == 4 || !grid[i+1][j]) && ball_vx < 0 && ball_vy < 0) || (ball_x == (j+1)*128 && ball_y + 32 == 112 + i*64 && !grid[i][j+1] && (i == 0 || !grid[i-1][j]) && ball_vx < 0 && ball_vy > 0) || (ball_x + 32 == j*128 && ball_y == 112 + (i+1)*64 && !grid[i][j-1] && (i == 4 || !grid[i+1][j]) && ball_vx > 0 && ball_vy < 0) || (ball_x + 32 == j*128 && ball_y + 32 == 112 + i*64 && !grid[i][j-1] && (i == 0 || !grid[i-1][j]) && ball_vx > 0 && ball_vy > 0))){
                        ball_vx = -ball_vx;
                        ball_vy = -ball_vy;
                        grid[i][j] = false;
                        if(bonus_x == 0) {
                            bonus_x = 48 + j * 128;
                            bonus_y = i* 64 + 112;
                        }
                        blocks--;
                        block_destroy_h = true;
                        block_destroy_v = true;
                    }
                }
            }
        }
    }
    if (at_border){
        ball_vx = - ball_vx;
    }
    if(block_destroy_v || block_destroy_h){
        return 1;
    }
    else{
        return 0;
    }
}

void draw_real_date(struct Date *real_date, uint16_t x, uint16_t y){
    unsigned digits[14];

    digits[0] = real_date->day/10;
    digits[1] = real_date->day%10;
    digits[2] = real_date->month/10;
    digits[3] = real_date->month%10;
    digits[4] = real_date->year/1000;
    digits[5] = (real_date->year%1000)/100;
    digits[6] = (real_date->year%100)/10;
    digits[7] = real_date->year%10;
    digits[8] = real_date->hour/10;
    digits[9] = real_date->hour%10;
    digits[10] = real_date->minute/10;
    digits[11] = real_date->minute%10;
    digits[12] = real_date->second/10;
    digits[13] = real_date->second%10;

    xpm_image_t img_bg;
    xpm_image_t digit;
    xpm_image_t img_double;
    xpm_image_t img_bar;
    xpm_load(DOUBLE_xpm, type, &img_double);
    xpm_load(SLASH_xpm, type, &img_bar);
    xpm_load(MENU_xpm, type, &img_bg);

    uint16_t xi=x, yi=y;

    for(int i = 0; i < 14; i++){
        vg_draw_rectangle(xi,yi,47,47,0);
        if (digits[i] == 0){
            xpm_load(a0_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 1){
            xpm_load(a1_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 2){
            xpm_load(a2_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 3){
            xpm_load(a3_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 4){
            xpm_load(a4_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 5){
            xpm_load(a5_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 6){
            xpm_load(a6_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 7){
            xpm_load(a7_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 8){
            xpm_load(a8_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        } else if (digits[i] == 9){
            xpm_load(a9_xpm, type, &digit);
            xpm_draw(digit.bytes, 47, 47, xi, yi);
        }
        if(i==7){
            xi=x;
            yi+=55;
        }
        else if(i==1 || i==3){
            xpm_draw(img_bar.bytes, 15, 47, xi+47, yi);
            xi+=15;
        }
        else if(i==9 || i==11){
            xpm_draw(img_double.bytes, 8, 47, xi+47, yi);
            xi+=8;
        }
        xi+=47;
    }
}

bool pause_game(){
    struct Date real_date;
    xpm_image_t img_pause;

    xpm_load(GPAUSE_xpm, type, &img_pause);

    xpm_draw(img_pause.bytes, img_pause.width, img_pause.height, 0, 0);
    rtc_time(&real_date);
    draw_real_date(&real_date,0,0);

    copybuffer();

    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    message msg;
    int r;
    time_elapsed=0;


    while(true) {
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(0)) {
                        timer_int_handler();
                        if(time_elapsed%20 == 0) {
                            rtc_time(&real_date);
                            draw_real_date(&real_date,0,0);
                        }
                        copybuffer();
                    }
                    if (msg.m_notify.interrupts & BIT(KBD_IRQ)) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                        if(code==ESC_BC){
                            time_elapsed=0;
                            clear_buffer();
                            return false;
                        }
                        else if(code==ENTER_BC){
                            time_elapsed=0;
                            clear_buffer();
                            return true;
                        }
                    }
                    if (msg.m_notify.interrupts & BIT(MOUSE_IRQ)){
                        mouse_ih();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
