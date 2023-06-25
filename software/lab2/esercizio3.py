# Ex3. Develop a client python application that emulates an IoT device, to invoke the RESTful Catalog developed in Exercise_1.
# This application must periodically (for example every 1 minute)
# either register a new device or refresh the old registration by updating its “insert-timestamp”.
# During the refresh of an old device registration, the Catalog must update also the “insert-timestamp”.

import requests
import json
import time
import random
import uuid

baseUrl = "http://localhost:8080"


def registerDevice(deviceID, endPoints, sensors):
    data = {
        "deviceID": deviceID,
        "endPoints": endPoints,
        "availableResources": sensors
    }
    response = requests.post(f'{baseUrl}/device', json=data)
    jsonData = response.json()

    return jsonData["deviceID"]


def updateDevice(deviceID, endPoints, sensors):
    data = {
        "deviceID": deviceID,
        "endPoints": endPoints,
        "availableResources": sensors
    }
    response = requests.put(f'{baseUrl}/device/{deviceID}', json=data)
    print(response.json())


def main():
    deviceIDs = []
    while True:
        # Randomly decide whether to register a new device or update an existing one
        if len(deviceIDs) == 0 or random.random() < 0.5:
            # Register a new device
            endpoints = ["http://example.com/endpoint1", "mqtt://example.com/topic1"]
            deviceID = registerDevice(uuid.uuid4(), ["http://example.com/endpoint1", "mqtt://example.com/topic1"], ["Temperature", "Humidity", "Motion sensor"])
            deviceIDs.append(deviceID)
        else:
            # Update an existing device
            deviceID = random.choice(deviceIDs)
            updateDevice(deviceID, ["http://example.com/endpoint1", "mqtt://example.com/topic1"], ["Temperature", "Humidity", "Motion sensor"])
        
        # Sleep for a minute
        time.sleep(60)


if __name__ == "__main__":
    main()
