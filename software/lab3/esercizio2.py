import paho.mqtt.client as mqtt
import requests
import json

resourceCatalogAddress = "http://localhost:8080"
arduinoID = "arduinoGroup8"


def onConnect(client, userdata, flags, rc):
    print(f'Connected with result code {str(rc)}')


def onMessage(client, userdata, msg):
    data = json.loads(msg.payload)
    print("Received temperature data.")
    print(data)


class MQTTSubscriber:
    def __init__(self, serviceID):
        self.serviceID = serviceID
        self.client = mqtt.Client()
        self.client.on_connect = onConnect
        self.client.on_message = onMessage

    @staticmethod
    def getSubscriptions():
        response = requests.get(f'{resourceCatalogAddress}/')

        if response.status_code != 200:
            response.raise_for_status()

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
            response.raise_for_status()

        print("Successfully registered as service!")

        return

    @staticmethod
    def getEndpoints():
        response = requests.get(f'{resourceCatalogAddress}/devices')

        if response.status_code != 200:
            response.raise_for_status()

        jsonData = response.json()
        if arduinoID not in jsonData.keys():
            raise Exception("Error: Arduino device not registered to catalog!!")

        response = requests.get(f'{resourceCatalogAddress}/devices/{arduinoID}')

        if response.status_code != 200:
            response.raise_for_status()

        jsonData = response.json()

        return jsonData["endPoints"]

    def start(self, subscriptions, endpoints):
        mqttAttributes = subscriptions["MQTT"]["device"]
        self.client.connect(mqttAttributes["hostname"], mqttAttributes["port"])
        self.client.loop_start()

        self.client.subscribe(endpoints["MQTT"]["temperature"])


if __name__ == "main":
    subscriber = MQTTSubscriber("arduinoTempSub")
    subscriptions = subscriber.getSubscriptions()
    endpoints = subscriber.getEndpoints()
    subscriber.registerAsService(subscriptions)
    subscriber.start(subscriptions, endpoints)
