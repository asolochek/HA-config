from math import log,exp

# `data` is available as builtin and is a dictionary with the input data.
entity_id = data.get("entity_id")
initial_humidity = data.get("initial_humidity")
initial_temperature = data.get("initial_temperature")
new_temperature = data.get("new_temperature")

new_humidity = relative_humidity_at_different_temp(initial_temperature, initial_humidity, new_temperature)

if entity_id is not None:
    service_data = {"entity_id": entity_id, "value": new_humidity}
    hass.services.call("input_number", "set_value", service_data, False)

# `logger` and `time` are available as builtin without the need of explicit import.
logger.info("Humidity correction called at {} to {}".format(initial_humidity, new_humidity))


def celsius_to_kelvin(temperature_celsius):
    temperature_kelvin = temperature_celsius + 273.15
    return temperature_kelvin

def celsius_to_rankine(temperature_celsius):
    temperature_kelvin = celsius_to_kelvin(temperature_celsius)
    temperature_rankine = temperature_kelvin * (9 / 5)
    return temperature_rankine

def psia_to_mmHg(pressure_psia):
    pressure_mmHg = pressure_psia * 51.7149
    return pressure_mmHg

def psia_to_mPa(pressure_psia):
    pressure_mPa = pressure_psia * 68.9476
    return pressure_mPa

def psia_to_pa(pressure_psia):
    pressure_pa = pressure_psia * 6894.76
    return pressure_pa

def water_vapor_saturation_pressure(temperature_celsius):
    A = -1.0440397e4
    B = -1.1294650e1
    C = -2.7022355e-2
    D = 1.2890360e-5
    E = -2.478068e-9
    F = 6.5459673

    temp = celsius_to_rankine(temperature_celsius)

    # Calculate the saturation pressure in psia
    saturation_pressure_psia = exp(A / temp + B + C * temp + D * pow(temp, 2) + E * pow(temp, 3) + F * log(temp))

    return psia_to_pa(saturation_pressure_psia)


def relative_humidity_at_different_temp(initial_temp, initial_rel_humidity, target_temp):
    temp_kelvin = celsius_to_kelvin(initial_temp)

    # Convert initial relative humidity to absolute humidity at the initial temperature
    initial_abs_humidity = water_vapor_saturation_pressure(initial_temp) * initial_rel_humidity * 10 / (461.5 * temp_kelvin)
    corrected_humidity = initial_rel_humidity * water_vapor_saturation_pressure(initial_temp) * celsius_to_kelvin(target_temp) / (water_vapor_saturation_pressure(target_temp) * celsius_to_kelvin(initial_temp))

#    print("initial RH: ", initial_rel_humidity)
#    print("initial temp: ", initial_temp)
#    print("absolute humidity: ", initial_abs_humidity)

    return corrected_humidity
