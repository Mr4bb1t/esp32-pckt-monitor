#include <TFT_eSPI.h>
#include <WiFi.h>
#include "esp_wifi.h"  // Inclua a biblioteca esp_wifi.h

#define BUTTON_UP 22
#define BUTTON_DOWN 21

TFT_eSPI tft = TFT_eSPI();  // Invoke library

int currentChannel = 1;
const int maxChannel = 14;
const int minChannel = 1;
volatile int packetCount = 0;
int displayedPacketCount = 0;  // Variável para armazenar o valor exibido
bool upPressed = false;
bool downPressed = false;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 200;  // Intervalo de atualização em milissegundos (200ms)
const unsigned long debounceDelay = 200;  // Debounce delay em milissegundos
unsigned long lastButtonPressTime = 0;
bool monitoring = false;  // Flag para indicar se o monitoramento está ativo

void setup() {
  Serial.begin(115200);

  // Inicializa a tela
  tft.init();
  tft.setRotation(3);  // Define a rotação para 3
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(3);  // Aumenta o tamanho das letras

  // Inicializa os pinos dos botões
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Exibe a tela inicial
  displayWelcomeScreen();
}

void loop() {
  if (!monitoring) {
    // Espera por qualquer botão ser pressionado para iniciar o monitoramento
    if ((digitalRead(BUTTON_UP) == LOW || digitalRead(BUTTON_DOWN) == LOW) && (millis() - lastButtonPressTime > debounceDelay)) {
      lastButtonPressTime = millis();
      monitoring = true;
      // Inicializa o WiFi em modo promiscuous
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      esp_wifi_set_promiscuous(true);
      esp_wifi_set_promiscuous_rx_cb(&promiscuous_callback);

      // Exibe o canal inicial
      setChannel(currentChannel);
      displayChannel();
    }
    return;
  }

  // Detecta clique rápido no botão UP
  if (digitalRead(BUTTON_UP) == LOW && (millis() - lastButtonPressTime > debounceDelay)) {
    lastButtonPressTime = millis();
    currentChannel = (currentChannel % maxChannel) + 1;
    setChannel(currentChannel);
    displayChannel();
  }

  // Detecta clique rápido no botão DOWN
  if (digitalRead(BUTTON_DOWN) == LOW && (millis() - lastButtonPressTime > debounceDelay)) {
    lastButtonPressTime = millis();
    currentChannel = (currentChannel - 2 + maxChannel) % maxChannel + 1;
    setChannel(currentChannel);
    displayChannel();
  }

  // Atualiza a contagem de pacotes na tela em tempo real a cada intervalo definido
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();
    displayedPacketCount = (packetCount * 1000) / updateInterval;  // Calcula a taxa de pacotes por segundo
    packetCount = 0;  // Reseta a contagem de pacotes
    displayPacketCount();
  }
}

void setChannel(int channel) {
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  packetCount = 0;  // Reseta a contagem de pacotes ao mudar de canal
  displayedPacketCount = 0;  // Reseta a última contagem de pacotes para forçar a atualização
}

void displayWelcomeScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Texto "r4bb1t"
  tft.setTextSize(3);
  int textWidth = tft.textWidth("r4bb1t");
  int x = (tft.width() - textWidth) / 2;
  tft.setCursor(x, 50);  // Centraliza o texto
  tft.println("r4bb1t");

  // Texto "Packet Monitor"
  tft.setTextSize(2);
  textWidth = tft.textWidth("Packet Monitor");
  x = (tft.width() - textWidth) / 2 + 3;  // Move 3 pixels à frente
  tft.setCursor(x, 100);  // Centraliza o texto
  tft.println("Packet Monitor");

  // Texto "Aperte qualquer botao para iniciar"
  tft.setTextSize(2);
  textWidth = tft.textWidth("Aperte qualquer");
  x = (tft.width() - textWidth) / 2;
  tft.setCursor(x, 160);  // Centraliza o texto
  tft.println("Aperte qualquer");

  textWidth = tft.textWidth("botao para iniciar");
  x = (tft.width() - textWidth) / 2;
  tft.setCursor(x, 190);  // Centraliza o texto
  tft.println("botao para iniciar");
}

void displayChannel() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(20, 30);  // Ajusta a posição do cursor
  tft.setTextSize(3);  // Define o tamanho do texto
  tft.printf("Canal");

  tft.setCursor(150, 30);  // Ajusta a posição do número do canal
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.printf("%d", currentChannel);

  // Desenha uma borda ao redor do texto
  tft.drawRect(10, 10, 220, 100, TFT_GREEN);
}

void displayPacketCount() {
  tft.fillRect(10, 120, 220, 100, TFT_BLACK);  // Limpa a área da contagem de pacotes
  tft.setCursor(20, 140);  // Ajusta a posição do cursor abaixo do número do canal
  tft.setTextSize(3);  // Define o tamanho do texto
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.printf("Pacotes");

  tft.setCursor(150, 140);  // Ajusta a posição do número de pacotes
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.printf("%d", displayedPacketCount);

  // Desenha uma borda ao redor do texto
  tft.drawRect(10, 120, 220, 100, TFT_GREEN);
}

void promiscuous_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  packetCount++;
}