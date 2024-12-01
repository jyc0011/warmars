#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define BUFFER_SIZE 1024

// WebSocket client management
struct lws *clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// 시리얼 포트 초기화 함수
int init_serial(const char* port, int baudrate);

float phValue_1, humidity_1, temperature_1, units_1, ounces_1, calibration_factor_1, lightValue_1, soilMoistureValue_1;
float phValue_2, humidity_2, temperature_2, units_2, ounces_2, calibration_factor_2, lightValue_2, soilMoistureValue_2;

void* cunnect_arduino_1(void* arg);
void* cunnect_arduino_2(void* arg);

void save_data_to_csv(float ph, float light, float soil, float humidity, float temp);
int main() {
    pthread_t pt_a1, pt_a2;

    printf("\n[INFO] Starting threads for Arduino connections...\n");

    printf("[INFO] Creating Arduino_1 thread...\n");
    if (pthread_create(&pt_a1, NULL, connect_arduino_1, NULL) != 0) {
        fprintf(stderr, "[ERROR] Failed to create Arduino_1 thread\n");
        return -1;
    }

    printf("[INFO] Creating Arduino_2 thread...\n");
    if (pthread_create(&pt_a2, NULL, connect_arduino_2, NULL) != 0) {
        fprintf(stderr, "[ERROR] Failed to create Arduino_2 thread\n");
        return -1;
    }

    // Python 스크립트를 실행
    printf("[INFO] Launching Python script for visualization...\n");
    system("python3 visualize.py");

    pthread_join(pt_a1, NULL);
    pthread_join(pt_a2, NULL);

    return 0;
}



void save_data_to_csv(float ph, float light, float soil, float humidity, float temp) {
    FILE *file = fopen("data.csv", "a");
    if (file == NULL) {
        perror("[ERROR] Unable to open data.csv");
        return;
    }

    fprintf(file, "%.2f,%.2f,%.2f,%.2f,%.2f\n", ph, light, soil, humidity, temp);
    fclose(file);
}

void* cunnect_arduino_1(void* arg) {
    printf("[DEBUG] Initializing Arduino 1 serial connection...\n");
    int serial_fd = init_serial("/dev/ttyUSB0", B9600);
    if (serial_fd == -1) {
        perror("[ERROR] open_port: Unable to open port Arduino1");
        return (void*)-1;
    }

    char buffer[1024];
    int n;

    while (1) {
        n = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[DEBUG] Arduino 1 Raw Data: %s\n", buffer);

            if (sscanf(buffer, "PH,%f,Light,%f,Soil,%f,Humidity,%f,Temperature,%f",
                       &phValue_1, &lightValue_1, &soilMoistureValue_1, &humidity_1, &temperature_1) == 5) {
                printf("[INFO] Arduino 1 Parsed Data - PH: %.2f, Light: %.2f, Soil: %.2f, Humidity: %.2f, Temp: %.2f\n",
                       phValue_1, lightValue_1, soilMoistureValue_1, humidity_1, temperature_1);            } else {
                fprintf(stderr, "[ERROR] Failed to parse data from Arduino 1\n");
            save_data_to_csv(phValue_1, lightValue_1, soilMoistureValue_1, humidity_1, temperature_1);
            }
        } else if (n < 0) {
            perror("[ERROR] Arduino 1 read failed");
        }

        usleep(1000000);
    }

    close(serial_fd);
    return NULL;
}


void* cunnect_arduino_2(void* arg) {
    printf("[DEBUG] Initializing Arduino 2 serial connection...\n");
    int serial_fd = init_serial("/dev/ttyUSB1", B9600);
    if (serial_fd == -1) {
        perror("[ERROR] open_port: Unable to open port Arduino2");
        return (void*)-1;
    }

    char buffer[1024];
    int n;

    while (1) {
        n = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[DEBUG] Arduino 2 Raw Data: %s\n", buffer);

            if (sscanf(buffer, "PH,%f,Light,%f,Soil,%f,Humidity,%f,Temperature,%f",
                       &phValue_2, &lightValue_2, &soilMoistureValue_2, &humidity_2, &temperature_2) == 5) {
                printf("[INFO] Arduino 2 Parsed Data - PH: %.2f, Light: %.2f, Soil: %.2f, Humidity: %.2f, Temp: %.2f\n",
                       phValue_2, lightValue_2, soilMoistureValue_2, humidity_2, temperature_2);
            save_data_to_csv(phValue_2, lightValue_2, soilMoistureValue_2, humidity_2, temperature_2);
            } else {
                fprintf(stderr, "[ERROR] Failed to parse data from Arduino 2\n");
            }
        } else if (n < 0) {
            perror("[ERROR] Arduino 2 read failed");
        }

        usleep(1000000);
    }

    close(serial_fd);
    return NULL;
}

// 시리얼 포트 초기화 함수
int init_serial(const char* port, int baudrate) {
    printf("[DEBUG] Initializing serial port: %s\n", port);
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("[ERROR] open_port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(fd, TCSANOW, &options);
    printf("[INFO] Serial port %s initialized successfully\n", port);
    return fd;
}