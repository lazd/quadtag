# quadtag
> An Arduino-based laser tag system for quadcopters

## Parts List

| Part | Usage | Quantity | Number | Price each |
|------|-------|----------|--------|------------|
| Arduino Pro Micro 5V | Brain | 1 | - | [$19.95](https://www.sparkfun.com/products/12640) |
| 38KHz IR Reciever | Hit detectors | 2 | [Vishay TSOP4838](http://www.mouser.com/ds/2/427/tsop48-542449.pdf) | [$1.27](http://www.mouser.com/Search/ProductDetail.aspx?R=TSOP4838virtualkey61370000virtualkey782-TSOP4838) |
| 980nm 5mW Laser | IR Laser | 1 | Aixiz AH980-51230 | [$7.80](http://www.aixiz.com/store/product_info.php/cPath/67/products_id/365) |
| Piezo Buzzer | Hit indicator buzzer | 1 | [TDK SD1209T5-A1](http://www.mouser.com/ds/2/400/ec211_sd-558554.pdf) | [$2.12](http://www.mouser.com/Search/ProductDetail.aspx?R=SD1209T5-A1virtualkey52130000virtualkey810-SD1209T5-A1) |
| Red LEDs | Hit indicator LEDs | 4 | [Vishay TLCR5100](http://www.mouser.com/ds/2/427/tlcx510-266692.pdf) | [$0.51](http://www.mouser.com/Search/ProductDetail.aspx?R=TLCR5100virtualkey61370000virtualkey78-TLCR5100) |
| 1W 15ohm resistor | Hit indicator resistor | 1 | [MOS1CT52R150J](http://www.mouser.com/ds/2/219/MOS-16613.pdf) | [$0.10](http://www.mouser.com/ProductDetail/KOA-Speer/MOS1CT52R150J/?qs=sGAEpiMZZMtlubZbdhIBIMDn16p%2fHRN%2f254PcrlYqZg%3d) |

[Parts list on Mouser](https://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=2984454573)

### Brain

The Aruduino Micro was chosen for its built-in USB port for easy updates and customization. Other boards, such as the Arudino Pro Mini, would require a FTDI cable for programming.

### IR Reciever

The Vishay TSOP4838 was chosen for:

* Insensitive to supply voltage ripple and noise
  * No additional capacitor is required in the circuit
* Improved immunity against ambient light
  * Unlike the cheaper VS1838B, Vishay's TSOP* line of receivers won't be triggered unintentionally by sunlight [†](http://www.analysir.com/blog/2014/12/08/infrared-receiver-showdown-tsop34438-vs-vs1838b-winner-revealed/)
* Wide angle of half transmission distance ϕ = ± 45°
  * This means hits can be detected over 90 degrees
* Most sensitive at 950nm wavelength is a close match to the chosen IR laser
* The TSOP34438 was tested and did not produce reliable results

### IR Transmitter

The Aixiz 980nm 5mW laser module was chosen for:

* Focusability
* 980nm wavelength is a close match to the IR reciever
* It's a laser

## Wiring

![Breadboard](http://i.imgur.com/YcDN9bD.png)

![Breadboard](http://i.imgur.com/1rDpGhW.png)


