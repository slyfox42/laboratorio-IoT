import paho.mqtt.client as PahoMQTT
import time
import uuid
import json
import random

class MySubscriber:
		def __init__(self, clientID):
			self.clientID = clientID
			self.id = 1234
			# create an instance of paho.mqtt.client
			self._paho_mqtt = PahoMQTT.Client(clientID, False) 

			# register the callback
			self._paho_mqtt.on_connect = self.myOnConnect
			self._paho_mqtt.on_message = self.myOnMessageReceived

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


		def start (self):
			#manage connection to broker
			self._paho_mqtt.connect(self.messageBroker, 1883)
			self._paho_mqtt.loop_start()
			self._paho_mqtt.subscribe(f"{self.topic}{self.id}",2)


		def stop (self):
			self._paho_mqtt.unsubscribe(self.topic)
			self._paho_mqtt.loop_stop()
			self._paho_mqtt.disconnect()

		def myOnConnect (self, paho_mqtt, userdata, flags, rc):
			print ("Connected to %s with result code: %d" % (self.messageBroker, rc))
   
		def myPublish(self, topic, message):
			# publish a message with a certain topic
			self._paho_mqtt.publish(topic, json.dumps(message), 2)

		def myOnMessageReceived (self, paho_mqtt , userdata, msg):
			# A new message is received
			print ("Topic:'" + msg.topic+"', QoS: '"+str(msg.qos)+"' Message: '"+str(msg.payload) + "'")
			payload = json.loads(msg.payload)
			if msg.topic is "/tiot/group8/temperature":
				speed = 0
				brightness = 0
				temperature = payload["e"][0]["v"]
				if temperature < self.maxTempLED:
					if (temperature <= self.minTempLED):
						brightness = 255
					else:
						brightness = 255 * (self.maxTempLED - temperature) / (self.maxTempLED - self.minTempLED)
				message = {"bn": "Cloud", "e": [{"n": "brightness", "u": None, "t": time.time(), "v": brightness}]}
				self.myPublish("/tiot/group8/led", message)
				if temperature >= self.minTempFan:
					if temperature > self.maxTempFan:
						speed = 255
					else:
						speed = 255 * (temperature - self.minTempFan) / (self.maxTempFan - self.minTempFan)
				message = {"bn": "Cloud", "e": [{"n": "speed", "u": None, "t": time.time(), "v": speed}]}
				self.myPublish("/tiot/group8/fan", message)
			if msg.topic is "/tiot/group8/sound":
				self.sound = payload["e"][0]["v"]
				self.people = self.sound or self.motion
				self.changeThreshold()
			if msg.topic is "/tiot/group8/motion":
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

		def runner(self):
			deviceIDs = []
			while True:
				# Randomly decide whether to register a new device or update an existing one
				if len(deviceIDs) == 0 or random.random() < 0.5:
					# Register a new device
					message = {
						"deviceID": self.id,
						"endPoints": "/tiot/8/catalog/subscription/devices/subscription",
						"availableResources": ["Temperature", "Humidity", "Motion sensor"],
					}
					deviceIDs.append(message["deviceID"])
					self._paho_mqtt.subscribe(f"{self.topic}{message['deviceID']}", 2)
					self.myPublish(self.topic, message)

				else:
					# Update an existing device
					deviceID = random.choice(deviceIDs)
					message = {
						"deviceID": deviceID,
						"endPoints": "/tiot/8/catalog/subscription/devices/subscription",
						"availableResources": ["Temperature", "Humidity", "Motion sensor"],
					}
					deviceIDs.append(message["deviceID"])
					self.myPublish(self.topic, json.dumps(message))


				# Sleep for a minute
				for _ in range(61):
					time.sleep(1)



if __name__ == "__main__":
	iotDevice = MySubscriber("MySubscriber 1")
	iotDevice.start()

	message = {
		"deviceID": "1234",
        "endPoints": "/tiot/8/catalog/subscription/devices/subscription/1234",
        "availableResources": "temperature",
	}

	iotDevice.runner()


