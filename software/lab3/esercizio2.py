import paho.mqtt.client as mqtt
import requests
import json

resourceCatalogAddress = "http://localhost:8080"


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

        return

    @staticmethod
    def getEndpoints():
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

        return jsonData["endPoints"]

    def start(self, subscriptions, endpoints):
        mqttAttributes = subscriptions["MQTT"]["device"]
        self.client.connect(mqttAttributes["hostname"], mqttAttributes["port"])
        self.client.loop_start()
        for endpoint in endpoints:
            self.client.subscribe(f'{mqttAttributes["topic"]}/{endpoint}', 2)


if __name__ == "main":
    subscriber = MQTTSubscriber("arduinoTempSub")
    subscriptions = subscriber.getSubscriptions()
    endpoints = subscriber.getEndpoints()
    subscriber.registerAsService(subscriptions)
    subscriber.start(subscriptions, endpoints)
