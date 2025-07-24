/*간단한 슬립 모드 테스트 (방법-2 기반):
기본 Arduino 환경에서 슬립 모드 구현
스위치 입력 또는 1초 타이머로 슬립 모드에서 깨어난 후 LED를 켰다 끄고,
다시 슬립 모드로 들어가는 기능 구현
*/

#include <avr/sleep.h>     // 절전 모드 설정을 위한 라이브러리
#include <avr/wdt.h>       // 워치독 타이머 사용을 위한 라이브러리

// 핀 설정
const int wakeUpPin = 2;   // 인터럽트 0번 핀 (디지털 2번)
const int LED = 13;        // 내장 LED 핀

// 슬립 모드에서 깨어날 때 사용되는 플래그들
volatile bool wakeUpFlag = false;
volatile bool timerWakeUp = false;

//
// 초기 설정 함수 (1회 실행)
// - LED/스위치 핀 설정, 워치독 타이머 초기화
//
void setup() {
    pinMode(wakeUpPin, INPUT_PULLUP);  // 스위치 핀 입력 설정 (내부 풀업 저항 사용)
    pinMode(LED, OUTPUT);              // LED 핀 출력 설정

    // 워치독 타이머 초기화
    MCUSR &= ~(1 << WDRF);             // 워치독 리셋 플래그 클리어
    WDTCSR |= (1 << WDCE) | (1 << WDE); // 워치독 변경 허용
    WDTCSR = 0x00;                     // 워치독 비활성화

    delay(3000);  // 시작 후 3초 대기 (전류 측정을 위한 충분한 시간)
}

//
// 메인 루프 함수
// - LED를 3초간 켠 후 슬립 모드로 진입
// - 스위치가 눌리면 깨어나서 반복
//
void loop() {

    digitalWrite(LED, HIGH);  // LED 켜기
    delay(3000);              // 3초 동안 LED 켜둠
    

    digitalWrite(LED, LOW);   // LED 끄기
    delay(100);               // 시리얼 출력 완료 대기
    
    goToSleep();              // 슬립 모드 진입
    

    delay(100);               // 디바운싱을 위한 짧은 대기
}

//
// 슬립 모드 진입 함수 (방법-2 기반 + 1초 타이머 추가)
// - 기본 Arduino 환경에서 안정적인 슬립 구현
// - 스위치 인터럽트 또는 1초 타이머로 깨어남
//
void goToSleep() {
    // 플래그 초기화
    timerWakeUp = false;
    
    // 워치독 타이머 설정 (1초)
    wdt_reset();                           // 워치독 리셋
    WDTCSR |= (1 << WDCE) | (1 << WDE);    // 워치독 변경 허용
    WDTCSR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1); // 1초 간격, 인터럽트 모드
    
    sleep_enable();                        // 슬립 활성화
    attachInterrupt(0, wakeUp, LOW);       // 인터럽트 0 (핀 2)에 인터럽트 설정
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // 가장 전력 소모가 적은 모드 설정
    cli();                                 // 인터럽트 비활성화
    
    byte oldADC = ADCSRA;                  // ADC 제어 레지스터 저장
    ADCSRA = 0;                            // ADC 비활성화 (전력 절약)
    
    // sleep_bod_disable();                // ATmega168에서는 지원하지 않음 (주석 처리)
    
    sei();                                 // 인터럽트 활성화
    sleep_cpu();                           // 슬립 모드 진입... z z z z z
    
    // 여기서 깨어남 (스위치 또는 타이머에 의해)
    sleep_disable();                       // 슬립 비활성화
    detachInterrupt(0);                    // 인터럽트 해제
    ADCSRA = oldADC;                       // ADC 설정 복원
    
    // 워치독 타이머 비활성화
    WDTCSR |= (1 << WDCE) | (1 << WDE);    // 워치독 변경 허용
    WDTCSR = 0x00;                         // 워치독 완전히 비활성화
}

//
// 인터럽트 핸들러 (스위치가 눌리면 깨어남)
//
void wakeUp() {
    wakeUpFlag = true;  // 깨어났다는 플래그 설정
    // 필요하면 여기에 추가 처리 코드 작성
}

//
// 워치독 타이머 인터럽트 핸들러 (1초 후 자동으로 깨어남)
//
ISR(WDT_vect) {
    timerWakeUp = true;  // 타이머에 의해 깨어났다는 플래그 설정
}
