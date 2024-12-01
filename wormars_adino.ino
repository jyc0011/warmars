/*
    Arduino pin 5 -> HX711 CLK
    Arduino pin 6 -> HX711 DOUT
    Arduino pin 5V -> HX711 VCC
    Arduino pin GND -> HX711 GND 
 */


#include <dht.h> // DHT11
#include "HX711.h" // 로드셀 
#define DHT11_PIN 2 // DHT11 A2
#define Vref 4.95 // 아날로그 참조값 (pH 계산에 사용)

dht DHT;
HX711 scale(6, 5);

// 변수 전역 선언
float calibration_factor = -14; // 로드셀 캘리브레이션 값
float units;
float ounces;
float humidity; // 습도
float temperature; // 온도
float phValue; // pH
int lightValue; // 조도
int soilMoistureValue; // 토양 수분
unsigned long int avgValue; // 평균 값 저장 변수

// 함수 원형 선언
void setup();
void loop();
void readDHT11();
void readPH();
void readLight();
void readSoilMoisture();
void readLoadCell();
void printSensorValues();
void adjustCalibrationFactor();

void setup() {
    Serial.begin(9600); // 시리얼 통신 초기화 (9600 보드레이트)
    pinMode(A0, INPUT); // A0 핀을 pH 센서 입력
    pinMode(A1, OUTPUT);// A1 핀을 pH 센서 출력
    pinMode(A3, INPUT); // A3 핀을 조도 센서 입력
    pinMode(A4, INPUT); // A4 핀을 토양 수분 센서 입력

    // 로드셀 초기화
    scale.set_scale();
    scale.tare();  // 로드셀 초기화
    long zero_factor = scale.read_average(); // 기준 값 읽기
    Serial.print("Zero factor: "); // 이 값은 로드셀 프로젝트에서 0점 조정 없이 사용될 수 있습니다.
    Serial.println(zero_factor);
}

void loop() {
    readDHT11(); // DHT11 센서 값 읽기
    readPH(); // pH 센서 값 읽기
    readLight(); // 조도 센서 값 읽기
    readSoilMoisture(); // 토양 수분 센서 값 읽기
    readLoadCell(); // 로드셀 값 읽기
    adjustCalibrationFactor(); // 캘리브레이션 값 조정
    printSensorValues(); // 센서 값 출력

    delay(1000); // 1초 대기
}

void readDHT11() {
    DHT.read11(DHT11_PIN);
    humidity = DHT.humidity;
    temperature = DHT.temperature;
}

void readPH() {
    int buf[10];
    for (int i = 0; i < 10; i++) {
        buf[i] = analogRead(A0);
        delay(10);
    }

    // 값 정렬
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buf[i] > buf[j]) {
                int temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    avgValue = 0;
    for (int i = 2; i < 8; i++) { // 중간 6개 값의 평균 계산
        avgValue += buf[i];
    }

    float sensorValue = avgValue / 6.0;
    phValue = 7 - 1000 * (sensorValue - 365) * Vref / 59.16 / 1023;
}

void readLight() {
    lightValue = analogRead(A3);
}

void readSoilMoisture() {
    soilMoistureValue = analogRead(A4);
}

void readLoadCell() {
    scale.set_scale(calibration_factor);
    units = scale.get_units(), 10;
    if (units < 0) {
        units = 0.00;
    }
    ounces = units * 0.035274;
}

void adjustCalibrationFactor() {
    if (Serial.available()) {
        char temp = Serial.read();
        if (temp == '+' || temp == 'a') {
            calibration_factor += 1;
        } else if (temp == '-' || temp == 'z') {
            calibration_factor -= 1;
        }
    }
}

void printSensorValues() {
    Serial.print("PH,");
    Serial.print(phValue, 2);
    Serial.print(",Light,");
    Serial.print(lightValue);
    Serial.print(",Soil,");
    Serial.print(soilMoistureValue);
    Serial.print(",Humidity,");
    Serial.print(humidity);
    Serial.print(",Temperature,");
    Serial.print(temperature);
    Serial.print(",Load_Cell,");
    Serial.print(units);
    Serial.print(",Grams,");
    Serial.print(ounces);
    Serial.print(",Calibration,");
    Serial.println(calibration_factor);
}