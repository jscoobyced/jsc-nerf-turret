import sys

import bluetooth


addr = None

if len(sys.argv) < 2:
    print("No device specified. Searching all nearby bluetooth devices for "
          "the requested service...")
else:
    addr = sys.argv[1]
    print("Searching for the requested service on {}...".format(addr))

# search for the SampleServer service
uuid = "AD423591-A6EC-495A-996B-5B56B5704E52"
service_matches = bluetooth.find_service(uuid=uuid, address=addr)

if len(service_matches) == 0:
    print("Couldn't find the requested service.")
    sys.exit(0)

first_match = service_matches[0]
port = first_match["port"]
name = first_match["name"]
host = first_match["host"]

print("Connecting to \"{}\" on {} port {}".format(name, host, port))

# Create the client socket
sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
sock.connect((host, port))

print("Connected. Type something...")
while True:
    data = input()
    if not data:
        break
    sock.send(data + "\n")

sock.close()