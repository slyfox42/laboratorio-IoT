import paho.mqtt.client as mqtt
import requests


class MQTTSubscriber:

    @staticmethod
    def getSubscriptions():
        response = requests.get("http://localhost:8080/devices")

        if response.status_code != 200:
            raise Exception("Unable to retrieve available subscriptions from catalog!")

        print("Fetched available subscriptions!")

        return response.json()

    @staticmethod
    def registerAsService():
        serviceData = {
          "serviceID": "arduinoTempSub",
          "description": "Arduino temperature mqtt subscriber",
          "endPoints": None
        }
        response = requests.post("http://localhost:8080/service", serviceData)

        if response.status_code != 200:
            raise Exception("Unable to register as service to the catalog!")

        print("Successfully registered as service!")

        return

    def getTopics(self, deviceID):
        response = requests.get("http://localhost ")
    @staticmethod
    def getAllTopics(self):
       devices = self.getSubscriptions()
       topics =

        if response.status_code != 200:
            raise Exception("Unable to get Arduino topics from catalog!")

        return response.json().endPoints


# # The callback for when the client receives a CONNACK response from the server.
# def on_connect(client, userdata, flags, rc):
#     print("Connected with result code "+str(rc))
#
#     # Subscribing in on_connect() means that if we lose the connection and
#     # reconnect then subscriptions will be renewed.
#     client.subscribe("$SYS/#")
#
#
# # The callback for when a PUBLISH message is received from the server.
# def on_message(client, userdata, msg):
#     print(msg.topic+" "+str(msg.payload))
#
#
# client = mqtt.Client()
# client.on_connect = on_connect
# client.on_message = on_message
#
# client.connect("test.mosquitto.org", 1883, 60)
#
# # Blocking call that processes network traffic, dispatches callbacks and
# # handles reconnecting.
# # Other loop*() functions are available that give a threaded interface and a
# # manual interface.
# client.loop_forever()
