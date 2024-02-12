import paho.mqtt.client as mqtt
import time
import os

# MQTT Broker Configuration
broker_address = "192.168.1.6"
broker_port = 1883
base_topic = "example/fota"
# File Path
file_path = "C:/Users/User/Desktop/myscratchsnake22.hex"
#file_path = "C:/Users/User/Desktop/APP2.hex"

def on_publish(client, userdata, mid):
    print("Message Published")

def publish_file(file_path, base_topic, chunk_size=1500):
    client = mqtt.Client()

    # Set the on_publish callback
    client.on_publish = on_publish

    # Connect to the MQTT broker
    client.connect(broker_address, broker_port, 60)

    # Get the file size
    file_size = os.path.getsize(file_path)

    # Open the file
    with open(file_path, 'r') as file:
        #client.publish(base_topic,(f"Size:{file_size}\n"))
        #time.sleep(0.9)
        while file.tell() < file_size:
            # Read a chunk of data
            chunk = file.read(chunk_size)

            # Create a subtopic based on file position
            #subtopic = f"{base_topic}/{file.tell()}"
            # Publish the chunk to the subtopic
            client.publish(base_topic, chunk)

            # Wait for a short duration (adjust as needed)
            time.sleep(0.5)
        client.publish(base_topic, "Done")

    # Disconnect from the MQTT broker
    client.disconnect()

if __name__ == "__main__":
    publish_file(file_path, base_topic)
