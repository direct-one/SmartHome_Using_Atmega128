#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#define DHT11_ERROR 255

// CLCD 명령 정의
#define BIT4_LINE2_DOT58 0x28 // 4비트 모드, 2라인, 5x8 도트
#define DISPON_CUROFF_BLKOFF 0x0C // 디스플레이 켜기, 커서 끄기, 깜박임 끄기
#define DISPCLEAR 0x01 // 디스플레이 지우기
#define CUR1LINE 0x80 // 커서 위치: 첫 번째 라인
#define CUR2LINE 0xC0 // 커서 위치: 두 번째 라인

// 초음파 센서 및 모터 제어 정의
#define CDS_50 550 // 조도 센서 임계값
#define TRIG 1 // 트리거 신호 (출력 = PE6)
#define ECHO 2 // 에코 신호 (입력 = PE7)
#define SOUND_VELOCITY 340UL // 소리의 속도 (m/sec)
#define LOWER_SPEED 255 // PWM 값: 50% 듀티 사이클
#define MOTOR_CW 0xb0
#define MOTOR_STOP 0x30

// DHT11 핀 정의
#define DHT11_DDR DDRG
#define DHT11_PORT PORTG
#define DHT11_PIN PING
#define DHT11_INPUTPIN PG0

// 함수 선언
void init_adc();
unsigned short read_adc();
void show_adc_led(unsigned short data);
uint8_t getdata(uint8_t select);
void dht11_getdata(uint8_t *data);
void CLCD_cmd(char cmd);
void CLCD_data(char data);
void CLCD_puts(char *str);

int main(void) {
   unsigned int distance;
   unsigned short adc_value;
   unsigned int no_detection_counter = 0;
   uint8_t temp = 0;
   char temp_str[16];
   uint8_t clcd_on = 0; // CLCD 상태 플래그

   // LED 및 모터 제어 포트 초기화
   DDRA = 0xff; // PORTA를 LED 출력으로 설정
   DDRB = 0xf0; // PB7 ~ PB4를 모터 제어로 설정
   PORTB = MOTOR_STOP; // 초기 정지 상태

   // 초음파 센서를 위한 포트 초기화
   DDRD = ((DDRD | (1 << TRIG)) & ~(1 << ECHO)); // TRIG는 출력, ECHO는 입력으로 설정

   // ADC 초기화
   init_adc();

   // CLCD 초기화
   DDRC = 0xFF; // PORTC : CLCD 명령/데이터 포트로 설정
   DDRE = 0xFF; // PORTE : CLCD 제어 포트로 설정

   _delay_ms(50); // 전원 안정화 지연

   while (1) {
      // 초음파 센서 작동 및 거리 측정
      TCCR1B = 0x03;
      PORTD &= ~(1 << TRIG);
      _delay_us(5);
      PORTD |= (1 << TRIG);
      _delay_us(10);
      PORTD &= ~(1 << TRIG);
      
      while (!(PIND & (1 << ECHO)));
      TCNT1 = 0x0000;
      while (PIND & (1 << ECHO));
      TCCR1B = 0x00;
      
      distance = (unsigned int)(SOUND_VELOCITY * (TCNT1 * 4 / 2) / 1000);
      
      if (distance < 600) { //반복문을 통해 초음파 센서에 감지시 모터 & LED & CLCD 동작
         no_detection_counter = 0;
         PORTA |= 0x0c;
         PORTB = MOTOR_CW;

         if (!clcd_on) {
            clcd_on = 1;
            CLCD_cmd(BIT4_LINE2_DOT58);
            CLCD_cmd(DISPON_CUROFF_BLKOFF);

            dht11_getdata(&temp);

            if (temp == DHT11_ERROR) {
               CLCD_cmd(CUR2LINE);
               CLCD_puts("Sensor Error ");
               } else {
               snprintf(temp_str, sizeof(temp_str), "NOW Temp: %dC", temp);
               CLCD_cmd(CUR1LINE);
               CLCD_puts("WELCOME");
               CLCD_cmd(CUR2LINE);
               CLCD_puts(temp_str);
            }
         }
         } else {
         no_detection_counter++;
         if (no_detection_counter >= (1500 / 50)) {
            PORTA &= ~0x0c;
            PORTB = MOTOR_STOP;

            if (clcd_on) {
               clcd_on = 0;
               CLCD_cmd(DISPCLEAR);
            }
         }
      }

      _delay_ms(50);

      adc_value = read_adc(); //광센서를 통한 LED작동
      show_adc_led(adc_value);

      _delay_ms(50);
   }
}

