#include <RH_RF95.h>
#include <M5Stack.h>

RH_RF95 rf95(5,36); 									// Une instance de la couche radio

uint8_t state; 								// état courant
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN] ; 		// buffer de trame en émission
uint8_t txbuflen = RH_RF95_MAX_MESSAGE_LEN; 	// taille de la trame à émettre
uint8_t i;
char temp[255];

// déclaration des noms et numéros de chaque état
#define EMISSION 0
#define DELAI	 1

#define canal 0 			 // déclaration du canal

void setup ()
{
	M5.begin() ;  // Initialisation port console pour debug
  M5.Power.begin();
  termInit();	 

	if (!rf95.init())
		M5.lcd.println("RF95 init erreur") ;
	else
		M5.lcd.println("RF95 init OK");

	// configuration de la fréquence radio
	rf95.setTxPower(10);
	rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
	rf95.setFrequency(867.4);

	state = EMISSION; 		// état de départ

	delay(3000); 			// délai au début pour laisser le temps de lancer le terminal (putty)
	M5.lcd.println("boucle principale");

}

void loop() {
	switch ( state ) {
	case EMISSION: 					// code source de 1’état EMISSION
    printString("--------------------------------------------\r");
		printString("EMISSION\r");
    sprintf(temp,"%d",txbuflen);
    printString("Taille de la trame : ");
    printString(temp);
    printString("\r");
		digitalWrite(13, HIGH) ; 	// pour aider a débugger

		// formater une trame a émettre de 32 octets
		txbuf[0] = 0x0AA;
    txbuf[1] = 0x055;

    for (i=0; i<32;i++) txbuf[i] = i;
		rf95.send(txbuf, 32); 		// émission

		rf95.waitPacketSent() ; 	// attente fin d’émission

		digitalWrite(13, LOW);		// pour aider Aa débugger

		state = DELAI; 				// transition vers état suivant
		break;

	case DELAI: // code source de l’état DELAI
		printString("ATTENTE\r") ;
		delay (1000) ; 				// 1s avant d’émettre la trame suivante
		state = EMISSION; 			// transition vers état suivant
		break;

	default:
		break;
	}
}
