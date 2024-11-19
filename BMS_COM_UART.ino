#include <Arduino.h>

// Définir les broches pour UART matériel
#define RX_PIN 16  // Broche de réception (Rx)
#define TX_PIN 17  // Broche d'émission (Tx)

HardwareSerial bmsSerial(1);  // Utilisation de l'UART 1 

byte calculateChecksum(byte *data, int length) {
  byte checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum += data[i];
  }
  return checksum;
}

void sendRequest_info() {
  byte command[13] = {
    0xA5,    // Start Byte
    0x80,    // Host Address
    0x90,    // Command ID (Voltage, Current, SOC)
    0x08,    // Data Length (fixed)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00     // Placeholder for checksum
  };

  byte checksum = calculateChecksum(command, 12);
  command[12] = checksum;  


  Serial.print("Trame envoyée: ");
  for (int i = 0; i < 13; i++) {
  
    if (command[i] < 0x10) {
      Serial.print("0");  
    }
    Serial.print(command[i], HEX);  
    Serial.print(" ");
  }
  Serial.println();  

  bmsSerial.write(command, 13);
}
void readResponse() {
  if (bmsSerial.available() >= 13) {
    byte response[13];
    bmsSerial.readBytes(response, 13);

    Serial.print("Trame brute reçue: ");
    for (int i = 0; i < 13; i++) {
      if (response[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    byte checksum = calculateChecksum(response, 12);
    if (checksum == response[12]) {
    
      // Extraire les valeurs
      int voltage = (response[4] << 8) | response[5];  // Tension
      int current = (response[8] << 8) | response[9];  // Courant
      int soc = (response[10] << 8) | response[11];     // SOC

      // Convertir la tension en volts
      float voltageFloat = voltage / 10.0;

      // Calculer le courant en tenant compte de l'offset de 30000 pour la gestion des courants négatifs
      float currentFloat = 0.0;
      if (current < 30000) {
        // Si la valeur est inférieure à 30000, cela signifie un courant négatif
        currentFloat = (current - 30000) / 10.0;  // Valeur négative
      } else {
        // Si la valeur est supérieure à 30000, c'est un courant positif
        currentFloat = (current - 30000) / 10.0;  // Valeur positive
      }

      // Calculer le SOC (State of Charge)
      float socFloat = soc / 10.0;

      // Affichage des résultats
      Serial.print("Voltage: ");
      Serial.print(voltageFloat, 1);  // Affiche la tension avec 1 chiffre après la virgule
      Serial.println(" V");

      Serial.print("Current: ");
      Serial.print(currentFloat, 1);  // Affiche le courant avec 1 chiffre après la virgule
      Serial.println(" A");

      Serial.print("State of Charge (SOC): ");
      Serial.print(socFloat, 1);     // Affiche le SOC avec 1 chiffre après la virgule
      Serial.println(" %");
      
    } else {
      Serial.println("Checksum error");
    }
  }
}

void sendTemperatureRequest() {
  byte command[13] = {
    0xA5,    // Start Byte
    0x80,    // Host Address
    0x96,    // Command ID pour la température des cellules (getCellTemperature)
    0x08,    // Data Length (fixe)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Data (toujours 0)
    0x00     // Placeholder pour le checksum
  };

  // Calcul du checksum et ajout à la trame
  byte checksum = calculateChecksum(command, 12);
  command[12] = checksum;

  // Affichage de la trame envoyée pour la température
  Serial.print("Trame envoyée (Température): ");
  for (int i = 0; i < 13; i++) {
    if (command[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(command[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Envoi de la trame à l'ESP32
  bmsSerial.write(command, 13);
}

void readTemperatureResponse() {
  if (bmsSerial.available() >= 13) {
    byte response[13];
    bmsSerial.readBytes(response, 13);

    // Affichage de la trame brute reçue en hexadécimal
    Serial.print("Trame brute reçue (Température): ");
    for (int i = 0; i < 13; i++) {
      if (response[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Vérification du checksum
    byte checksum = calculateChecksum(response, 12);
    if (checksum == response[12]) {
      
      // Extraction de la température
      int temperature = (response[4] << 8) | response[5];  // Température dans les octets 4 et 5
      
      // Conversion de la température (en dixièmes de °C)
      float temperatureFloat = temperature / 10.0;

      // Affichage de la température
      Serial.print("Temperature: ");
      Serial.print(temperatureFloat, 1);  // 1 décimale
      Serial.println(" °C");

    } else {
      Serial.println("Checksum error");
    }
  }
}

// Fonction d'envoi de la commande 0x94 (Status Information)
void sendStatusRequest() {
  byte command[13] = {
    0xA5,    // Start Byte
    0x80,    // Host Address
    0x94,    // Command ID pour obtenir les informations de statut
    0x08,    // Data Length (fixe)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00     // Placeholder pour checksum
  };

  byte checksum = calculateChecksum(command, 12);
  command[12] = checksum;  

  Serial.print("Trame envoyée (Status Information): ");
  for (int i = 0; i < 13; i++) {
    if (command[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(command[i], HEX);  
    Serial.print(" ");
  }
  Serial.println();

  bmsSerial.write(command, 13);
}

void readStatusResponse() {
  if (bmsSerial.available() >= 13) {
    byte response[13];
    bmsSerial.readBytes(response, 13);

    // Affichage de la trame brute reçue en hexadécimal
    Serial.print("Trame brute reçue (Status Information): ");
    for (int i = 0; i < 13; i++) {
      if (response[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    byte checksum = calculateChecksum(response, 12);
    if (checksum == response[12]) {
      
      // Extraction des informations de statut
      int numberOfCells = response[4];  // Nombre de cellules (octet 4)
      int numOfTempSensors = response[5]; // Nombre de capteurs de température (octet 5)
      bool chargeState = response[6] == 1;  // État de charge (octet 6)
      bool loadState = response[7] == 1;    // État de charge/décharge (octet 7)

      // Extraction des états des DIO (octets 8-11)
      bool dIO[8];
      for (int i = 0; i < 8; i++) {
        dIO[i] = (response[8 + i / 4] >> (i % 4)) & 1;  // Extraire les 8 bits des DIO
      }

      // Extraction des cycles BMS (octets 12 et 13)
      int bmsCycles = (response[9] << 8) | response[10];  // Nombre de cycles (2 octets)

      // Affichage des informations extraites
      Serial.print("Number of Cells: ");
      Serial.println(numberOfCells);

      Serial.print("Number of Temperature Sensors: ");
      Serial.println(numOfTempSensors);

      Serial.print("Charge State (0 = disconnected, 1 = connected): ");
      Serial.println(chargeState);

      Serial.print("Load State (0 = disconnected, 1 = connected): ");
      Serial.println(loadState);

      Serial.print("DIO States: ");
      for (int i = 0; i < 8; i++) {
        Serial.print(dIO[i]);
        Serial.print(" ");
      }
      Serial.println();

      Serial.print("BMS Cycles: ");
      Serial.println(bmsCycles);

    } else {
      Serial.println("Checksum error");
    }
  }
}


void setup() {
  Serial.begin(115200);  
  bmsSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);  
  delay(100); 
}

void loop() {  
  sendTemperatureRequest();delay(100);
  readTemperatureResponse();delay(100);
  sendRequest_info();delay(100);
  readResponse();delay(100);
  sendStatusRequest();delay(100);
  readStatusResponse();
  delay(2000);
}
