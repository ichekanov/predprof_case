import random
import time
import paho.mqtt.client as mqtt_client
import random as r


broker = 'srv1.clusterfly.ru'
port = 9124
debug = "user_e5d5320c/debug"
data = "user_e5d5320c/data"
# generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 1000)}'
username = 'user_e5d5320c'
password = 'pass_92aa380f'


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client, topic, msg):
    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")


client = connect_mqtt()
client.loop_start()
while True:
    # msg = time.strftime("%d %b %Y %H:%M:%S", time.localtime())
    msg = str(r.randint(185, 250)/10) + "," + str(r.randint(200, 800)/10) + "," + str(r.randint(3500, 8000)/10) + \
        "," + str(r.randint(0, 1)) + "," + str(r.randint(0, 1000) /
                                               10) + "," + str(r.randint(130, 1000)/10)
    publish(client, debug, msg)
    time.sleep(5)
