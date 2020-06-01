/*	Author: Sana
 *  Partner(s) Name: 
 *	Lab Section: 25
 *	Assignment: Lab #EC HW  Exercise # 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "../header/timer.h"
#endif

unsigned char A;
unsigned char A0;
unsigned char A14;
unsigned char A5;
unsigned char A67;
unsigned char pwr;
unsigned char mv;
unsigned char cam;
static unsigned char cnt, cnt2;

enum R_States{R_SMStart, read};
int TickFct_Read(int state) {
	switch(state) {
		case R_SMStart:
			state = read;
			break;
		case read:
			state = read;
			break;
		default:
			state = R_SMStart;
			break;
	}
	switch(state) {
		case R_SMStart: break;
		case read:
			A = ~PINA;
			A0 = A & 0x01;
			A14 = (A >> 1) & 0x0F;
			A5 = (A >> 5) & 0x01;
			A67 = (A >> 6);
			break;
		default: break;

	}
	return state;
}

enum P_States{P_SMStart, off, on_w, on, off_w};
int TickFct_Power(int state) {
	switch(state) {
		case P_SMStart:
			state = off;
			break;
		case off:
			if (A0) state = on_w;
			else state = off;
			break;
		case on_w:
			if (A0) state = on_w;
			else state = on;
			break;
		case on:
			if (A0) state = off_w;
			else state = on;
			break;
		case off_w:
			if (A0) state = off_w;
			else state = off;
			break;
		default:
			state = P_SMStart;
			break;
	}
	switch(state) {
		case P_SMStart: 	break;
		case off: pwr = 0x00; 	break;
		case on_w: pwr = 0x01;	break;
		case on: pwr = 0x01; 	break;
		case off_w: pwr = 0x00; break;
		default: 		break;
	}
	return state;
}

enum M_States{M_SMStart, still, up, down, left, right};
int TickFct_Move(int state) {
	switch(state) {
		case M_SMStart: 
			state = still;
			break;
		case still:
			if (A14 == 0x01) state = up;
			else if (A14 == 0x02) state = down;
			else if (A14 == 0x04) state = left;
			else if (A14 == 0x08) state = right;
			else state = still;
			break;
		case up:
			if (A14 != 0x01) state = still;
			else state = up;
			break;
		case down:
			if (A14 != 0x02) state = still;
			else state = down;
			break;
		case left:
			if (A14 != 0x04) state = still;
			else state = left;
			break;
		case right:
			if (A14 != 0x08) state = still;
			else state = right;
			break;
		default:
			state = M_SMStart;
			break;
	}
	switch(state) {
		case M_SMStart: 	break;
		case still: mv = 0x00; 	break;
		case up: mv = 0x01; 	break;
		case down: mv = 0x02; 	break;
		case left: mv = 0x04; 	break;
		case right: mv = 0x08;  break;
		default: 		break;
	}
	return state;
}

enum Camera_States{C_SMStart, c_off, c_wait, pic, vid_w, vid, c_off_w};
int TickFct_Camera(int state) {
	switch(state) {
		case C_SMStart:
			state = C_SMStart;
			break;
		case c_off:
			if (A67 == 0x03) state = c_wait;
			else state = c_off;
			break;
		case c_wait:
			if ((A67 & 0x01) && cnt <= 100) state = c_wait;
			else if (!(A67 & 0x01) && cnt <= 100) state = pic;
			else if ((A67 & 0x01) && cnt > 100) state = vid_w;
			else state = vid;
			break;
		case pic:
			if (cnt2 <= 100) state = pic;
			else state = c_off_w;
			break;
		case vid_w:
			if (A67 & 0x01) state = vid_w;
			else state = vid;
			break;
		case vid:
			if (A67 & 0x01) state = c_off_w;
			else state = vid;
			break;
		case c_off_w:
			if (A67 & 0x01) state = c_off_w;
			else state = c_off;
			break;
		default:
			state = C_SMStart;
			break;
	}
	switch(state) {
		case C_SMStart:			break;
		case c_off: cnt = 0x00;	cnt2 = 0;	break;
		case c_wait: cnt++;		break;
		case pic: cam = 0x01; cnt2++;		break;
		case vid_w: cam = 0x01;		break;
		case vid: cam = 0x01;		break;
		case c_off_w: cam = 0x00;	break;
		default: 			break;
	}
	return state;
}

enum O_States{O_SMStart, output};
int TickFct_Output(int state) {
	switch(state) {
		case O_SMStart:
			state = output;
			break;
		case output:
			state = output;
			break;
		default:
			state = O_SMStart;
			break;
	}
	switch (state) {
		case O_SMStart: break;
		case output:
			if (pwr) PORTB = pwr | (mv << 1) | (A5 << 5) | (cam << 6);
			else PORTB = 0x00;
			break;
		default: break;
	}
	return state;
}

int main(void) {

	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	unsigned char i = 0x00;

	tasks[i].state = R_SMStart;
	tasks[i].period = 0x0A;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Read;
	i++;
	tasks[i].state = P_SMStart;
	tasks[i].period = 50;
	tasks[i].elapsedTime= tasks[i].period;
	tasks[i].TickFct = &TickFct_Power;
	i++;
	tasks[i].state = M_SMStart;
        tasks[i].period = 0x32;
        tasks[i].elapsedTime= tasks[i].period;
        tasks[i].TickFct = &TickFct_Move;
	i++;
	tasks[i].state = C_SMStart;
	tasks[i].period = 0x0A;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Camera;
	i++;
	tasks[i].state = O_SMStart;
	tasks[i].period = 0x0A;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Output;

	TimerSet(timerPeriod);
	TimerOn();

	while (1) {
//		A = ~PINA;
//		A0 = A & 0x01;
//		A14 = (A >> 1) & 0x0F;
//		A67 = (A >> 6);

	}

    return 1;
}
