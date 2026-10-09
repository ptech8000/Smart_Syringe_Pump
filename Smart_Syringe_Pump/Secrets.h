#ifndef SECRETS_H
#define SECRETS_H

// Fill these in locally. Do not share this file or commit it to a public repo.
// Your previous Wi-Fi and MQTT passwords were pasted in plain text, so rotate them
// (HiveMQ Cloud > Access Management) and use the new ones here.

const char* WIFI_SSID     = "*******";
const char* WIFI_PASSWORD = "*******";

const char* MQTT_HOST     = "your_mqtt_url.s1.eu.hivemq.cloud";
const uint16_t MQTT_PORT  = ****;   // TLS port
const char* MQTT_USERNAME = "********";
const char* MQTT_PASSWORD = "*********";

// Paste the full root CA certificate (ISRG Root X1) from your working v1 file
// between the EOF markers, or re-fetch it with:
//   openssl s_client -connect <cluster>:8883 -showcerts
const char* HIVEMQ_ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----

copy and paste your certificate


-----END CERTIFICATE-----
)EOF";
#endif
