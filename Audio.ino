#include <driver/i2s.h>


//---------- MICROPHONE PINS
const int BUZZER_PIN = 27; 
//---------- 
const int I2S_SD = 4;
const int I2S_WS = 5;
const int I2S_SCK = 14;

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000  // 16 kHz
#define SAMPLE_BUFFER_SIZE 512

const long INTERVAL_AUDIO = 20;
unsigned long previousMillisAudio = 0;

void setupAudio() {
  pinMode(BUZZER_PIN, OUTPUT);
  
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // INMP441 -> Left channel
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  // Install and start I2S driver
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

void loopAudio(unsigned long currentMillis) {
  if (currentMillis - previousMillisAudio < INTERVAL_AUDIO) { return; }
  previousMillisAudio = currentMillis;

  int32_t samples[SAMPLE_BUFFER_SIZE];
  size_t bytes_read;

  // Read samples from the I2S bus
  i2s_read(I2S_PORT, (void*)samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);

  // Compute an RMS value for a simple amplitude measure
  double sum_squares = 0;
  for (int i = 0; i < samples_read; i++) {
    // Convert 32-bit sample to 24-bit (mic data is left-aligned)
    int32_t sample = samples[i] >> 8;
    sum_squares += (double)sample * (double)sample;
  }

  double rms = sqrt(sum_squares / samples_read);
  double db = 20 * log10(rms / 2147483647.0) + 120;  // approximate dB scale

  // Print a single value for Arduino Serial Plotter
  Serial.println(db);
}