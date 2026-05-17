import sys
import time
import serial
from Adafruit_IO import MQTTClient # importar librerias 

ADAFRUIT_AIO_USERNAME = ""
ADAFRUIT_AIO_KEY      = ""

SERIAL_PORT = "COM3"
BAUDRATE = 9600

S1_TX = "servo-1"
S2_TX = "servo-2"
S3_TX = "servo-3"
S4_TX = "servo-4"

S1_RX = "servo1-rx"
S2_RX = "servo2-rx"
S3_RX = "servo3-rx"
S4_RX = "servo4-rx" #FEEDS DE ADAFRUIT

valores = {
    S1_TX: 90, 
    S2_TX: 90,
    S3_TX: 90,
    S4_TX: 90 #diccionario para guardar datos servos
}

arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
time.sleep(2)# abrir comunicacion serial (conectar python con el micro)

def limitar(valor): #revisar y limitar angulo recibido desde adafruit
    try:
        angulo = int(float(valor)) #valor valido entero
    except ValueError:
        return None

    if angulo < 0:
        angulo = 0
    if angulo > 180:
        angulo = 180

    return angulo

def leer_micro():
    while arduino.in_waiting > 0:# revisar si hay datos
        linea = arduino.readline().decode("utf-8", errors="ignore").strip() 
        #leer linea del micro hasta encontrar \n convierte bytes recibidos a texto 
        if linea:
            print("Micro:", linea)

def enviar_micro(comando): #enviar el comando entendible al micro
    print("Enviando al micro:", comando)
    arduino.write((comando + "\n").encode("utf-8"))
    time.sleep(0.25)
    leer_micro()

def publicar_rx(): #publicar en los feeds rx el valor actualizado
    client.publish(S1_RX, valores[S1_TX])
    client.publish(S2_RX, valores[S2_TX])
    client.publish(S3_RX, valores[S3_TX])
    client.publish(S4_RX, valores[S4_TX])

def mandar_servos(): #armar el comando que entiende el micro para los servos
    comando = "M,{},{},{},{}".format(
        valores[S1_TX],
        valores[S2_TX],
        valores[S3_TX],
        valores[S4_TX]
    )

    enviar_micro(comando)
    publicar_rx()

def connected(client):# funcion automatica cuando python se conecta con adafruit
    print("Conectado a Adafruit IO")

    client.subscribe(S1_TX)
    client.subscribe(S2_TX)
    client.subscribe(S3_TX)
    client.subscribe(S4_TX)

    print("Suscrito a feeds de servos")

    enviar_micro("U")
    publicar_rx()

def disconnected(client):
    print("Desconectado de Adafruit IO")
    arduino.close()
    sys.exit(1)

def message(client, feed_id, payload):
    print("Feed recibido:", feed_id, "=", payload)

    angulo = limitar(payload)

    if angulo is None:
        print("Valor invalido recibido")
        return

    if feed_id in valores:
        valores[feed_id] = angulo
        mandar_servos()

client = MQTTClient(ADAFRUIT_AIO_USERNAME, ADAFRUIT_AIO_KEY) #crear cliente que se conecta a adafruit

client.on_connect = connected
client.on_disconnect = disconnected
client.on_message = message

client.connect()
client.loop_background()

print("Sistema Adafruit-Python-Microchip corriendo...")

while True:
    leer_micro()
    time.sleep(1)