# 🚪 Smarthome Entrance & Environment Monitoring System

ATmega MCU를 기반으로 다중 센서(초음파, 온습도, 조도)를 활용하여 자동 출입 제어 및 주변 환경 모니터링을 수행하는 임베디드 시스템입니다.

## 📌 주요 기능 (Features)
- **자동 접근 감지 (Ultrasonic):** 사용자가 일정 거리(600mm) 이내로 접근하면 시스템이 활성화됩니다.
- **자동문 제어 (Motor):** 사용자 감지 시 모터가 작동(CW)하여 문을 개방하며, 일정 시간 감지되지 않으면 정지합니다.
- **환경 정보 제공 (DHT11 & CLCD):** 시스템 활성화 시 환영 문구("WELCOME")와 현재 온도(NOW Temp)를 CLCD에 출력합니다.
- **스마트 조명 제어 (CDS & LED):** 주변 조도를 실시간으로 파악하여 어두울 경우(임계값 550 이하) 자동으로 LED 조명을 켭니다.

## 🛠 하드웨어 구성 및 핀맵 (Pin Mapping)

| 모듈 / 센서 | MCU 포트 | 핀 (Pin) | 비고 |
| :--- | :--- | :--- | :--- |
| **LED 제어** | `PORTA` | `PA0~PA3` | 조도 및 시스템 상태 표시 |
| **모터 제어** | `PORTB` | `PB4~PB7` | L298N 등 모터 드라이버 연결 |
| **CLCD Data** | `PORTC` | `PC4~PC7` | 4-bit 모드 데이터 통신 |
| **초음파 (HC-SR04)** | `PORTD` | `PD1(Trig)`, `PD2(Echo)` | Timer1을 이용한 거리 측정 |
| **CLCD Control** | `PORTE` | `PE2(RS)`, `PE4(EN)` | 디스플레이 제어 신호 |
| **조도 센서 (CDS)** | `PORTF` | `PF0 (ADC0)` | ADC 10-bit 변환 |
| **온습도 (DHT11)** | `PORTG` | `PG0` | 1-wire 통신 |

## ⚙️ 시스템 동작 흐름도 (Workflow)
1. **초기화:** ADC, CLCD, Timer, 각 I/O 포트 초기화.
2. **거리 측정 (Loop):** 초음파 센서로 전방 거리를 지속해서 측정.
3. **감지 시 (Distance < 60cm):** 
   - 모터 정방향 회전.
   - 상태 LED 점등.
   - DHT11 데이터 읽기 및 CLCD에 출력.
4. **미감지 시:** 카운터가 일정 수치에 도달하면 모터 정지 및 CLCD 화면 클리어.
5. **조도 판별:** 위 과정과 독립적으로 ADC 값을 읽어 주변이 어두우면 보조 LED를 점등.

## 🚀 개발 환경 (Environment)
- **MCU:** ATmega128 (추정)
- **Clock:** 16 MHz (`F_CPU 16000000UL`)
- **IDE/Compiler:** Microchip Studio / AVR-GCC