void init_adc() {
   ADMUX = 0x40;
   ADCSRA = 0x87;
}

unsigned short read_adc() {
   unsigned char adc_low, adc_high;
   unsigned short value;

   ADCSRA |= 0x40;
   while ((ADCSRA & 0x10) != 0x10);

   adc_low = ADCL;
   adc_high = ADCH;

   value = (adc_high << 8) | adc_low;

   return value;
}

void show_adc_led(unsigned short value) {
   if (value <= CDS_50) {
      PORTA |= 0x03;
      } else {
      PORTA &= ~0x03;
   }
}

void dht11_getdata(uint8_t *data) {
   uint8_t buf = getdata(0);
   if (buf != DHT11_ERROR) {
      *data = buf;
   }
}

uint8_t getdata(uint8_t select) {
   uint8_t bits[5];
   uint8_t i,j = 0;

   memset(bits, 0, sizeof(bits));
   DHT11_DDR |= (1<<DHT11_INPUTPIN);
   DHT11_PORT |= (1<<DHT11_INPUTPIN);
   _delay_ms(100);

   DHT11_PORT &= ~(1<<DHT11_INPUTPIN);
   _delay_ms(18);
   DHT11_PORT |= (1<<DHT11_INPUTPIN);
   _delay_us(1);
   DHT11_DDR &= ~(1<<DHT11_INPUTPIN);

   _delay_us(39);

   if((DHT11_PIN & (1<<DHT11_INPUTPIN))) return DHT11_ERROR;

   _delay_us(80);

   if(!(DHT11_PIN & (1<<DHT11_INPUTPIN))) return DHT11_ERROR;

   _delay_us(80);

   for (j=0; j<5; j++) {
      uint8_t result=0;
      for(i=0; i<8; i++) {
         while(!(DHT11_PIN & (1<<DHT11_INPUTPIN)));
         _delay_us(30);
         if(DHT11_PIN & (1<<DHT11_INPUTPIN)) result |= (1<<(7-i));
         while(DHT11_PIN & (1<<DHT11_INPUTPIN));
      }
      bits[j] = result;
   }

   DHT11_DDR |= (1<<DHT11_INPUTPIN);
   DHT11_PORT |= (1<<DHT11_INPUTPIN);

   _delay_ms(100);

   if (bits[0] + bits[1] + bits[2] + bits[3] == bits[4]) {
      if (select == 0) return(bits[2]);
   }

   return DHT11_ERROR;
}

void CLCD_cmd(char cmd) {
   PORTE = 0x00;
   _delay_us(1);

   PORTE = 0x10;
   PORTC = cmd & 0xF0;

   PORTE = 0x00;
   _delay_us(1);

   PORTE = 0x10;
   PORTC = (cmd << 4) & 0xF0;

   PORTE = 0x00;

   _delay_ms(2);
}

void CLCD_data(char data) {
   PORTE = 0x04;

   _delay_us(1);

   PORTE = 0x14;

   PORTC = data & 0xF0;

   PORTE = 0x04;

   _delay_us(1);

   PORTE = 0x14;

   PORTC = (data << 4) & 0xF0;

   PORTE = 0x04;

   _delay_ms(2);
}

void CLCD_puts(char *str) {
   uint8_t i = 0;

   while (*str && i < 16) {
      CLCD_data(*str++);
      i++;
   }

   while (i++ < 16) {
      CLCD_data(' ');
   }
}