import cherrypy
import json
import time
import paho.mqtt.client as PahoMQTT
import uuid


# TO DOO
# Implement an additional feature of the Catalog to remove all the devices and services with “insert-timestamp” 
# higher than two minutes. The Catalog has to take this action periodically (for example every 1 minute).

# INSERT ERROR MESSAGES

class Catalog(object):
    exposed = True
    
    def __init__(self):
        self.log = {"devices" : {}, "users" : {}, "services" : {}} 
        
        self.clientID = "team 8"
		# create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(self.clientID, False) 
		# register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_message = self.myOnMessageReceived

        self.topic = '/tiot/8/catalog/subscription/devices/subscription'
        self.messageBroker = 'mqtt.eclipseprojects.io'
        
    def start (self):
        #manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 1883)
        self._paho_mqtt.loop_start()
		# subscribe for a topic
        self._paho_mqtt.subscribe(self.topic, 2)

    def myPublish(self, topic, message):
		# publish a message with a certain topic
        self._paho_mqtt.publish(topic, message, 2)

    def myOnConnect (self, paho_mqtt, userdata, flags, rc):
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def myOnMessageReceived (self, paho_mqtt , userdata, msg):
		# A new message is received
        data = json.loads(msg.payload)
        topic = f"{self.topic}/{data['deviceID']}"
        device = {
            "deviceID": data['deviceID'],
            "endPoints": data["endPoints"],
            "availableResources": data["availableResources"],
            "timestamp": time.time()
            }
        self.log["devices"][data['deviceID']] = device

        self._paho_mqtt.publish(topic, json.dumps(device), 2)

    
    def cleanup(self):
        while True:
            current_time = time.time()

            # iterate over devices and remove old entries
            self.log["devices"] = {deviceID: device for deviceID, device in self.log["devices"].items() if current_time - device["timestamp"] <= 120}

            # iterate over services and remove old entries
            self.log["services"] = {serviceID: service for serviceID, service in self.log["services"].items() if current_time - service["timestamp"] <= 120}

            #time.sleep(60)

    
    def GET(self, *path, **query):
        
        if ((path[0] == "devices" or path[0] == "users" or path[0] == "services") and len(path) == 1):
            print("ciao")
            return json.dumps(self.log[path[0]])
        elif (path[0] == "device" and len(path) == 2):
            return json.dumps(self.log["devices"][path[1]])
        elif (path[0] == "user" and len(path) == 2):
            return json.dumps(self.log["users"][path[1]])
        elif (path[0] == "service" and len(path) == 2):
            return json.dumps(self.log["services"][path[1]])

            
        return ("Params: %s %s; params length %s" % (str(path), str (query), len(query)))
    
    
    def POST(self,  *path):
        
        #Reads raw data sent as Json POST method
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if (path[0] == "device" and len(path) == 1):
            #print(data["endPoints"])
            device = {
            "deviceID": data['deviceID'],
            "endPoints": data["endPoints"],
            "availableResources": data["sensors"],
            "timestamp": time.time()
            }
            self.log["devices"][data['deviceID']] = device
            return json.dumps(device)
        elif (path[0] == "user" and len(path) == 1):
            user = {
            "userID": data['userID'],
            "name": data["name"],
            "surname": data["surname"],
            "email": data["email"]
            }
            self.log["users"][data['userID']] = user
            return json.dumps(user)
        elif (path[0] == "service" and len(path) == 1):
            service = {
            "serviceID": data['serviceID'],
            "description": data["description"],
            "endPoints": data["endPoints"],
            "timestamp": time.time()
            }
            self.log["services"][data['serviceID']] = service
            return json.dumps(service)
        
        return "ERROR" #(f"Path len {len(path)} {path[0]} ")
    
    
    def PUT(self, *path):
        # Reads raw data sent as Json PUT method
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if (path[0] == "device" and len(path) == 2):
            deviceID = path[1]
            if deviceID in self.log["devices"]:
                self.log["devices"][deviceID]["endPoints"] = data["endPoints"]
                self.log["devices"][deviceID]["availableResources"] = data["sensors"]
                self.log["devices"][deviceID]["timestamp"] = time.time()  # Update the timestamp
                return json.dumps(self.log["devices"][deviceID])
            else:
                return "Device not found"
        else:
            return "Invalid path"


if __name__ == '__main__':

    test = Catalog()
    test.start()
    
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.sessions.on': True,
        }
    }

    cherrypy.tree.mount(test, '/', conf)

    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    cherrypy.config.update({'server.socket_port': 8080})

    cherrypy.engine.start()
    cherrypy.engine.block()

    