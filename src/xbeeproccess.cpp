//
// Created by robin on 1/28/22.
//
#include "xbeeinterface.h"


#include <cstring>


int xbee_port;

#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <thread>

const char *port = "/dev/Xbee";
using std::thread;

int serial_port;
struct termios tty;
int s = 1; //sending mode. s==0 when sleeping mode.

int n = 0;

int bm = 1; //buffer mode. Either 1 or 2.
int tsize = 0;


bool setup() {
    if (tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return false;
        //return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 1;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        //return 1;
        return false;
    }
    return true;
}

void changebm() {
    if (bm == 1) {
        bm = 2;
    } else {
        bm = 1;
    }
}


void send() {
    while (!getRestart()) {
        auto msg = getmsgToSend().c_str();
        if (strlen(msg) != 0) {
            tsize = tsize + sizeof(byte);

            char byte[strlen(msg)];
            tsize = tsize + sizeof(byte);
            for (int i = 0; i < sizeof(byte); i++) {
                byte[i] = msg[i];
                //printf("%d",i);
            }
            delete (msg);
            char tail[] = {'t', 'a', 'i', 'l', '\n'};
            if (s == 1) {
                //strcpy(byte,T2all_l[n]);
                if (byte[0] == '-') {
                    write(serial_port, tail, sizeof(tail)); //Send tail.
                    printf("Size : %d\n", tsize);
                    tsize = 0;
                    sleep(1);
                } else {
                    write(serial_port, byte, sizeof(byte)); //Send byte.
                }
            } else //s is not 1, Send SLEEPING.
            {
                if (byte[0] == '-') {
                    //printf("SLEEPING:%d\n",n);
                    char sleeping[] = {'S', 'L', 'E', 'E', 'P', 'I', 'N', 'G', '\n'};
                    write(serial_port, sleeping, sizeof(sleeping));
                    sleep(1);
                }
            }
            n = n + 1;
            if (bm == 1) {
                char B1[sizeof(byte)];
                strcpy(B1, byte);
            } else {
                char B2[sizeof(byte)];
                strcpy(B2, byte);
            }
            changebm();
        }
    }
}


void readfromNear() {
    int num_bytes;
    char read_buf[256];
    while (!getRestart()) {
        num_bytes = read(serial_port, read_buf, sizeof(read_buf));
        if (read_buf[0] == 'S') {
            s = 0;
        } else if (read_buf[0] == 'T') {
            s = 1;
        }
        printf("%s", read_buf);
        addmsgtoUnpack(string(read_buf));
        //memset(&read_buf, '\0', sizeof(read_buf));
    }
}


bool xbeeLoop() {

    serial_port = open(port, O_RDWR);
    if (!setup()) { return false; }
    thread _readfromNear(readfromNear);
    thread _send(send);
    _readfromNear.join();
    _send.join();
    return true;
}
