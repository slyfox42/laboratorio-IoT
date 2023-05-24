import cherrypy
import json
import time


class ConvertService(object):
    exposed = True

    def POST(self, *path):
        if len(path) > 0 and path[0] == "convert":
            body = json.loads(cherrypy.request.body.read())
            original, target, values = ConvertService.checkQuery(body)
            output = [self.convert(original, target, value) for value in values]
            timestamp = int(time.time())
            return json.dumps({
                "values": body["values"],
                "targetValues": output,
                "originalUnit": body["originalUnit"],
                "finalUnit": body["targetUnit"],
                "timestamp": timestamp
            })
        else:
            return "Head over to /convert to convert temperature."

    @staticmethod
    def checkQuery(body):
        keys = list(body.keys())
        if ("values" not in keys
                or "originalUnit" not in keys
                or "targetUnit" not in keys):
            raise cherrypy.HTTPError(400, "Missing required parameters: originalUnit, values, targetUnit.")

        units = ["k", "c", "f"]
        original = body["originalUnit"].lower()
        target = body["targetUnit"].lower()
        values = body["values"]

        if (original not in units
                or target not in units
                or original == target):
            raise cherrypy.HTTPError(
                400, "Must insert valid units of measurement.")

        for x in values:
            if not str(x).isnumeric():
                raise cherrypy.HTTPError(
                    400, "Value must be numeric.")

        return original, target, [int(val) for val in values]

    @staticmethod
    def convert(original, target, value):
        output = None

        if original == "c":
            if target == "k":
                output = value + 273.15
            else:
                output = (value * 9 / 5) + 32

        if original == "k":
            if target == "c":
                output = value - 273.15
            else:
                output = ((value - 273.15) * 9 / 5) + 32

        if original == "f":
            if target == "k":
                output = (value - 32) * 5 / 9 + 273.15
            else:
                output = (value - 32) * 5 / 9

        return round(output, 2)


if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.sessions.on': True, }
        }
    cherrypy.tree.mount(ConvertService(), '/', conf)
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    cherrypy.config.update({'server.socket_port': 8080})
    cherrypy.engine.start()
    cherrypy.engine.block()
