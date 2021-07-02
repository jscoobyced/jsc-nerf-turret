import bluetooth

server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
server_sock.bind(("", 22))
server_sock.listen(1)

port = server_sock.getsockname()[1]

uuid = "AD423591-A6EC-495A-996B-5B56B5704E52"

bluetooth.advertise_service(server_sock, "JSC Nerf Turret", service_id=uuid,
                            service_classes=[uuid, bluetooth.SERIAL_PORT_CLASS],
                            profiles=[bluetooth.SERIAL_PORT_PROFILE],
                            )

print("Waiting for connection on RFCOMM channel", port)

client_sock, client_info = server_sock.accept()
print("Accepted connection from", client_info)

try:
    while True:
        data = client_sock.recv(1024)
        if not data:
            break
        print("Received", str(data))
except OSError:
    pass

print("Disconnected.")

client_sock.close()
server_sock.close()
print("All done.")