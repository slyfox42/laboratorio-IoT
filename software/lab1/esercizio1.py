import cherrypy
import json


class ConvertService(object):
    exposed = True

    def GET(self, *path, **query):
        if len(path) > 0 and path[0] == "convert":
            original, target, value = ConvertService.checkQuery(query)
            output = self.convert(original, target, value)

            return json.dumps({
                "originalValue": query["value"],
                "originalUnit": query["originalUnit"],
                "finalUnit": query["targetUnit"],
                "outputValue": output
            })
        else:
            return "Head over to /convert to convert temperature."

    @staticmethod
    def checkQuery(query):
        params = list(query.keys())
        if ("value" not in params
                or "originalUnit" not in params
                or "targetUnit" not in params):
            raise cherrypy.HTTPError(400, "Missing required parameters: originalUnit, value, targetUnit.")

        units = ["k", "c", "f"]
        original = query["originalUnit"].lower()
        target = query["targetUnit"].lower()
        value = query["value"]

        if (original not in units
                or target not in units
                or original == target):
            raise cherrypy.HTTPError(
                400, "Must insert valid units of measurement.")

        if not value.isnumeric():
            raise cherrypy.HTTPError(
                400, "Value must be numeric.")

        return original, target, int(value)

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
