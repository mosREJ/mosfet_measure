# Automatic MOSFET measurement system using remote controlled lab equipment

(WIP)

## Hardware prerequisites:

The app requires:

- a programmable DC power supply,

- a programmable multimeter, capable of measuring resistance.

Each of the devices has to have a communication protocol compliant with the IEE-488.3 standard.

This software is being tested on:

- KORAD KA3005P programmable DC power supply,

- SDM3055 programmable multimeter.

## Building and running 

```
mkdir build

cd build 

cmake ..

make -j`nproc`
```

``` 
./fetmes --multimeter=<ip of your multimeter> -mmport=<port used by your multimeter> \
--powersupply-serial=<path to the powersupply serial devicee (e.g. /dev/ttACM0)> \ 
--pwbaudrate=<power supply serial baudrate 9600/115200>
``` 

Alternatively if your power supply uses TCP, you may use `--powersuppy=<ip> --psport=<port>` and vice versa.
There's also help list if you have any doubts:

```
./fetmes --help
```

## API

See: src/cmds.h



# TODO:
- [ ] allow to use multimeter/powersuppy separately 
- [ ] add more crucial commands e.g. current limit before measure session 
- [ ] check max/optimal delay between commands 
- [ ] check how measure with auto range on\
- [ ] add write to file e.g. `tee` 
- [ ] ?? add  smth that checks overcurrent occured to many times e.g. 5 times in a row
- [ ] try python's lib for 3D printer control 
- [ ] add device names in linux (e.g. `usb-LeafLabs_Maple_if00` -> 3dprinter
- [ ] add flags "gcc -std=c89 -Wall -Wextra -pedantic -o main.out main.c'
- [ ] **implement all on RP4** 
- [ ] room temperature sensor
- [ ] light sensor?
- [ ] **find which method is better: Y1->Y2 all time or Y1 -> Z up -> Y2 -> Z down**
- [ ] **print square verision of holder**
- [ ] stworzyć algorytm, który sam bd wracał do pozycji Y start -> zmieni X tak aby mierzyć ale nie wyjść poza krawędź 
    - [ ] np. sprawdzić czy normalnie takie wafle bd przewodzić/powinny przewodzić -> wtedy manipulując YZX moze sam sprawdzać czy nie wyszedł (wymiary od usera) jako podpowiedź
- [ ] choose mechanism which allows to log to at least 2 indepented files in order to minimalize lost of data.
- [ ] R of current source as parameter 
- [ ] **!!! Gdy chcemy mierzyć cały wafel od razu:**
    - [ ] wszerz:
        - [ ] ustawia X -> leci Y0->Ykoniec 
        - [ ] wraca na Y0 -> zmienia X i powtórka 
        - [ ] **!!! za każdym przejściem musi iść zapis do pliku, aby (jak w przypadku) terminala zapis nie następował dopiero przy końcu programu**
- [ ] **!!!!! taki 3d plot jak góry/wysokość terenu - rezystancja XY**
- [ ] te wykresy se strony pythona

> powersuppy part:

- [ ] make sure that appropritate current limit, memory is set 
- [ ] off/on after/during measure
- [ ] ?? feedback if overcurrent presents too long
- [ ] better funcions name

> measure part:
- [ ] ?? check how long takes to stabilizie multimeter with auto option enabled 
- [ ] find avr time response

> float problem: 

```c
#define start_Z     5.0
#define opti_Z      2.6
#define step        0.2
#define limit_conts 1.2 // arbitrary choosen to prevent going beyond edge of wafer
#define wafer_len   15.20 
#define wafer_width 9.7
#define start_Y     66.0
#define stop_Y      52.5 // to stop on 52.6
#define start_X     99.6
#define stop_X      97.0


    const int cnt_step = (start_Y - stop_Y) / 0.2;
    float minstep = 0.0;
    printf("cnt_step: %d\n", cnt_step);

    for (int i = 0; i <= cnt_step; ++i)
    {
        printf("%d\n", i);
        printf("%f\n", minstep = start_Y - (i * step));

    }

```

## Devices name/port/path

- Linux:
    - Power supply and 3d printer:
        - `ll /dev/serial/by-id/` (ttyxxx)
    - multimeter:
        1. Connect multimeter to LAN. 
        2. Click `shift` + `Dual` to enter `utility` menu
        3. Choose `I/O config` (panel under display)
        4. Choose `Lan settings`
        5. Choose `modify setting`
        6. Read IP addr and put it to build script/use as arg
        7. Defualt port: 5025 (RTFM)
- Windows: 
    - open device manager 

- **usb-LeafLabs_Maple-if00** -> 3D printer
- **usb-USB_Vir_USB_Virtual_COM_NT2009101400-if00** -> powersuppy 

## How it works/steps


- jak używamy ustawienie przez drukare -> zawsze wracac do info screena (moze w kodzie to ustawić) -> dzięki temu pozycje drukary się updetują po stronie kompa usera i kompa drukarki
- recznie na poczatek sobie ustawaic wafel, aby nie wbijało -> ustalić granice x, y
### Potential problems 

- tee doesnt print to stddout: https://stackoverflow.com/questions/41026503/tee-output-not-appearing-until-cmd-finishes https://stackoverflow.com/questions/71040342/why-would-tee-fail-to-write-to-stdout


