import paho.mqtt.client as mqtt
import sched, time
import requests
import json

resourceCatalogAddress = "http://localhost:8080"


def onConnect(client, userdata, flags, rc):
    print(f'Connected with result code {str(rc)}')


def onMessage(client, userdata, msg):
    data = json.loads(msg.payload)
    print("Received temperature data.")
    print(data)


class MQTTPublisher:
    def __init__(self, serviceID):
        self.serviceID = serviceID
        self.client = mqtt.Client()
        self.client.on_connect = onConnect
        self.client.on_message = onMessage
        self.scheduler = sched.scheduler(time.time, time.sleep)
        self.mqttHost = ""
        self.mqttPort = 1883
        self.publishTopic = ""
        self.previousLed = 0

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
          "description": "Arduino temperature mqtt subscriber",
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

    def getEndpoints(self):
        response = requests.get(f'{resourceCatalogAddress}/devices')

        if response.status_code != 200:
            print("Error: Unable to get devices list!")
            print(response.status_code)
            return

        jsonData = response.json()
        deviceList = list(jsonData)
        deviceID = jsonData[deviceList[0]]["deviceID"]

        response = requests.get(f'{resourceCatalogAddress}/devices/{deviceID}')

        if response.status_code != 200:
            print("Error: Unable to get device data!")
            print(response.status_code)
            return

        jsonData = response.json()

        self.publishTopic = jsonData["MQTT"]["Led"]

    def loopPublish(self, scheduler):
        newValue = 1 if self.previousLed == 0 else 0
        self.scheduler.enter(60, 1, self.loopPublish, argument=[scheduler])
        self.client.single(self.publishTopic, newValue, 0, False, self.mqttHost, self.mqttPort)
        self.previousLed = newValue

    def start(self):
        self.client.connect(self.mqttHost, self.mqttPort)
        self.client.loop_start()

        self.scheduler.enter(60, 1, self.loopPublish, argument=[self.scheduler])



if __name__ == "main":
    subscriber = MQTTPublisher("arduinoLedPub")
    subscriptions = subscriber.getSubscriptions()
    subscriber.getEndpoints()
    subscriber.registerAsService(subscriptions)
    subscriber.start()

