# Ex3. Develop a client python application that emulates an IoT device, to invoke the RESTful Catalog developed in Exercise_1.
# This application must periodically (for example every 1 minute)
# either register a new device or refresh the old registration by updating its “insert-timestamp”.
# During the refresh of an old device registration, the Catalog must update also the “insert-timestamp”.

import requests
import json
import time
import random
import uuid

# Register a new device
def register_device(deviceID, endPoints, sensors):
    url = "http://localhost:8080/device"
    data = {"deviceID" : deviceID,"endPoints": endPoints, "sensors": sensors}
    response = requests.post(url, json=data)
    print(response.json())
    return response.json()["deviceID"]

# Update a device
def update_device(deviceID, endPoints, sensors):
    url = f"http://localhost:8080/device/{deviceID}"
    data = {"deviceID" : deviceID,"endPoints": endPoints, "sensors": sensors}
    response = requests.put(url, json=data)  # Assuming that you have a PUT method to update device
    print(response.json())

def main():
    deviceIDs = []
    while True:
        # Randomly decide whether to register a new device or update an existing one
        if len(deviceIDs) == 0 or random.random() < 0.5:
            # Register a new device
            deviceID = register_device(uuid.uuid4(), ["http://example.com/endpoint1", "mqtt://example.com/topic1"], ["Temperature", "Humidity", "Motion sensor"])
            deviceIDs.append(deviceID)
        else:
            # Update an existing device
            deviceID = random.choice(deviceIDs)
            update_device(deviceID, ["http://example.com/endpoint1", "mqtt://example.com/topic1"], ["Temperature", "Humidity", "Motion sensor"])
        
        # Sleep for a minute
        time.sleep(60)

if __name__ == "__main__":
    main()
