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
     
     
 