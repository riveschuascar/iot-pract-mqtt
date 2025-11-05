from typing import Any, Optional
import asyncio
from mcp.server.fastmcp import FastMCP
from asyncio_mqtt import Client, MqttError

mcp = FastMCP("nlp-mqtt-controller")

# MQTT Config
MQTT_BROKER = "c877f40f18e34e858972f14b8a14d0aa.s1.eu.hivemq.cloud"
MQTT_PORT = 8883

SENSOR_TOPIC = "sensors/ultrasonic/distance"
ACTUATOR_TOPIC = "actuators/servo/state"

mqtt_client: Client | None = None
last_distance: Optional[str] = None


async def ensure_mqtt_connected() -> Client:
    global mqtt_client
    if mqtt_client is None:
        mqtt_client = Client(MQTT_BROKER, port=MQTT_PORT, tls=True)
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
    if "distancia" in text or "medida" in text or "detectada" in text:
        return "READ_DISTANCE"

    # Servo rápido
    if ("servo" in text) and ("rápido" in text or "rapido" in text):
        return ACTUATOR_TOPIC, "2"

    # Servo lento
    if ("servo" in text) and ("lento" in text):
        return ACTUATOR_TOPIC, "1"

    # Parar servo
    if ("servo" in text) and ("detener" in text or "parar" in text or "alto" in text):
        return ACTUATOR_TOPIC, "0"

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
    # Ejecutar lector del sensor en paralelo
    loop.create_task(sensor_listener())
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
