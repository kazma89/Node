#include <ESP8266WiFi.h>
#include <WiFiClient.h>

int contconexion = 0;

//Datos de conexion al router
const char *ssid = "Cordoba";
const char *pass = "DianiJireh15";

//Contador de la duracion del tiempo de conexion
unsigned long previousMillis = 0;

char host[48];
String strhost = "192.168.0.100"; //IP o DNS del servidor
//URL de la página PHP que enviara los datos al MYSQL y
//al sitio web a visualizar
String strurl = "/vida/enviardatos.php";
String chipid = ""; //ID del dispositivo usa el ID del ESP8266


String enviarDatos(String datos) // Funcion para enviar los datos a la base de datos
{
  String linea = "error";
  WiFiClient client;
  strhost.toCharArray(host, 49); //Convierte lo recibido en un arreglo de 49 carateres
  if (!client.connect(host, 80))//Si no se conecta al host en el puerto 80
  {
    Serial.println("Fallo de conexion a la DB");
    return linea;
  }
  //Si se conecta
  client.print(String("POST ") + strurl + " HTTP/1.1" + "\r\n" + 
               "Host: " + strhost + "\r\n" +
               "Accept: */*" + "*\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
               "\r\n" + datos);//envia la informacion anterior al servidor por metodo POST
  delay(10);

  Serial.print("Enviando datos a SQL...");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if ((millis() - timeout) > 5000) //Tiempo que espera para la conexion
    {
      Serial.println("Cliente fuera de tiempo");
      client.stop();
      return linea;
    }
  }

  //Imprime la informacion recibida del server en el monitor serial
  while (client.available()) {
    linea = client.readStringUntil('\r');
  }
  Serial.println(linea);
  return linea;
}

void setup() {
  Serial.begin(9600);
  Serial.println("");

  Serial.print("chipID: ");
  chipid = String(ESP.getChipId());//Obtiene el ID del ESP8266
  Serial.println(chipid); 

  //Conexion a la red
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED and contconexion < 50) {
    ++contconexion;
    delay(500);
    Serial.print(".");
  }

  if (contconexion < 50) {
    //configuración del modulo usando ip fija (OJO SEPARADOS POR COMA NO PUNTOS)
    IPAddress ip(192, 168, 0, 200); //ip que tendra el modulo wifi
    IPAddress subnet(255, 255, 255, 0); //máscara de subred
    IPAddress gateway(192, 168, 0, 1); //ip del router para la salida a Internet
    WiFi.config(ip, gateway, subnet);

    Serial.println("");
    Serial.print("Conectado a la red con la IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Error de conexion");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 10000) //Envia la temperatura al servidor cada 10 segundos
  {
    previousMillis = currentMillis;
    int tmp = analogRead(17);//Pin del sensor de temperatura (A0 para el NodeMCU)
    float temp = tmp * 0.322265625;
    Serial.println(temp);
    enviarDatos("chipid=" + chipid + "&temperatura=" + String(temp, 2));
  }
}
