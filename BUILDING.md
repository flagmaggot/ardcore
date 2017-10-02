# Bill of Materials

Suggested Part | Qty | Value | Package | Parts | Description
---------------| --- | ----- | ------- | ----- | ------------
[9mm Alpha pots](https://www.thonk.co.uk/shop/alpha-9mm-pots-dshaft/) | 4 | 10k | ALPS_POT_VERTICAL       
[thonkicons](https://www.thonk.co.uk/shop/3-5mm-jacks/) | 11 | WQP-PJ301M-12_JACK | WQP-PJ301M-12_JACK      
n/a | 1 | Header 3x2 | 2X3 | AVR_ISP | Header 3x2
n/a | 1 | Header 5x2  AVR_ICSP | JP1 | Header 5x2
n/a | 4 | LED3MM | LED1, LED2, LED3, LED4  LED
81-GRM39C101J50 | 6 | 100n 1 C0605 | C1, C2, C3, C7, C27, C28 | CAPACITOR, European symbol
81-GRM1885C1H101GA1J | 2 | 100p | C0603 | C21, C22 | CAPACITOR, American symbol
667-EEE-FK1C470P | 2 | 47u | PANASONIC_D | C25, C30 | POLARIZED CAPACITOR, American symbol
81-GRM39C470J50 | 1 | 47p | C0603 | C29 | CAPACITOR, American symbol
667-ECE-A1VKS100 | 3 | 10u | E2,5-5 | C4, C5, C6 | POLARIZED CAPACITOR, European symbol
621-1N5819HW-F | 2 | 1N5819HW | SOD123 | D1, D2 | DIODE
78-1N4148W-E3-08 | 8 | 1n4148 | SOD80_DO213AA_MINIMELF | D3, D4, D5, D6, D7, D8, D9, D10 | 1N4148 General Purpose Rectifie
595-TL072CD | 1 | TL072 | SO08 | IC10 | OP AMP
511-L7805CDT-TR 1 | 7805DT | TO252 | IC2 | Positive VOLTAGE REGULATOR
579-MCP4921-E/SN | 1 | MCP4921 | SO08 | IC4 
595-SN74AHCT1G125DBV | 4 | 74AHCT1G125DBV | SOT23-5 | IC5, IC7, IC8, IC11 | Single Bus Buffer Gate with 3-State Output
556-ATMEGA328P-AU | 1 | ATMEGA328P-AU | TQFP32-08 | IC6 
863-MMBT3904LT1G | 2 | SOT23-BEC | Q1, Q5 | NPN Transistror
652-CR0603FX-2200ELF | 4 | __100 to 220__ | R0603 | R1, R2, R5, R33 | Resistor for LEDS, so depends on your leds color.
652-CR0603FX-1002ELF | 3 | 10k | R0603 | R3, R6, R7 | RESISTOR, American symbol
652-CR0603-FX-1003GLF | 3 | 100k | R0603 | R4, R13, R56 | RESISTOR, American symbol
652-CR0603FX-3902ELF | 2 | 39k | R0603 | R45, R46 | RESISTOR, American symbol
652-CR0603FX-2492ELF | 1 | 24.9k | R0603 | R50 | RESISTOR, American symbol
652-CR0603-FX1001HLF | 9 | 1K | R0603 | RC1, RC2, RC3, RC4, R32, R34, R38, R43, R55 | RESISTOR, European symbol
81-CSTCE16M0V53-R0 | 1 | RESONATORSMD | RESONATOR-SMD | Y1 | Resonator

# Building your Ardcore

1. Solder all the power components as show in the picture:

   * IC2 - 5V regulator 
   * D1, D2 - 1n5819
   * C5, C6 - 10u eletrolitic caps
   * R3 a 10k resistor for the MC reset
   * C1, C2, C3, C7 - 100n ceramic caps
   * ISP pin header for programming
   * 2x5 pin header for power

   ![power supply components]( images/ardcore-building-0.jpg)

2. test the power on the board, plug the eurorack power supply cable and check that nothing gets too hot, if you have a current measure tool its also a good practice to measure the module current consumption which should be a few milliamps. If you detect your module is compsuming more than 200mha then something is wrong. This way of testing your module is much better than directly plugin it into your euorack module so you avoid any damages to the rest of your modules.

3. Now its time to change the fuses of the atmega328p-au, this is important because by default the MC uses its internal oscillator (which is not accurate), so to let it know if should use an external oscillator you should use the avrdude tool and this command and your AVRPISP-mkii

```
[luix@boxita ardcore]$ sudo avrdude -c avrispmkII -p m328p -U lfuse:w:0xFF:m -U hfuse:w:0xDA:m -U efuse:w:0x05:m
[sudo] password for luix: 

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: reading input file "0xFF"
avrdude: writing lfuse (1 bytes):

Writing | ################################################## | 100% 0.01s

avrdude: 1 bytes of lfuse written
avrdude: verifying lfuse memory against 0xFF:
avrdude: load data lfuse data from input file 0xFF:
avrdude: input file 0xFF contains 1 bytes
avrdude: reading on-chip lfuse data:

Reading | ################################################## | 100% 0.00s

avrdude: verifying ...
avrdude: 1 bytes of lfuse verified
avrdude: reading input file "0xDA"
avrdude: writing hfuse (1 bytes):

Writing | ################################################## | 100% 0.01s

avrdude: 1 bytes of hfuse written
avrdude: verifying hfuse memory against 0xDA:
avrdude: load data hfuse data from input file 0xDA:
avrdude: input file 0xDA contains 1 bytes
avrdude: reading on-chip hfuse data:

Reading | ################################################## | 100% 0.00s

avrdude: verifying ...
avrdude: 1 bytes of hfuse verified
avrdude: reading input file "0x05"
avrdude: writing efuse (1 bytes):

Writing | ################################################## | 100% 0.01s

avrdude: 1 bytes of efuse written
avrdude: verifying efuse memory against 0x05:
avrdude: load data efuse data from input file 0x05:
avrdude: input file 0x05 contains 1 bytes
avrdude: reading on-chip efuse data:

Reading | ################################################## | 100% 0.00s

.....

avrdude done.  Thank you.

[luix@boxita ardcore]$
```

4. Since its an Arduino based project, and it the atmega328p-au comes completely blank we have to burn the bootloader, you need to chose the board "Arduno NANO w/Atmega328" on the board menu, and change your programmer to "AVRIS-mkii" on the programmer menu. To burn the bootloader  "Tools -> Burn bootloader".

5. If everything goes OK with the bootloader now you can upload sketches, you can try to upload the __test__ sketch we have prepared