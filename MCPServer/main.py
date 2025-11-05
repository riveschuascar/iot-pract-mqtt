import dotenv
import ssl
import asyncio
from typing import Any, Optional
from mcp.server.fastmcp import FastMCP
from aiomqtt import Client, MqttError
import os

dotenv.load_dotenv()

mcp = FastMCP("mqtt-controller")

# MQTT Config desde .env
MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = int(os.getenv("MQTT_PORT", "8883"))
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")

SENSOR_TOPIC = os.getenv("SENSOR_TOPIC", "sensors/ultrasonic/distance")
ACTUATOR_TOPIC = os.getenv("ACTUATOR_TOPIC", "actuators/servo/state")

mqtt_client: Client | None = None
last_distance: Optional[str] = None

async def ensure_mqtt_connected() -> Client:
    global mqtt_client
    if mqtt_client is None:
        ssl_context = ssl.create_default_context()
        mqtt_client = Client(
            MQTT_BROKER,
            port=MQTT_PORT,
            username=MQTT_USERNAME,
            password=MQTT_PASSWORD,
            tls_context=ssl_context
        )
        await mqtt_client.connect()
    return mqtt_client

async def sensor_listener():
    """
    Mantiene actualizada la última distancia recibida.
    """
    global last_distance
    client = await ensure_mqtt_connected()
    async with client.unfiltered_messages() as messages:
        await client.subscribe(SENSOR_TOPIC)
        async for msg in messages:
            last_distance = msg.payload.decode().strip()

def interpret_command(command: str) -> tuple[str, str] | str:
    """
    Traduce lenguaje natural a acciones MQTT o solicitudes.
    """
    text = command.lower()

    # Consultar distancia
    if any(term in text for term in ["distancia", "medida", "detectada", "sensor"]):
        return "READ_DISTANCE"

    # Acciones relacionadas al servo
    if "servo" in text or "mover" in text or "girar" in text or "activar" in text:
        
        # Movimiento rápido
        if any(term in text for term in ["rápido", "rapido", "veloz", "máxima", "maxima", "rápidamente"]):
            return ACTUATOR_TOPIC, "2"
        
        # Movimiento lento
        if "lento" in text or "suave" in text:
            return ACTUATOR_TOPIC, "1"
        
        # Detener
        if any(term in text for term in ["detener", "parar", "alto", "stop", "quieto"]):
            return ACTUATOR_TOPIC, "0"

        # Si llega aquí, velocidad media por defecto
        return ACTUATOR_TOPIC, "1"

    return "UNKNOWN"

@mcp.tool()
async def process_command(message: str) -> str:
    """
    Procesa lenguaje natural y ejecuta acciones MQTT.
    """
    result = interpret_command(message)

    # Consulta de distancia
    if result == "READ_DISTANCE":
        if last_distance is None:
            return "Aún no hay lectura disponible del sensor."
        return f"Última distancia registrada: {last_distance}"

    if result == "UNKNOWN":
        return "No se pudo interpretar el comando."

    topic, payload = result

    try:
        client = await ensure_mqtt_connected()
        await client.publish(topic, payload)
        return f"Servo actualizado: {payload}"
    except MqttError as e:
        return f"Error enviando comando MQTT: {e}"

def main():
    loop = asyncio.get_event_loop()
    loop.create_task(sensor_listener())
    print(f"Conectando a MQTT Broker en {MQTT_BROKER}:{MQTT_PORT}:{SENSOR_TOPIC}:{ACTUATOR_TOPIC}")
    mcp.run(transport="stdio")

if __name__ == "__main__":
    main()
