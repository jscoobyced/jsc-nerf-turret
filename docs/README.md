# JSC Nerf Turret - How-To

## Compiling the code

### JSC-Bluetooth dependency

#### Quick Start
```
sudo apt install -y build-essential libtool libbluetooth-dev \
    libglib2.0-dev libdbus-glib-1-dev libdbus-1-dev libgio3.0-cil-dev
cd jsc-bluetooth/src
./autogen.sh
./buildme.sh install
cd ../..
```

#### Porject details

This project depends on the [jsc-bluetooth](https://github.com/jscoobyced/jsc-bluetooth) project for Bluetooh communication with the turret. Please follow its [README](https://github.com/jscoobyced/jsc-bluetooth/blob/main/README.md) on how to compile it from within the `jsc-bluetooth` folder in this project.

### Turret code

In order to build the project code, and after you have built the JSC-Bluetooth dependecy above, you can run the following commands:
```
sudo apt install libwiringpi-dev
cd src
./autogen.sh
./buildme.sh install
cd ..
```