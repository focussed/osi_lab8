// Userspace sanity test - communicates with Arduino over USB serial
// Compile with: gcc -o temp_test temp_test.c
// Run with: ./temp_test
// ATU Sligo - Operating Systems Interfacing Lab 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B9600

int configure_serial(int fd) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }
    
    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                     // disable break processing
    tty.c_lflag = 0;                            // no signaling, echo, etc.
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0;                        // read doesn't block
    tty.c_cc[VTIME] = 5;                        // 0.5 seconds read timeout
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // shut off flow control
    tty.c_cflag |= (CLOCAL | CREAD);            // ignore modem controls
    tty.c_cflag &= ~(PARENB | PARODD);          // no parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                    // no hardware flow control
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    
    return 0;
}

int main() {
    int fd;
    char buf[64];
    ssize_t n;
    
    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", SERIAL_PORT, strerror(errno));
        fprintf(stderr, "Check Arduino is connected and you have permission (sudo chmod 666 /dev/ttyACM0)\n");
        return 1;
    }
    
    if (configure_serial(fd) < 0) {
        close(fd);
        return 1;
    }
    
    // Send 'T' to request temperature
    write(fd, "T", 1);
    tcdrain(fd);  // wait for transmission to complete
    
    // Read response
    usleep(100000); // give Arduino time to respond
    n = read(fd, buf, sizeof(buf) - 1);
    
    if (n > 0) {
        buf[n] = '\0';
        printf("Arduino response: %s", buf);
        
        // Extract temperature value
        char *temp_start = strstr(buf, "TEMP: ");
        if (temp_start) {
            float temp;
            sscanf(temp_start + 6, "%f", &temp);
            printf("Temperature: %.1f°C\n", temp);
        }
    } else {
        printf("No response from Arduino. Check connections.\n");
    }
    
    close(fd);
    return 0;
}