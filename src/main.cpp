#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

typedef struct __attribute__((packed)) mensagem {
  float dados[4+3+3+3+3+1];
  char info[35]; // dados para informacao;
} mensagem;
typedef struct __attribute__((packed)) controle {
  uint8_t dados;
} controle;

uint8_t confirmacao = 0;

uint8_t _broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // Para cada pino
  mensagem msg;
  memcpy(&msg, data, sizeof(mensagem));
  if (strlen(msg.info) < 2) {
    Serial.print(",");
    for (size_t i = 0; i < sizeof(msg.dados) / sizeof(*msg.dados); i++) {
      Serial.print(msg.dados[i], 6);
      Serial.print(",");
    }
  } else {
    Serial.println(msg.info);
  }
  Serial.println();
  delay(5);
}

void setup() {
  // disconeta WIfi para evitar instabilidades
  WiFi.disconnect();
  // inicializa wifi em modo station
  WiFi.mode(WIFI_STA);
  // tenta estabelecer coneccao
  while (esp_now_init() != ESP_OK) {
    ESP.restart();
  };
  Serial.println("ESP Inicializado");
  // Criamos uma variável que irá guardar as informações do slave
  esp_now_peer_info_t slave;
  slave.channel = 1; // Informamos o canal
  slave.encrypt = 0; // 0 para não usar criptografia ou 1 para usar
  // Copia o endereço do array para a estrutura
  memcpy(slave.peer_addr, _broadcast, sizeof(_broadcast));
  esp_now_add_peer(&slave); // Adiciona o slave
  pinMode(15, OUTPUT);
  pinMode(13, INPUT);
  Serial.begin(115200);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  esp_now_init();
  if (esp_now_init() != ESP_OK) {
    ESP.restart();
  }
  Serial.println("OK");
  esp_now_register_recv_cb(OnDataRecv);
}
void envia(int a) {
  controle ctrl;
  ctrl.dados = a;
  uint8_t bs[sizeof(controle)];
  memcpy(&bs, &ctrl, sizeof(ctrl));
  esp_now_send(_broadcast, bs, sizeof(bs));
}

void loop() {
  if (digitalRead(13) != 0) {
    confirmacao = 1;
    envia(confirmacao);
    delay(1);
  } else if (confirmacao == 1) {
    envia(0);
    confirmacao = 0;
    delay(1);
  } else {
    delay(1);
  }
}
