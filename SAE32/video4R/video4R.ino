#include <RH_RF95.h>
#include <M5Stack.h>

RH_RF95 rf95(5,36); 

uint8_t state,i;
uint8_t rxbuf [RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf [RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t RxSeq;
uint32_t attente;

#define E0 0
#define E1 1
#define E2 2
#define E3 3

#define TYPE_DATA 1
#define TYPE_ACK 2

#define MyAdr 0

void setup() {
  M5.begin();
  M5.Power.begin();
  if (!rf95.init()) M5.lcd.println("RF95 init failed");
  else M5.lcd.println('"RF95 init OK');
  rf95.setTxPower (10) ;
  rf95.setModemConfig (RH_RF95::Bw125Cr45Sf128) ;
  rf95.setFrequency(867.4);

  state = E0;
  delay(3000);
  M5.lcd.println("Boucle principale");
  RxSeq = 255;
}

void loop() {
int j;
  switch (state)
  {
    case E0:
    delay(3000);
      M5.lcd.println("E0");
      rf95.setModeRx();
      state = E1;
      break;

    case E1:
      rxlen = RH_RF95_MAX_MESSAGE_LEN;
      if (rf95.recv(rxbuf, &rxlen))  
      {
        if ((rxbuf[2] == TYPE_DATA) && (rxbuf[1]== MyAdr))
          {
            state = E2;
          }      
        }
      else 
      {
       state = E0;
      }
      break;

    case E2:
      if (RxSeq != rxbuf[3])
        {
          RxSeq = rxbuf[3];
          M5.lcd.printf("[%d]", RxSeq);
          M5.lcd.printf("DATA de %d octets :", rxlen);
          M5.lcd.println();
          for (j=0; j<rxlen; j++)
            {
              M5.lcd.printf("%02x|", rxbuf[j]);
            }
          M5.lcd.println();
        }
      else
        {
          M5.lcd.printf("Duplication");
        }
      state = E3;
      break; 
    
    case E3:
      M5.lcd.println("E3");
      txbuf[0] = MyAdr;
      txbuf[1] = rxbuf[0];
      txbuf[2] = TYPE_ACK;
      txbuf[3] = rxbuf[3];
      rf95.send(txbuf, 4);
      rf95.waitPacketSent();

      state = E0;
      break;

    default:
      state = E0;
      break;  
  }
}
