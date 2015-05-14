#include "mbed.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include "SDFileSystem.h"


#define MAXLINE 100

using namespace std;

DigitalIn start_stop(p11); // start/stop button detection on pin 11
DigitalIn ctmode_toggle(p12); // capture/transit mode toggle button detection on pin 12
PwmOut c_led(p21); // capture LED on PWM pin 21
PwmOut t_led(p22); // transmit LED on PWM pin 22
PwmOut vib_out(p23); // vibrator control PWM pin 23

SDFileSystem sd(p5, p6, p7, p8, "sd"); // the pinout on the mbed Cool Components workshop board
char myline[MAXLINE];
int loc, inch, clear;
AnalogIn inputx(p20); // input pins 20,19,18 for x,y,z axis respectively.
AnalogIn inputy(p19);
AnalogIn inputz(p18);
Serial pc(USBTX, USBRX); //tx, rx
Serial bt(p13, p14); // tx, rx

long int pos;
int fseek_test;
int fseek_test2;
int vib_on; 
int time_since; // time since last spike to control vibrator 
Timer t;
int vib_ready; 
int high_time;
int vib_time;


int mode; // capture or transmit
int session; // for indexing runs
int ctr; // counter for toggle

float ax;
float ay;
float az;
float gx;
float gy;
float gz;
float at; 
float gt;
int vib_start; 

int stroke_count = 0; 
int time_1_start = 0;
int time_1 = 0;
int time_2_start = 0;
int time_2 = 0;
int time_3_start = 0;
int time_3 = 0;
int stroke_avg; 
int high = 0;
int time_4; 
int time_4_start;
int last_mode; 
int last_valid_mode;
int new_mode; 

int i;
int ts;

int last_ctmode;
int last_ssval; 

int cur_ctmode;
int cur_ssval;

int ctmode_tog;
int ssval_tog; 

