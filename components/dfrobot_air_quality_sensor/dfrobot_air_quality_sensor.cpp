#include "dfrobot_air_quality_sensor.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace dfrobot_air_quality_sensor {

static const char *const TAG = "dfrobot_air_quality";

void DFRobotAirQualitySensor::setup() {
  ESP_LOGI(TAG, "Starting DFRobot Air Quality Sensor...");

  // IMPORTANT:
  // Do NOT call Arduino Wire.begin() here (directly or indirectly).
  // ESPHome owns the I2C bus. Calling Wire.begin() in ESPHome 2025.12+
  // on ESP32-S3 can acquire the bus twice and break the whole I2C stack.

  uint8_t version = 0;
  if (!this->read_byte(PARTICLENUM_GAIN_VERSION, &version)) {
    ESP_LOGE(TAG, "Error initializing sensor (I2C probe failed at 0x%02X)", this->address_);
    mark_failed();
    return;
  }

  ESP_LOGI(TAG, "Sensor initialized successfully (version: %u).", version);
  
}

void DFRobotAirQualitySensor::update() {
  auto read_u16 = [&](uint8_t reg) -> uint16_t {
    uint8_t buf[2]{0, 0};
    if (!this->read_bytes(reg, buf, 2)) {
      return 0xFFFF;
    }
    return (uint16_t(buf[0]) << 8) | uint16_t(buf[1]);
  };

  const uint16_t pm1 = read_u16(PARTICLE_PM1_0_STANDARD);
  const uint16_t pm25 = read_u16(PARTICLE_PM2_5_STANDARD);
  const uint16_t pm10 = read_u16(PARTICLE_PM10_STANDARD);

  const uint16_t pm03 = read_u16(PARTICLENUM_0_3_UM_EVERY0_1L_AIR);
  const uint16_t pm05 = read_u16(PARTICLENUM_0_5_UM_EVERY0_1L_AIR);
  const uint16_t pm50 = read_u16(PARTICLENUM_5_0_UM_EVERY0_1L_AIR);
  const uint16_t pm105 = read_u16(PARTICLENUM_10_UM_EVERY0_1L_AIR);
  
  int16_t aqi = -1;
  if (pm25 != 0xFFFF && pm10 != 0xFFFF) {
    AbstractAQICalculator *calculator = aqi_calculator_factory_.get_calculator(aqi_calc_type_);
    aqi = calculator->get_aqi(pm25, pm10);
  }

  ESP_LOGD(TAG, "PM1.0: %u µg/m³, PM2.5: %u µg/m³, PM10: %u µg/m³", pm1, pm25, pm10);

  if (pm_1_0_sensor_ != nullptr)
    pm_1_0_sensor_->publish_state(pm1 == 0xFFFF ? NAN : pm1);
    
  if (pm_2_5_sensor_ != nullptr)
    pm_2_5_sensor_->publish_state(pm25 == 0xFFFF ? NAN : pm25);
    
  if (pm_10_sensor_ != nullptr)
    pm_10_sensor_->publish_state(pm10 == 0xFFFF ? NAN : pm10);
    
  if (aqi_sensor_ != nullptr)
    aqi_sensor_->publish_state(aqi);    
    
  if (pm_0_3_sensor_ != nullptr)
    pm_0_3_sensor_->publish_state(pm03 == 0xFFFF ? NAN : pm03);
    
  if (pm_0_5_sensor_ != nullptr)
    pm_0_5_sensor_->publish_state(pm05 == 0xFFFF ? NAN : pm05);
    
  if (pm_5_0_sensor_ != nullptr)
    pm_5_0_sensor_->publish_state(pm50 == 0xFFFF ? NAN : pm50);
    
  if (pm_10_5_sensor_ != nullptr)
    pm_10_5_sensor_->publish_state(pm105 == 0xFFFF ? NAN : pm105);

}

}  // namespace dfrobot_air_quality_sensor
}  // namespace esphome
