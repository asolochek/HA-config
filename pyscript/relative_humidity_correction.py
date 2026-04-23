import math


@service(supports_response="only")
def humidity_correction(dest_id=None, initial_humidity=None, initial_temperature=None, new_temperature=None, return_response=True):
    """Get the temperature-corrected relative humidity value given a relative humidity computed at the wrong temperature"""

    humidity1_state = state.get(initial_humidity)
    temperature1_state = state.get(initial_temperature)
    temperature2_state = state.get(new_temperature)

    try:
        humidity1 = float(humidity1_state)

        if "F" in temperature1_state.unit_of_measurement:
            temperature1 = fahrenheit_to_celsius(float(temperature1_state))
        else:
            temperature1 = float(temperature1_state)

        if "F" in temperature2_state.unit_of_measurement:
            temperature2 = fahrenheit_to_celsius(float(temperature2_state))
        else:
            temperature2 = float(temperature2_state)

        humidity2 = relative_humidity_at_different_temp(temperature1, humidity1, temperature2)
        humidity2 = round(humidity2, 2)

        if dest_id is not None:
            input_number.set_value(entity_id=dest_id, value=humidity2)

    except ValueError:
        return False

    return humidity2

@service(supports_response="only")
def dew_point(dest_id=None, temperature=None, relative_humidity=None, return_response=True):
    """Get the dew point given a temperature and a relative humidity"""

    temperature_state = state.get(temperature)
    humidity_state = state.get(relative_humidity)

    try:
        if "F" in temperature_state.unit_of_measurement:
            temperature = fahrenheit_to_celsius(float(temperature_state))
        else:
            temperature = float(temperature_state)

        humidity = float(humidity_state)

        dew_point_value = rh_to_dew_point(temperature, humidity)
        dew_point_value = round(dew_point_value, 2)

        if dest_id is not None:
            input_number.set_value(entity_id=dest_id, value=dew_point_value)

    except ValueError:
        return { False }

    return { "dew_point": dew_point_value }

def fahrenheit_to_celsius(temperature_fahrenheit):
    temperature_celsius = (temperature_fahrenheit - 32) / 1.8
    return temperature_celsius

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

    # Using equation2 from https://www.engineersedge.com/calculators/water_vapor_saturation_pressure_15730.htm
    A = -1.0440397e4
    B = -1.1294650e1
    C = -2.7022355e-2
    D = 1.2890360e-5
    E = -2.478068e-9
    F = 6.5459673

    temp = celsius_to_rankine(temperature_celsius)

    # Calculate the saturation pressure in psia
    saturation_pressure_psia = math.exp(A / temp + B + C * temp + D * pow(temp, 2) + E * pow(temp, 3) + F * math.log(temp))

    return psia_to_pa(saturation_pressure_psia)


def relative_humidity_at_different_temp(initial_temp, initial_rel_humidity, target_temp):
    temp1_kelvin = celsius_to_kelvin(initial_temp)
    temp2_kelvin = celsius_to_kelvin(target_temp)

    # Convert initial relative humidity to absolute humidity at the initial temperature
#    initial_abs_humidity = water_vapor_saturation_pressure(initial_temp) * initial_rel_humidity * 10 / (461.5 * temp1_kelvin)
    corrected_humidity = initial_rel_humidity * water_vapor_saturation_pressure(initial_temp) * temp2_kelvin / (water_vapor_saturation_pressure(target_temp) * temp1_kelvin)

    print("Corrected ", initial_rel_humidity, "% @ ", initial_temp, "ºC to ", corrected_humidity, "% @ ", target_temp, "ºC")
    return corrected_humidity


def dew_point_to_rh(temperature, dp):
  return 100 * (math.exp((17.625 * dp) / (243.04 + dp)) / math.exp((17.625 * temperature) / (243.04 + temperature)))


def rh_to_dew_point(temperature, rh):
  pressure = math.log(rh / 100) + ((17.625 * temperature) / (243.04 + temperature))
  return (237.3 * pressure) / (17.269 - pressure)