int main()
{
    
    
    pc.printf("Smart Paddles by Laura Kingsley and Catherine Yee\n\r");
    mkdir("/sd/mydir", 0777);

    pc.printf("Ready to capture\r\n");
    pc.baud(9600); //set pc baud rate
    bt.baud(115200); // set bluetooth baud rate
    mode = 1; // start with capture standby mode
    session = 0;
    c_led = 4.0f; // turn on capture led
    t_led = 0.0f;   // transmit led off
    vib_out = 0.0f; // vibrator off
    t.start();
    t.reset();
    ctr = 0;
    vib_on = 0;
    
    ctmode_tog = 0;
    ssval_tog = 0; 
    last_ctmode = 0;
    last_ssval = 0; 
   
    time_since = 0; 
    

    while(1) {
        
        /* set mode for time in while loop */
        cur_ssval = start_stop;
        cur_ctmode = ctmode_toggle;
        ctmode_tog = 0;
        ssval_tog = 0; 
        
        if (last_ssval != cur_ssval) {
            ssval_tog = 1;
            pc.printf("ssval tog\r\n");
        }
        
        if (last_ctmode != cur_ctmode) {
            ctmode_tog = 1; 
            pc.printf("ctmode tog\r\n");
        } 
        
        last_ctmode = cur_ctmode;
        last_ssval = cur_ssval; 
        
        
        if(ssval_tog) { // start/stop button press detected
            if(mode == 1) { // capture standby mode
                mode = 2; // capture mode
                session++;
                wait(0.5);
                t.reset();
                c_led = 4.0f; // on
                t_led = 0.0f; // off
                vib_out = 0.0f; // off
                vib_ready = 0;
                vib_on = 0;
                ctr = 0;

                FILE *fp = fopen("/sd/mydir/sdtest.txt", "w");
                if(fp == NULL) {
                    error("Could not open file for write\n\r");
                }

                // pos = ftell(fp);
                // pc.printf("pos in fp before write: %d\n\r", pos);
                fseek_test = fseek(fp, pos, SEEK_SET);
                if (fseek_test != 0) {
                    pc.printf("fseek fail\n");
                }
                pos = ftell(fp);
                // pc.printf("pos in fp after write: %d\n\r", pos);
                pc.printf("Session %d\r\n", session);
                stroke_count = 0; 
                time_1_start = 0;
                time_1 = 0;
                time_2_start = 0;
                time_2 = 0;
                time_3_start = 0;
                time_3 = 0;
                vib_ready = 0;
                high = 0;
                
                fclose(fp);

            } else if(mode == 2) { // capture mode
                mode = 1; // return to capture standby
                c_led = 4.0f; // on
                t_led = 0.0f;   // off
                vib_out = 0.0f; // off
                vib_ready = 0;
                vib_on = 0;
                wait(1);

            } else if(mode == 3) { // transmit standby mode
                mode = 4; // capture mode
                t_led = 1.0f; // on
                c_led = 0.0f; // off
                vib_out = 0.0f; // off
                ctr = 0;
                pc.printf("Transmitting...\r\n");
                wait(0.5);

            } else if(mode == 4) { // transmit mode
                mode = 3; // return to transmit standby
                t_led = 1.0f; // on
                c_led = 0.0f;   // off
                vib_out = 0.0f; // off
                pc.printf("Ready to transmit\r\n");
                wait(1);

            } else if (mode == 5) {
                mode = 3; 
        
            } 
        
        }

        else if(ctmode_tog) { // mode toggle button press detected
            if(mode == 1) { // capture standby -> transmit standby
                pc.printf("EOF\r\n");
                mode = 3; // return to transmit standby
                c_led = 0.0f;   // off
                t_led = 1.0f; // on
                vib_out = 0.0f; // off
                vib_ready = 0;
                vib_on = 0;
                session = 0;
                pc.printf("Ready to transmit\r\n");
                wait(1);

            } else if(mode == 3) { // transmit standby -> capture standby
                mode = 1; // return to capture standby
                c_led = 4.0f; // on
                t_led = 0.0f;   // off
                vib_out = 0.0f; // off
                vib_ready = 0;
                vib_on = 0;
                pc.printf("Ready to capture\r\n");
                wait(1);
            }
            else if (mode == 5) {
                c_led = 4.0f; // on
                t_led = 0.0f;   // off
                vib_out = 0.0f; // off
                vib_ready = 0;
                vib_on = 0;
                pc.printf("Ready to capture\r\n");
                wait(1);
                mode = 2;
            } 
        }

        if(mode == 2) {
            ctr++;
            if (ctr%3 == 0) { // toggle capture LED
                if(c_led == 0.0f) {
                    c_led = 1.0f;
                } else if(c_led == 1.0f) {
                    c_led = 0.0f;
                }
            }
            ts = t.read_ms();
            ax = inputx.read();
            ay = inputy.read();
            az = inputz.read();
            
            // calculate # of Gs
            gx = 9.615*(ax-0.503);
            gy = 8.696*(ay-0.491);
            gz = 9.709*(az-0.503);
            gt = sqrt(gx*gx + gy*gy + gz*gz);
            //gt = (gx^2 + gy^2 + gz^2)^0.5;
            FILE *fp1 = fopen("/sd/mydir/sdtest.txt", "w");
            if(fp1 == NULL) {
                error("Could not open file for write\n\r");
            }
            // pc.printf("pos in fp1 before write: %d\n\r", pos);
            //pos = ftell(fp1);
            fseek_test = fseek(fp1, pos, SEEK_SET);
            if (fseek_test != 0) {
                pc.printf("fseek fail\n");
            }
 


            if (gt > 1.3) {
                if (stroke_count < 5) {
                    if (high) {

                    } else {
                        if (stroke_count == 0) {
                            time_1_start = ts;
                            pc.printf("time 1 start %d\n\r",time_1_start);

                        } else if (stroke_count == 1) {
                            time_2_start = ts;
                            time_1 = ts - time_1_start;
                            pc.printf("time 2 start %d\n\r",time_2_start);
                            pc.printf("time 1 %d\n\r",time_1);
                        } else if (stroke_count == 2) {
                            time_3_start = ts;
                            pc.printf("time 3 start %d\n\r",time_3_start);
                            time_2 = ts - time_2_start;
                            pc.printf("time 2 %d\n\r",time_2);
                        } else if (stroke_count == 3) {
                            time_3_start = ts;
                            pc.printf("time 3 start %d\n\r",time_3_start);
                            time_2 = ts - time_2_start;
                            pc.printf("time 2 %d\n\r",time_2);
                        

                        } else {
                            time_4 = ts - time_4_start;
                            pc.printf("time 1 %d\n\r",time_1);
                            pc.printf("time 2 %d\n\r",time_2);
                            pc.printf("time 3 %d\n\r",time_3);
                            pc.printf("time 4 %d\n\r",time_4);
                            vib_ready = 1;
                            stroke_avg = (time_1 + time_2 + time_3 + time_4) / 4;
                            pc.printf("stroke_avg %d\n\r",stroke_avg);

                        }
                        stroke_count++;
                    }



                } else {

                    if (high) {

                    } else {
                        high_time = ts;
                        vib_on = 0;
                        vib_out = 0.0f;
                    }

                }

                high = 1;

            }

            else {  // lower than high threshold
                high = 0;
                time_since = ts - high_time;
            }

            if (vib_ready) {
                if (time_since > (stroke_avg)) {
                    if (vib_on == 1) {
                        vib_time = ts - vib_start;
                    } else {
                        vib_on = 1;
                        vib_start = ts;
                        vib_time = 0;
                    }

                    if ((vib_time > 1000 && vib_time < 3000)) {
                        vib_out = 0.0f;
                    } else {
                        vib_out = 8.0f;
                    }
                }
            }
            
            fprintf(fp1, "%d,%d,%f,%f,%f,%f,%d\r\n", session, ts, gx, gy, gz, gt, vib_on);
            
            pos = ftell(fp1);
            //  pc.printf("pos in fp1 after write: %d\n\r", pos);
            fclose(fp1);
            pc.printf("%d,%d,%f,%f,%f,%f,%d\r\n", session, ts, gx, gy, gz, gt, vib_on);
            
        } else if(mode == 3) {
            c_led = 0.0f; // off
            t_led = 1.0f; // on
        } else if(mode == 4) {
            pc.printf("Transmit code\r\n");

            FILE *fp2 = fopen("/sd/mydir/sdtest.txt", "r");
            if(fp2 == NULL) {
                pc.printf("open error");
                error("Could not open file for read\n");
            }

            pc.printf("Opened file for read\n\r");

            rewind(fp2);
            loc= 0;

            pc.printf("Read from file: \n\r");
            while ((inch = fgetc (fp2)) && inch != EOF) {
                if (loc == 99) {
                    pc.printf("%s",myline);
                    bt.printf("%s",myline);
                    loc = 0;
                }
                myline[loc++] = inch;
                ctr++;
                if (ctr%5 == 0) { // toggle transmit LED
                    if(t_led == 0.0f) {
                        t_led = 1.0f;
                    } else if(t_led == 1.0f) {
                        t_led = 0.0f;
                    }
                }
            }

            clear = 0;
            while (clear < loc) {
                pc.printf("%c",myline[clear]);
                bt.printf("%c",myline[clear]);
                clear++;
                ctr++;
                if (ctr%5 == 0) { // toggle transmit LED
                    if(t_led == 0.0f) {
                        t_led = 8.0f;
                    } else if(t_led == 8.0f) {
                        t_led = 0.0f;
                    }
                }
            }

            fclose(fp2);

            //mode = 3;
            mode = 5;
        }

    }
}

