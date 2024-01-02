#include <RH_RF95.h>
#include <M5Stack.h>

RH_RF95 rf95(5,36); 

uint8_t rxbuf [RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t txbuf [RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t state ,RxSeq,TxSeq,credit ,backoff ,NewFrame ,EIT;
uint32_t attente;
uint8_t A3;

#define E0 0
#define E1 1
#define E2 2
#define E3 3
#define E4 4
#define E5 5
#define TYPE_DATA 1
#define TYPE_ACK 2
#define TIMEOUT_ACK 40

#define MyAdr 1 // adresse locale du noeud Tx1 ou Tx2 ou Tx9

void setup()
{
M5.begin();
M5.Power.begin();
if (!rf95.init()) M5.lcd.println("RF95 init failed");
else M5.lcd.println('"RF95 init OK');
rf95.setTxPower (10) ;
rf95.setModemConfig (RH_RF95::Bw125Cr45Sf128) ;
rf95.setFrequency(867.4);

state = E0;
delay (3000) ;
TxSeq = 0; credit = 5;
M5.lcd.println("Boucle principale");
NewFrame = 1;
randomSeed(analogRead(A3)); // init générateur aléatoire sur entrée en l'air

}


void loop() {
 switch (state)
{
case E0:

  if (NewFrame == 1) // loi d'arrivée aléatoire du flux de trames à envoyer
    {
      EIT = random(5,100);
      delay(EIT); 
      M5.lcd.printf(" EIT : %d ",EIT); M5.lcd.println();
    }
  M5.lcd.printf ("EMISSION %d : ",TxSeq) ;
  txbuf[0] = MyAdr; // @S : moi
  txbuf[1] = 0; // @D : le puits
  txbuf[2] = TYPE_DATA; // type de message
  txbuf[3] = TxSeq;
  txbuf[4] = 0x0AA;
  txbuf[5] = 0x055;
  rf95.send(txbuf, 6); // envoie de données (une trame de 6 octets)
  rf95.waitPacketSent();
  credit--; 
  state = E1;
break;

case E1 :
  attente = millis()+ TIMEOUT_ACK;
  state = E2;
break;

case E2:
  rf95.setModeRx();
  state = E3;
break;

case E3:
  if (millis() > attente)
    { state = E5;}
  else 
    {
      if (rf95.recv(rxbuf, &rxlen))
      {
        if (((rxbuf[2]) == TYPE_ACK) && (rxbuf[3] == TxSeq) && (rxbuf[1] == MyAdr))
          {
            state = E4;      
          }
        else state = E2;
      }       
    }
break;

case E4:
  M5.lcd.println("ACK_RECU");
  state = E0; TxSeq++; credit = 5;
break;

case E5:
  M5.lcd.println("E5");
  if (credit == 0)
    {
      M5.lcd.println("ECHEC");
      state = E0; NewFrame = 1;
      credit = 5; TxSeq++;
      break;  
    }    
  else
    {
      M5.lcd.printf("Collision ? Nouvelle tentative n° %d", 5-credit);
      state = E0; NewFrame = 0;
      backoff = random(0,100);
      delay(backoff);
      M5.lcd.printf("Backoff : %d", backoff);
      M5.lcd.println();
     }
    break;
  default:
    state = E0;
    break;
  }
}
