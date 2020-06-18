void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(REDLIGHTPIN, red_light_value);
  analogWrite(GREENLIGHTPIN, green_light_value);
  analogWrite(BLUELIGHTPIN, blue_light_value);
}
