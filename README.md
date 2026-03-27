<h1 align="center">
    <br>
    <img src=".github/imgs/ciotto.png" alt="CHIP-8" width="500">
    <br>
    ciotto - A CHIP-8 Emulator in C
    <br>
</h1>

## Usage

First, clone the repository and build the project:

```bash
git clone https://github.com/AntonioBerna/ciotto.git
cd ciotto

# build
make

# clean build
make clean
```

Then, you can run the emulator with a CHIP-8 ROM:

```bash
./chip8 --emu <rom.ch8>
```

> [!NOTE]
> You can find some CHIP-8 ROMs in the [roms](https://github.com/AntonioBerna/ciotto/tree/master/roms) directory.
