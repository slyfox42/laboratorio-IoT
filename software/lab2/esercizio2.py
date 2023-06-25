# Ex2. Develop a client python service for invoking the RESTful Catalog developed in Exercise_1

import requests
import json
import uuid

# POST data to register a new device
def register_device(endPoints, sensors, uuid):
    url = "http://localhost:8080/device"
    data = {"deviceID" : uuid,"endPoints": endPoints, "sensors": sensors}
    response = requests.post(url, json=data)
    return response.json()

# GET data for a specific device
def get_device(deviceID):
    url = f"http://localhost:8080/device/{deviceID}"
    response = requests.get(url)
    return response.json()

# POST data to register a new service
def register_device(endPoints, description, uuid):
    url = "http://localhost:8080/services"
    data = {"serviceID" : uuid,"endPoints": endPoints, "description": description}
    response = requests.post(url, json=data)
    return response.json()

# GET data for a specific service
def get_device(serviceID):
    url = f"http://localhost:8080/service/{serviceID}"
    response = requests.get(url)
    return response.json()

# GET data for all devices
def get_all_devices():
    url = "http://localhost:8080/devices"
    response = requests.get(url)
    return response.json()

# POST data to register a new user
def register_user(uuid, name, surname, email):
    url = "http://localhost:8080/user"
    data = {"userID" : uuid,"name": name, "surname": surname, "email": email}
    response = requests.post(url, json=data)
    return response.json()

# GET data for a specific user
def get_user(userID):
    url = f"http://localhost:8080/user/{userID}"
    response = requests.get(url)
    return response.json()

# GET data for all users
def get_all_users():
    url = "http://localhost:8080/users"
    response = requests.get(url)
    return response.json()


if __name__ == '__main__':
    # Register a new device
    response = register_device(uuid.uuid4(),["http://example.com/endpoint1", "mqtt://example.com/topic1"], ["Temperature", "Humidity", "Motion sensor"])
    print(response)
    # Get data for a specific device
    print(get_device(response["deviceID"]))

    # Get data for all devices
    get_all_devices()
    # Register a new user
    response = register_user(uuid.uuid4(),"John", "Doe", "john.doe@example.com")
    print(response)
    # Get data for a specific user
    print(get_user(response["userID"]))

    # Get data for all users
    print(get_all_users())