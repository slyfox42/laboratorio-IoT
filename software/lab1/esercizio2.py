import cherrypy
import json


class ConvertService(object):
    exposed = True
    logs = []

    def GET(self, *path):
        if len(path) > 0 and path[0] == "convert":
            original, target, value = ConvertService.checkQuery(path)
            output = self.convert(original, target, value)

            return json.dumps({
                "originalValue": path[1],
                "originalUnit": path[2],
                "finalUnit": path[3],
                "outputValue": output
            })
        elif len(path) > 0 and path[0] == "log":
            return json.dumps(self.logs)
        else:
            return "Head over to /convert to convert temperature."

    def POST(self, *path):
        if len(path) > 0 and path[0] == "log":
            try:
                stringBody = cherrypy.request.body.fp.read()
                jsonBody = json.loads(stringBody)
            except:
                raise cherrypy.HTTPError(400, "Invalid JSON value.")
            else:
                if self.validateSenML(jsonBody):
                    self.logs.append(jsonBody)
                    return "Logs successfully posted."
                else:
                    raise cherrypy.HTTPError(400, "Body not in valid SenML format.")
        else:
            raise cherrypy.HTTPError(404, "Not Found.")

    @staticmethod
    def checkQuery(path):
        if len(path) != 4:
            print(len(path))
            print(path)
            raise cherrypy.HTTPError(400, "Missing required parameters: originalUnit, value, targetUnit.")

        units = ["k", "c", "f"]
        # convert to lowercase and destructure list
        value, original, target = [x.lower() for x in path[1:]]
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

    @staticmethod
    def validateSenML(jsonObject):
        baseFields = ["bn", "e"]
        recordFields = ["n", "t", "v", "u"]
        jsonBaseFields = jsonObject.keys()
        if jsonBaseFields == baseFields:
            jsonRecordFields = jsonObject["e"].keys()
            if jsonRecordFields == recordFields:
                return True
        return False


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
