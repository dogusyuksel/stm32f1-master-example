
#  PCAN-USB FD pcan-linux-installation

This guide is about the istallation and running of the `PCAN-USB FD` and running test using socketcan, with your custom CAN profile.
This is the product page : https://www.peak-system.com/PCAN-USB-FD.365.0.html?&L=1

The official documentation can be found here : https://www.peak-system.com/fileadmin/media/linux/files/PCAN-Driver-Linux_UserMan_eng.pdf

This guide is for using the new `PCAN-USB FD` with the support of the CAN-FD frame using updated `socketCAN`
 
This is a quick guide on how to install linux driver for the pcan usb adapter and run some demo test setup.

This setup is tested on the moteus-motor-drivers which require custom CAN parameter to be configured : 
## Installion specs. of system :
* Ubuntu 22.04 (*Kernel version : * Linux ASUS 6.0.0-1016-oem )
* PCAN-USB FD 
* Mjbots-Moteus-r4.11
* Peak Linux Driver version : peak-linux-driver-8.16.0
* CAN Configuration parameter from : [reference.md](https://github.com/mjbots/moteus/blob/main/docs/reference.md#80-mhz-clock-systems)

# Installation step
1. Go to the offical website of the peak : https://www.peak-system.com/fileadmin/media/linux/index.htm or 
  
     1. Navigate to this section : `Driver Package for Proprietary Purposes` on the above given website.
     2. Click on `Download PCAN Driver Package` to download the latest driver.(This setup is tested on the driver version : `8.16.0` )
     3. You can also install pcan view from website section `PCAN-View for Linux` or follow these command : 
        ```bash
        # add peak-system.list in the update list
        
        $ wget -q http://www.peak-system.com/debian/dists/`lsb_release -cs`/peak-system.list -O- | sudo tee /etc/apt/sources.list.d/peak-system.list

        $ wget -q http://www.peak-system.com/debian/peak-system-public-key.asc -O- | sudo apt-key add -
        
        $ sudo apt-get update
        $ sudo apt-get install pcanview-ncurses
        ``` 

2. Now install these following pacakage in your system : 
    ```bash
    # install required packages
    sudo apt-get install libncurses5
    sudo apt-get install can-utils
    sudo apt-get install gcc-multilib
    sudo apt-get install libelf-dev
    sudo apt-get install libpopt-dev
    sudo apt-get install tree
    # build and install pcan driver

    tar -xzf peak-linux-driver-X.Y.Z.tar.gz # navigate to download path
    cd peak-linux-driver-X.Y.Z
    make clean
    make NET=NETDEV_SUPPORT  # for network interface based pcan 
    sudo make install
    
    # install the modules   
    sudo modprobe pcan
    sudo modprobe can
    sudo modprobe vcan
    sudo modprobe slcan
    sudo modprobe pcan_usb

    # setup and configure "can0" net device
    
    sudo ip link set can0 type can
    
    sudo ip link set can0 up type can tq 12 prop-seg 25 phase-seg1 25 phase-seg2 29 sjw 10 dtq 12 dprop-seg 6 dphase-seg1 2 dphase-seg2 7 dsjw 12 fd on  loopback on
    ```
    **Note** : If you want to update a new configuration try to turn off the `can0` network and
    ```bash
    sudo ip link set can0 down

    # then again run the new parameter of CAN command again 
    sudo ip link set can0 up type can tq 12 prop-seg 25 phase-seg1 25 phase-seg2 29 sjw 10 dtq 12 dprop-seg 6 dphase-seg1 2 dphase-seg2 7 dsjw 12 fd on  loopback on
    ``` 
3. Once you made the installation let's check :
    ```bash
    # check installation
    ./driver/lspcan --all       # should print "pcanusb32" and pcan version
    tree /dev/pcan-usb          # should show a pcan-usb device
    ip -a link                  # should print some "can0: ..." messages
    ip -details link show can0  # should print some details about "can0" net device
    ```
4. Example output after successfull installation of a pcan usb adapter:
    ```bash 
    mighty@ASUS:~/Downloads/peak-linux-driver-8.16.0$ ./driver/lspcan --all
    pcanusbfd32     CAN1    -       80MHz   1M+5M   WARNING 0.00    4       2       114461

    mighty@ASUS:~/Downloads/peak-linux-driver-8.16.0$ tree /dev/pcan*
    /dev/pcan32  [error opening dir]
    /dev/pcan-usb_fd
    └── 0
        └── can0 -> ../../pcanusbfd32
    /dev/pcanusbfd32  [error opening dir]

    1 directory, 1 file

    mighty@ASUS:~/Downloads/peak-linux-driver-8.16.0$ ip -details link show can0
    21: can0: <NOARP,UP,LOWER_UP> mtu 72 qdisc pfifo_fast state UP mode DEFAULT group default qlen 10
        link/can  promiscuity 0 minmtu 0 maxmtu 0 
        can <LOOPBACK,FD> state ERROR-WARNING (berr-counter tx 127 rx 0) restart-ms 0 
            bitrate 1000000 sample-point 0.637 
            tq 12 prop-seg 25 phase-seg1 25 phase-seg2 29 sjw 10
            pcan: tseg1 1..256 tseg2 1..128 sjw 1..128 brp 1..1024 brp-inc 1
            dbitrate 5000000 dsample-point 0.562 
            dtq 12 dprop-seg 6 dphase-seg1 2 dphase-seg2 7 dsjw 12
            pcan: dtseg1 1..32 dtseg2 1..16 dsjw 1..16 dbrp 1..1024 dbrp-inc 1
            clock 80000000 numtxqueues 1 numrxqueues 1 gso_max_size 65536 gso_max_segs 65535 
    ```
5. Now, Once we have done with the installation, we will try to send the the can data and receive the can data as well : 
   
   1. Run this command in one terminal :
        ```bash
        cansend can0 00008001##0420120
        # where  fdcan id is 00008001 on can0
        # the '##' represent the fdcan frame
        # and the first bit is '0' represent 'without switching bitrate'
        # the data is '420120'
        ```
    
   2.  Run this command in another terminal :
        ```bash
        candump -x can0
        ``` 
6. The output of the above command if you are using `moteus-r4.11` :
    ```bash
    # in the first terminal 
    mighty@ASUS:~$ cansend can0 00008001##0420120

    #in the second terminal
    mighty@ASUS:~$ candump -x can0
      can0  TX - -  00008001  [03]  42 01 20
      can0  RX - -  00008001  [03]  42 01 20
      can0  RX - -       100  [03]  41 01 00
    ```


## Reference :

* PCAN-USB FD : https://www.peak-system.com/PCAN-USB-FD.365.0.html?&L=1
* Peak Linux system : https://www.peak-system.com/fileadmin/media/linux/index.htm 
* mjbot moteus r4.11 CAN id setting: https://github.com/mjbots/moteus/blob/main/docs/reference.md#80-mhz-clock-systems
* mjbot moteus r4.11 : https://mjbots.com/products/moteus-r4-11
* can-utils guide: https://dissec.to/kb/chapters/can/canfd.html
* peak usb guide :https://github.com/SICKAG/sick_line_guidance/blob/master/doc/pcan-linux-installation.md
* CAN Configuration parameter : https://github.com/mjbots/moteus/blob/main/docs/reference.md#80-mhz-clock-systems

If you want to make any suggestion please comment below or reach at twitter [@pranav083](https://twitter.com/pranav083)


GCC12 maybe needed,
cd /usr/bin
sudo rm gcc
sudo ln -s gcc-12 gcc
gcc --version