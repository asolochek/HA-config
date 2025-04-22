
#include "qwiic_led_stick.h"

namespace esphome {
namespace qwiic_led_stick {


QwiicLEDStick::QwiicLEDStick(uint16_t num_leds) : i2c::I2CDevice() {
  this->num_leds_ = num_leds;
  this->buffer_size_ = this->num_leds_ * 4 + 8;
  ExternalRAMAllocator<uint8_t> allocator(ExternalRAMAllocator<uint8_t>::ALLOW_FAILURE);
  this->buf_ = allocator.allocate(this->buffer_size_);
  if (this->buf_ == nullptr) {
    ESP_LOGE("qwiic_led_stick", "Error allocating buffer");
    this->mark_failed();
    return;
  }

  this->effect_data_ = allocator.allocate(this->num_leds_);
  if (this->effect_data_ == nullptr) {
    ESP_LOGE("qwiic_led_stick", "Error allocating effect data");
    this->mark_failed();
    return;
  }
  memset(this->buf_, 0x00, this->buffer_size_);
  memset(this->buf_, 0, 4);

}

void QwiicLEDStick::setup() {
  i2c::ErrorCode ret = i2c::NO_ERROR;
  uint8_t buf[2] = {COMMAND_CHANGE_LED_LENGTH, this->num_leds_};
  ret = this->write(buf, 2);
  //  ret = this->write_register(COMMAND_CHANGE_LED_LENGTH, this->num_leds_);
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting LED length: %d", ret);
    this->mark_failed();
    return;
  }

}


void QwiicLEDStick::write_state(light::LightState *state) {
  if (this->is_failed()) {
    return;
  }

  auto color = state->current_values;
  static uint8_t led = 0;


  ESP_LOGVV(TAG, "LED: %d on? %s color: %f %f %f brightness: %f", led, color.is_on() ? "yes" : "no", color.get_red(), color.get_green(), color.get_blue(), color.get_brightness());
  led++;
  if (led >= this->num_leds_) {
    led = 0;
  }

  if (ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE) {
    ESP_LOGVV(TAG, "buffer size: %d", this->buffer_size_);
    for (int i = 0; i < this->buffer_size_; i++) {
      ESP_LOGVV(TAG, "buf[%d]: %d", i, this->buf_[i]);
    }
  }

  for (int i = 0; i < this->num_leds_; i++) {
    this->blue_array_[i] = this->buf_[i * 4 + 5];
    this->green_array_[i] = this->buf_[i * 4 + 6];
    this->red_array_[i] = this->buf_[i * 4 + 7];
  }

  // for (int i = 0; i < this->num_leds_; i++) {
  //   auto color = state->current_values;
  //   //ESP_LOGVV(TAG, "LED: %d on? %s color: %f %f %f brightness: %f", i, color.is_on() ? "yes" : "no", color.get_red(), color.get_green(), color.get_blue(), color.get_brightness());

  //   on_array[i] = color.is_on();

  //   if (color.is_on()) {
  //     float brightness = color.get_brightness();
  //     red_array[i] = color.get_red() * brightness * 255;
  //     green_array[i] = color.get_green() * brightness * 255;
  //     blue_array[i] = color.get_blue() * brightness * 255;

  //   } else {
  //     red_array[i] = 0;
  //     green_array[i] = 0;
  //     blue_array[i] = 0;
  //     brightness_array[i] = 0;
  //   }
  // }

  bool all_off = true;
  for (int i = 0; i < this->buffer_size_; i++) {
    if (this->buf_[i]) {
      all_off = false;
      break;
    }
  }

  if (all_off) {
    this->all_off();
  }
  else {
    this->set_color(this->red_array_, this->green_array_, this->blue_array_);
  }

  #if 0
  for (int i = 0; i < this->num_leds_; i++) {
    auto color = state->current_values;
    bool on = color.is_on();
    if (!on) {
      this->set_color(i, 0, 0, 0);
      this->set_brightness(i, 0);
      continue;
    }
    uint8_t brightness = color.get_brightness() * 255;
    uint8_t red = color.get_red() * 255;
    uint8_t green = color.get_green() * 255;
    uint8_t blue = color.get_blue() * 255;
    this->set_color(i, red, green, blue);
    this->set_brightness(i, color.get_brightness() * 255);
  }
  #endif

}


float QwiicLEDStick::get_setup_priority() const {
  return setup_priority::DATA;
}


int32_t QwiicLEDStick::size() const {
  return this->num_leds_;
}


light::LightTraits QwiicLEDStick::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}


