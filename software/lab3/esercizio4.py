import paho.mqtt.client as mqtt
import requests
import time
import json

resourceCatalogAddress = "http://localhost:8080"


def onConnect(client, userdata, flags, rc):
    print(f'Connected with result code {str(rc)}')


def onMessage(client, userdata, msg):
    data = json.loads(msg.payload)
    print("Received temperature data.")
    print(data)


class remoteController:
    def __init__(self, clientID):
        self.clientID = clientID
        self.client = mqtt.Client()
        self.client.on_connect = onConnect
        self.client.on_message = onMessage
        self.mqttHost = ""
        self.mqttPort = 1883
        # create an instance of paho.mqtt.client
        self._paho_mqtt = mqtt.Client(clientID, False)

        # register the callback
        self._paho_mqtt.on_connect = self.onConnect
        self._paho_mqtt.on_message = self.onMessageReceived

        self.topic = '/tiot/8/catalog/subscription/devices/subscription'
        self.messageBroker = 'mqtt.eclipseprojects.io'

        self.motion = 0
        self.sound = 0
        self.people = 0

        self.minTempL = 15
        self.maxTempL = 20
        self.minTempF = 25
        self.maxTempF = 35
        self.minTempLP = 16
        self.maxTempLP = 21
        self.minTempFP = 24
        self.maxTempFP = 29
        self.minTempLED = 15
        self.maxTempLED = 20
        self.minTempFan = 25
        self.maxTempFan = 35

    def myPublish(self, topic, message):
        # publish a message with a certain topic
        self._paho_mqtt.publish(topic, json.dumps(message), 2)

    def onMessageReceived(self, paho_mqtt, userdata, msg):
        # A new message is received
        print("Topic:'" + msg.topic + "', QoS: '" + str(msg.qos) + "' Message: '" + str(msg.payload) + "'")
        payload = json.loads(msg.payload)
        if msg.topic is "tiot/group8/temperature":
            speed = 0
            brightness = 0
            temperature = payload["e"][0]["v"]
            if temperature < self.maxTempLED:
                if temperature <= self.minTempLED:
                    brightness = 255
                else:
                    brightness = 255 * (self.maxTempLED - temperature) / (self.maxTempLED - self.minTempLED)

            message = {"bn": "Cloud", "e": [{"n": "brightness", "u": None, "t": time.time(), "v": brightness}]}
            self.myPublish("tiot/group8/led", message)
            if temperature >= self.minTempFan:
                if temperature > self.maxTempFan:
                    speed = 255
                else:
                    speed = 255 * (temperature - self.minTempFan) / (self.maxTempFan - self.minTempFan)
            message = {"bn": "Cloud", "e": [{"n": "speed", "u": None, "t": time.time(), "v": speed}]}
            self.myPublish("tiot/group8/fan", message)
        if msg.topic is "tiot/group8/sound":
            self.sound = payload["e"][0]["v"]
            self.people = self.sound or self.motion
            self.changeThreshold()
        if msg.topic is "tiot/group8/motion":
            self.motion = payload["e"][0]["v"]
            self.people = self.motion or self.sound
            self.changeThreshold()

    def changeThreshold(self):
        if self.people == 0:
            self.minTempLED = self.minTempL
            self.maxTempLED = self.maxTempL
            self.minTempFan = self.minTempF
            self.maxTempFan = self.maxTempF
        else:
            self.minTempLED = self.minTempLP
            self.maxTempLED = self.maxTempLP
            self.minTempFan = self.minTempFP
            self.maxTempFan = self.maxTempFP

    @staticmethod
    def getSubscriptions():
        response = requests.get(f'{resourceCatalogAddress}/')

        if response.status_code != 200:
            print("Unable to retrieve available subscriptions from catalog!")
            print(response.status_code)
            return

        print("Fetched available subscriptions!")
        jsonData = response.json()

        return jsonData["subscriptions"]

    def registerAsService(self, subscriptions):
        serviceAddress = subscriptions["REST"]["service"]
        serviceData = {
            "serviceID": self.serviceID,
            "description": "Smart home controller",
            "endPoints": None
        }
        response = requests.post(serviceAddress, json.dumps(serviceData))

        if response.status_code != 200:
            print("Error: Unable to register as service to the catalog!")
            print(response.status_code)
            return

        print("Successfully registered as service!")

        self.mqttHost = subscriptions["MQTT"]["device"]["hostname"]
        self.mqttPort = subscriptions["MQTT"]["device"]["port"]

        return

    def start(self, subscriptions):
        mqttAttributes = subscriptions["MQTT"]["device"]
        self.client.connect(mqttAttributes["hostname"], mqttAttributes["port"])
        self.client.loop_start()

        self.client.subscribe("tiot/group8/temperature")
        self.client.subscribe("tiot/group8/sound")
        self.client.subscribe("tiot/group8/motion")


if __name__ == "__main__":
    controller = remoteController("remoteControllerGroup8")
    subscriptions = controller.getSubscriptions()
    controller.registerAsService(subscriptions)
    controller.start(subscriptions)
