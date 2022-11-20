/*
 INFORMACIÓN ADICIONAL
¿Cómo funciona el Higrómetro FC-28?
El FC-28 cuenta con una placa de medición estándar que permite obtener la medición 
como valor analógico o como una salida digital, activada cuando la humedad supera un cierto umbral.

Los valores del umbral van desde 0 sumergido en agua, a 1023 en el aire (o en un suelo muy seco). 
Un suelo ligeramente húmero daría valores típicos de 600-700. Un suelo seco tendrá valores de 800-1023.

La salida digital dispara cuando el valor de humedad supera un cierto umbral, que ajustamos mediante 
el potenciómetro. Por tanto, obtendremos una señal LOW cuando el suelo no está húmedo, 
y HIGH cuando la humedad supera el valor de consigna.


AIRE: 1024
TIERRA SECA: 919
AGUA: 327
*/

const int fc_sensor = A0;
 
void setup() {
  // put your setup code here, to run once:
  pinMode(fc_sensor, INPUT);
  Serial.begin(115200);
}

int lectura;
void loop() {
  delay(1000);
  // put your main code here, to run repeatedly:
  lectura = analogRead(fc_sensor);

  Serial.print("Valor: ");
  Serial.println(lectura);
}