void QwiicLEDStick::dump_config() {
  ESP_LOGCONFIG("qwiic_led_stick", "Qwiic LED Stick");

}


void QwiicLEDStick::clear_effect_data() {
  memset(this->effect_data_, 0, this->num_leds_ * sizeof(this->effect_data_[0]));
}


light::ESPColorView QwiicLEDStick::get_view_internal(int32_t index) const {
  size_t pos = index * 4 + 5;
  return {this->buf_ + pos + 2,       
          this->buf_ + pos + 1, 
          this->buf_ + pos + 0, 
          nullptr,
          this->effect_data_ + index, 
          &this->correction_};
}


void QwiicLEDStick::set_color(uint8_t led, uint8_t red, uint8_t green, uint8_t blue) {
  uint8_t buf[5];
  i2c::ErrorCode ret = i2c::NO_ERROR;

  // First, boundary check
  if (red > 255)
    red = 255;
  if (red < 0)
    red = 0;
  if (green > 255)
    green = 255;
  if (green < 0)
    green = 0;
  if (blue > 255)
    blue = 255;
  if (blue < 0)
    blue = 0;

  buf[0] = COMMAND_WRITE_SINGLE_LED_COLOR;
  buf[1] = led;
  buf[2] = red;
  buf[3] = green;
  buf[4] = blue;

//  ESP_LOGVV(TAG, "Writing to 0x%.2X: %d %d %d %d", this->address_, led, red, green, blue);

  ret = this->write(buf, 5);
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting color: %s", i2cErrorToString(ret));
  }
}

void QwiicLEDStick::set_color(uint8_t red[], uint8_t green[], uint8_t blue[]) {
  uint8_t buf[this->num_leds_ + 3];
  i2c::ErrorCode ret = i2c::NO_ERROR;


  buf[0] = COMMAND_WRITE_RED_ARRAY;
  buf[1] = this->num_leds_;
  buf[2] = 0; // offset is always 0 since we send all LEDS at once
  memcpy(buf + 3, red, this->num_leds_);
  ret = this->write(buf, sizeof(buf));
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting red array: %s", i2cErrorToString(ret));
  }

  buf[0] = COMMAND_WRITE_GREEN_ARRAY;
  buf[1] = this->num_leds_;
  buf[2] = 0; // offset
  memcpy(buf + 3, green, this->num_leds_);
  ret = this->write(buf, sizeof(buf));
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting green array: %s", i2cErrorToString(ret));
  }

  buf[0] = COMMAND_WRITE_BLUE_ARRAY;
  buf[1] = this->num_leds_;
  buf[2] = 0; // offset
  memcpy(buf + 3, blue, this->num_leds_);
  ret = this->write(buf, sizeof(buf));
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting blue array: %s", i2cErrorToString(ret));
  }

}

void QwiicLEDStick::set_brightness(uint8_t led, uint8_t brightness) {
  uint8_t buf[3];
  i2c::ErrorCode ret = i2c::NO_ERROR;

  // First, boundary check
  if (brightness > 255)
    brightness = 255;
  if (brightness < 0)
    brightness = 0;

  buf[0] = COMMAND_WRITE_SINGLE_LED_BRIGHTNESS;
  buf[1] = led;
  buf[2] = brightness;

//  ESP_LOGVV(TAG, "Writing to 0x%.2X: %d %d", this->address_, led, brightness);

  ret = this->write(buf, 3);
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting brightness: %s", i2cErrorToString(ret));
  }
}


void QwiicLEDStick::set_brightness(uint8_t brightness) {
  uint8_t buf[2];
  i2c::ErrorCode ret = i2c::NO_ERROR;

  if (brightness > 31)
    brightness = 31;
  if (brightness < 0)
    brightness = 0;

  buf[0] = COMMAND_WRITE_ALL_LED_BRIGHTNESS;
  buf[1] = brightness;
  ret = this->write(buf, sizeof(buf));
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error setting brightness: %s", i2cErrorToString(ret));
  }
}


void QwiicLEDStick::all_off() {
  i2c::ErrorCode ret = i2c::NO_ERROR;
  uint8_t buf[1] = {COMMAND_WRITE_ALL_LED_OFF};

  ret = this->write(buf, 1);
  if (ret != i2c::NO_ERROR) {
    ESP_LOGE("qwiic_led_stick", "Error turning off all LEDs: %s", i2cErrorToString(ret));
  }
}

}
}