# exercise 5: Extend the functionalities of Catalog developed in Exercise_1 to work as MQTT subscriber
# either to register a new device or to refresh the old registration by updating its “insert-timestamp”.


import cherrypy
import json
import time
import paho.mqtt.client as PahoMQTT
import threading


class Catalog(object):
    exposed = True
    
    def __init__(self):
        self.log = {"devices" : {}, "users" : {}, "services" : {}} 
        
        self.clientID = "team 8"
        self._paho_mqtt = PahoMQTT.Client(self.clientID, False)
        self._paho_mqtt.on_connect = self.onConnect
        self._paho_mqtt.on_message = self.onMessageReceived

        self.topic = 'tiot/group8/catalog/subscription/devices/subscription'
        self.messageBroker = 'mqtt.eclipseprojects.io'
        
        # start the cleanup thread
        self.cleanup_thread = threading.Thread(target=self.cleanup)
        self.cleanup_thread.start()

    def start (self):
        #manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 1883)
        self._paho_mqtt.loop_start()
        # subscribe for a topic
        self._paho_mqtt.subscribe(self.topic, 2)

    def publish(self, topic, message):
        # publish a message with a certain topic
        self._paho_mqtt.publish(topic, message, 2)

    def onConnect (self, paho_mqtt, userdata, flags, rc):
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def onMessageReceived (self, paho_mqtt , userdata, msg):
        # update device data and refresh timestamp
        data = json.loads(msg.payload)
        if data["deviceID"] in self.log["devices"]:
            device = {
                "deviceID": data['deviceID'],
                "endPoints": data["endPoints"],
                "availableResources": data["availableResources"],
                "timestamp": time.time()
                }
            self.log["devices"][data['deviceID']] = device

    def cleanup(self):
        while True:
            current_time = time.time()

            # iterate over devices and remove old entries
            self.log["devices"] = {deviceID: device for deviceID, device in self.log["devices"].items() if current_time - device["timestamp"] <= 120}

            # iterate over services and remove old entries
            self.log["services"] = {serviceID: service for serviceID, service in self.log["services"].items() if current_time - service["timestamp"] <= 120}


    
    def GET(self, *path, **query):
        if  len(path) == 1 and path[0] in ["devices", "users", "services"]:
            return json.dumps(self.log[path[0]])

        elif len(path) == 2 and path[0] in ["device", "user", "service"]:
            try:
                return json.dumps(self.log[path[0]][path[1]])
            except:
                raise cherrypy.HTTPError(404, f'{path[0].capitalize()} Not Found')
        
        else:
            raise cherrypy.HTTPError(404, "Not found.")
            
    
    def POST(self,  *path):
        
        # Reads raw data sent as Json POST method
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if path[0] == "device" and len(path) == 1:
            device = {
                "deviceID": data['deviceID'],
                "endPoints": data["endPoints"],
                "availableResources": data["availableResources"],
                "timestamp": time.time()
            }
            self.log["devices"][data['deviceID']] = device
            return json.dumps(device)
        elif path[0] == "user" and len(path) == 1:
            user = {
                "userID": data['userID'],
                "name": data["name"],
                "surname": data["surname"],
                "email": data["email"]
            }
            self.log["users"][data['userID']] = user
            return json.dumps(user)
        elif path[0] == "service" and len(path) == 1:
            service = {
                "serviceID": data['serviceID'],
                "description": data["description"],
                "endPoints": data["endPoints"],
                "timestamp": time.time()
            }
            self.log["services"][data['serviceID']] = service
            return json.dumps(service)
        elif len(path) > 1:
            raise cherrypy.HTTPError(400, "Too many parameters should just be just, /device, /user or /service")
        elif len(path) == 1:
            raise cherrypy.HTTPError(404, "Not found, available paths are: /device, /user or /service")
        
    def PUT(self, *path):
        # Reads raw data sent as Json PUT method
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if path[0] == "device" and len(path) == 2:
            deviceID = path[1]
            if deviceID in self.log["devices"]:
                self.log["devices"][deviceID]["endPoints"] = data["endPoints"]
                self.log["devices"][deviceID]["availableResources"] = data["availableResources"]
                self.log["devices"][deviceID]["timestamp"] = time.time()  # Update the timestamp
                return json.dumps(self.log["devices"][deviceID])
            else:
                raise cherrypy.HTTPError(404, "Device not found")
        else:
            raise cherrypy.HTTPError(404, "Invalid path")


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

    