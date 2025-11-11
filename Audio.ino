#include <driver/i2s_std.h>
#include <driver/gpio.h>

//---------- MICROPHONE PINS
const int BUZZER_PIN = 27; 
//---------- 
const int I2S_SD = 4;
const int I2S_WS = 5;
const int I2S_SCK = 14;

#define SAMPLE_RATE 16000  // 16 kHz
#define SAMPLE_BUFFER_SIZE 512

const long INTERVAL_AUDIO = 20;
unsigned long previousMillisAudio = 0;

i2s_chan_handle_t rx_handle = NULL;

// Buzzer melody configuration
const int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};  // C, D, E, F, G, A, B, C (scale)
const int noteDurations[] = {200, 200, 200, 200, 200, 200, 200, 400};  // Duration for each note in ms
const int melodyLength = 8;

int currentNote = 0;
unsigned long noteStartTime = 0;
bool melodyPlaying = false;
bool melodyFinished = false;

void setupAudio() {
  pinMode(BUZZER_PIN, OUTPUT);
  setupMicrophone();
}

void loopAudio(unsigned long currentMillis) {
  loopMicrophone(currentMillis);
  loopBuzzer(currentMillis);
}

void setupMicrophone() {
  // New I2S driver API (compatible with adc_continuous)
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  
  esp_err_t err = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
  if (err != ESP_OK) {
    Serial.print("Failed to create I2S RX channel: ");
    Serial.println(err);
    return;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2S_SCK,
      .ws = (gpio_num_t)I2S_WS,
      .dout = I2S_GPIO_UNUSED,
      .din = (gpio_num_t)I2S_SD,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false,
      },
    },
  };

  err = i2s_channel_init_std_mode(rx_handle, &std_cfg);
  if (err != ESP_OK) {
    Serial.print("Failed to initialize I2S STD mode: ");
    Serial.println(err);
    return;
  }

  err = i2s_channel_enable(rx_handle);
  if (err != ESP_OK) {
    Serial.print("Failed to enable I2S channel: ");
    Serial.println(err);
    return;
  }

  Serial.println("I2S microphone initialized successfully (new driver)");
}

void loopMicrophone(unsigned long currentMillis) {
  if (rx_handle == NULL) return; // Not initialized
  if (currentMillis - previousMillisAudio < INTERVAL_AUDIO) { return; }
  previousMillisAudio = currentMillis;

  int32_t samples[SAMPLE_BUFFER_SIZE];
  size_t bytes_read = 0;

  // Read samples from the I2S bus using new API
  esp_err_t err = i2s_channel_read(rx_handle, (void*)samples, sizeof(samples), &bytes_read, 0);
  if (err != ESP_OK || bytes_read == 0) {
    return; // No data available or error
  }

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

void loopBuzzer(unsigned long currentMillis) {
  if (melodyFinished) return;  // Melody already played
  
  if (!melodyPlaying) {
    // Start playing the melody
    melodyPlaying = true;
    currentNote = 0;
    noteStartTime = currentMillis;
    tone(BUZZER_PIN, melody[currentNote]);
    Serial.println("Melody started");
  } else {
    // Check if current note duration has elapsed
    if (currentMillis - noteStartTime >= noteDurations[currentNote]) {
      currentNote++;
      
      if (currentNote < melodyLength) {
        // Play next note
        tone(BUZZER_PIN, melody[currentNote]);
        noteStartTime = currentMillis;
      } else {
        // Melody finished
        noTone(BUZZER_PIN);
        melodyFinished = true;
        Serial.println("Melody finished");
        // Uncomment to repeat: melodyFinished = false; melodyPlaying = false;
      }
    }
  }
}