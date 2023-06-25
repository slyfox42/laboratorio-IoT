import cherrypy
import json
import time


class Catalog(object):
    exposed = True
    
    def __init__(self):
        self.log = {
            "devices": {},
            "users": {},
            "services": {}
        }
    
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
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if path[0] == "device" and len(path) == 1:
            print(data["endPoints"])
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
        
        return cherrypy.HTTPError(404, "Not found.")

    def PUT(self, *path):
        # Reads raw data sent as Json PUT method
        raw_body = cherrypy.request.body.read().decode()
        data = json.loads(raw_body)
        
        if path[0] == "device" and len(path) == 2:
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

    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.sessions.on': True,
        }
    }

    cherrypy.tree.mount(Catalog(), '/', conf)

    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    cherrypy.config.update({'server.socket_port': 8080})

    cherrypy.engine.start()
    cherrypy.engine.block()
