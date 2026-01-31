# ch32fun_micropython
MicroPython port for WCH ch32 and ch5xx using ch32fun

This is very much a work in progress, and since I mostly have ch5xx's these
will receive support first.

## installation:
Clone three things:
1. https://github.com/cnlohr/ch32fun (join the discord here for support, also for this project)
2. https://github.com/micropython/micropython/
3. This repository.
Point the paths at the top of the Makefile to the ch32fun and micropython working directories, and run `make` or `make clean all`.

## roadmap
The plan is to support all RISC-V chips from WCH, which are quite a few.
This is only possible because `ch32fun` exists, which is a unified SDK
for all these, instead of all the separate EVTs provided by the vendor.
There will be a separate `ch32fun` module which implements specific libraries
that replace the vendor blobs, with submodules like `iSLER` for RF, `NFC` for
the NFC peripheral on the CH584/5 and `ch5xx_flash` for reading and writing flash
on the ch5xx series. Furthermore all the standard peripherals will be available in
the `machine` module like on the other MicroPython ports.

- [x] machine
	- [x] Pin
	- [ ] ADC
	- [ ] DAC
	- [ ] PWM
	- [ ] RTC
	- [ ] TIMER
	- [ ] UART
	- [ ] I2C
	- [ ] SPI
	- [ ] I2S
- [x] ch32fun
	- [x] RAM access
	- [x] System Register access
	- [-] flash access
	- [-] USB
	- [ ] iSLER
	- [ ] NFC
