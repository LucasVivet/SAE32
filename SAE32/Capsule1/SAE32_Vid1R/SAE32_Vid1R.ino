#include <RH_RF95.h>
#include <M5Stack.h>

RH_RF95 rf95(5,36); 	// Une instance de la couche radio

uint8_t state; 	// état courant
uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN] ; // buffer de réception
uint8_t rxbuflen = RH_RF95_MAX_MESSAGE_LEN; // taille max du buffer de réception
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN; 		// taille trame recue
int rxFrames; 		// nbre de trames regues
uint8_t j;
char temp[255];

#define ECOUTE 1 
#define TEST_RX 2 		
#define AFFICHAGE 3		

void setup ()
{
	M5.begin(); // Initialisation port console pour debug
  M5.Power.begin();
  termInit();

	if (!rf95.init())
    M5.lcd.println("RF95 init erreur");
	else
    M5.lcd.println("RF95 init OK");

	// configuration de la fréquence radio : idem Tx
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); 	// modulation GFSK
  rf95.setFrequency(867.4); 		// canal de la bande ISM 433 MHz

  state = ECOUTE;
  delay(3000); 		// délai pour activer terminal
  M5.lcd.println("boucle principale"); // message de début
  rxFrames = 0; 	// aucune trame reques au début
}

void loop() {
	switch ( state )
	{
		case ECOUTE:
      printString("--------------------------------------------\r");
			printString("ECOUTE\r") ;
			rf95.setModeRx();// mettre le tranceiver en mode réception
			state = TEST_RX; // état suivant
			break;

		case TEST_RX:
			if (rf95.available()) // test si trame disponible en réception
			{
				state = AFFICHAGE;
			}
			break;

		case AFFICHAGE:
			if (rf95.recv(rxbuf, &rxlen))	// récupération de la trame recue dans rxbuf
			{
				rxFrames++; // une trame de + regue
        printString("AFFICHAGE : Trame ");
        sprintf(temp,"%d",rxFrames);
        printString(temp);
        printString("de ");
        sprintf(temp,"%d",rxlen);
        printString(temp);
        printString(" octets : \r ");
        
				for (j=0; j<rxlen; j++)
				{
          sprintf(temp,"%02x",rxbuf[j]);
					printString(temp); 	// affichage du contenu de la trame recue
				}
				printString("\r");
			}
			state = ECOUTE;
			break;

		default:
			break;
	}
}
