# Dopre

Custom Electrostatic Capacitive Keyboards.

Dope + Topre = DOPRE.

Dopre Maintainer: [Drew Mills](https://github.com/franticrain)


## About Electrostatic Capacitive

Colloquially referred to using the brand name, Topre, electrostatic capacitive keyboards use a conical spring to generate a variable capacitance as a key is depressed. This variable capacitance is then read to determine if a key has been actuated. Dopre is an implementation of reading these types of keyboards within QMK.

## Building a Dopre-compatible Keyboard

At this time, Dopre is only confirmed to work on ARM platforms. The code exists for AVR platforms, but is untested, and is likely far too slow for keyboards of appreciable size. For this reason, it is recommended that if you are trying to implement a Dopre keyboard, you will need to use an ARM microcontroller.

### Initial Design Concerns

#### Operation Speed

The process of electrostatic capacitive sensing that Dopre uses involves using an analog to digital converter that is included on the microcontroller included in the keyboard. This converter is often slow. This severely slows down the reading process of a keyboard. For example, the "Southpaw" keyboard has 104 pins and scans at a rate slightly over 2KHz. This is using the fastest possible configuration for the ADC on the STM32F3, which is a 6-bit accuracy with minimal sample-and-hold time. In order for a keyboard to feel responsive, it needs to be 1KHz **at least**.

Also note that the Dopre solution uses a RC (resistor-capacitor) circuit to sense the capacitance of a keyswitch. This is notable because this process requires time to charge a capacitor. This is inherently a time limited process. However, it appears that QMK is slow enough that waiting for this circuit to charge is not required. If QMK's performance is significantly improved, or if microcontrollers get significantly faster, care may need to be taken to add "wait"s to the code that allow these circuits to charge.

Similarly, an analog multiplexer is used as part of the operation of the keyboard. This introduces a latency and has it's own switching time. This would need to be accounted for if significant gains in performance are made.

An operational amplifier is also used as part of the solution. It will also introduce latency into the solution. A further coverage of component selection will be covered later.

#### Parasitic Capacitance

As this is a capacitance-sensing solution, designers will need to consider parasitic capacitance in their PCB design. Complete coverage of the topic is outside the scope of this document, but further reading on the subject is recommended before embarking upon a Dopre PCB design.

### Capacitance Sensing

[The solution used to sense capacitance in the switches is identical to the solution defined by Tom Smalley](https://github.com/tomsmalley/custom-topre-guide). Dopre is largely a software solution built upon his initial work. For further documentation on the hardware implementation, please see his guide. In summary, a strobe will be sent out along each column, one row at a time. This strobe will allow an RC circuit to charge, generating a voltage relative to the capacitance level. That voltage is then passed through a non-inverting operation amplifier in order to return the voltage to reasonable levels for the ADC on the microcontroller to consume.

Note: The Dopre solution also supports column to row sensing, but it is not recommended, as the side with the least lines (most keyboards have less rows than columns) should be the side that analog read is completed from, in order to reduce the load on the analog multiplexer.

Note that the Dopre solution does not implement any of the normalization or calibration code suggested in that documentation. In order to maximize performance and decrease code complexity, it was deemed non-vital. This means that analog values read from Dopre sensing shouldn't be used for any sort of analog input. They are accurate only for digital input purposes. Additionally, this means that no individual calibration needs to be completed for a board, making board implementation simpler. Users may find minor differences in the actuation point of keys, but generally this difference will be so insignificant that a user shouldn't ever notice it.

### Component Selection

There are several components that need to live up to certain performance requirements in order to ensure that the Dopre solution can be implemented successfully. These components are:
- The microcontroller's ADC
- Analog multiplexer
- Operational amplifier

#### Microcontroller

As noted previously, the Dopre implementation is currently only tested for ARM devices. As such, it is recommended that only ARM devices are used for Dopre implementations.

When selecting a microcontroller, the most important aspect (for Dopre's purposes) is the speed of the ADC. Both of the popular ARM microcontroller options (STM32F3XX and STM32F0XX) have the important features of fast ADCs and ADC support in QMK. There is some configuration that will need to be made to these devices that will be covered in a later section.

QMK opts for a standard 10-bit solution for ADC sensing for AVR devices. This, combined with the general speed of AVR devices means that they are largely unsuitable for Dopre operation. However, if an appropriate device can be found, it isn't impossible to implement an AVR-based solution.]

### PCB Design

#### Pad Layout

Pads should be laid out identically to how they would be in a standard digital-based keyboard. Care will simply need to be taken in routing traces in order to avoid parasitic capacitance that might influence readings of the switches.

#### Sensing Circuitry

Once again, the sensing circuitry should be identical to that listed in [Tom Smalley's guide](https://github.com/tomsmalley/custom-topre-guide). The guide also serves as an excellent visual reference of the content of this section. Simply put, each column from the microcontroller will run to a column of pads, just as they would with a digital-based keyboard. The rows that come off of these keyboards will then all be connected to their own individual 22k Ohm resistor that is tied to ground (a pull-down resistor). These resistors are marked R5 and higher in Smalley's guide. It is important that each row has its own pull-down resistor. Each of these rows is then connected to an input channel of an analog multiplexer. This analog multiplexer will have its "select" lines tied to pins on the microcontroller. The output line of the analog multiplexer will then lead to the main RC circuit used to sense capacitance.

This RC circuit section consists of the components marked C1, R1, and R3. When sensing, R1 will be tied to a floating "drain" pin. This means that it can be ignored for the purposes of sensing. This RC circuit will charge to a voltage level corresponding to the capacitance of the switch that was depressed. When sensing is complete, the "drain" pin can be pulled to ground, producing a significant resistive load that will help discharge the circuit faster.

The output of this RC circuit is then fed into a non-inverting op-amp in order to return it to the 3.3V (or 5V) logic levels of the microcontroller so that the ADC can take maximum advantage of the available sensing range. It is technically possible that a reference voltage could be used instead of the non-inverting operation amplifier, but that implementation can be left as an exercise to the reader. Note that the R2 and R4 in this circuit are non-standard resistances. It is recommended to use a 46k Ohm and 10k Ohm resistor in series to get R2 and a 470 Ohm and 680 Ohm in parallel to achieve R4.

#### Pin Selection

Note that the Dopre solution uses a slightly different set of pins from a digital-based keyboard. You will need to assign a pin to each column, as expected, but you will not need to assign pins to the rows. Instead you will need to assign an analog-capable pin to be the "read" pin, and digital pins for the drain pin, and multiplexer selection lines. Note that for keyboards with only four rows, you will only need two selection pins. For most other applications, you will need three selection pins.

Note that the need for rows and columns can be swapped if the sensing circuitry is implemented the other way around.

### QMK Configuration

For any lingering questions about QMK configuration for a Dopre board, consult with the "Southpaw" layout, as it is a fairly simplistic implementation.

#### Matrix Configuration

For basic matrix configuration, you will need to assign the basic values:

```C
#define MATRIX_ROWS 6
#define MATRIX_COLS 18
#define MATRIX_ROW_PINS { }
#define MATRIX_COL_PINS { B0, B1, B2, B3, B4, B5, B6, B7, A9, A10, A8, A15, A13, A14, B11, B12, B15, B14 }
```

Note that `MATRIX_ROW_PINS` is left as an empty array. This is to silence QMK build warnings, as we won't actually be assigning any pins to that purpose. Do note that we still have to list the number of rows that will be used for our matrix.

#### Dopre Configuration

```C
#define HIGH_LATCH 58
#define HIGH_UNLATCH 48
#define DRAIN_PIN B10
#define ADC_READ_PIN A0
#define MATRIX_MUXS 3
#define MUX_PINS { A2, A1, B8 }
```

We first define the `HIGH_LATCH` and `HIGH_UNLATCH`. These two values are used to define the analog values at which the key is considered "activated" and swaps back to "unactivated", respectively. Following our example, this means that our switches aren't actuated until the ADC reads a 58 (out of 63). Once this threshold is met, the key is considered to be held until the value drops back down to 48 (out of 63). This helps debounce the switch and avoid issues caused by inconsistent ADC readings.

We then defined which pin we have assigned to be the drain pin and the read pin. Remember that the `ADC_READ_PIN` needs to be connected to an ADC. We then define the number of multiplexer selection pins we will need with `MATRIX_MUXS` and then assign the pins in `MUX_PINS`. Note that the order they are defined is least-significant bit first. So the first value will be connected to the `S0` line of the analog multiplexer.

#### ARM Configuration

Continuing in our `config.h`, we need to add these lines:

```C
#define ADC_BUFFER_DEPTH 1
#define ADC_RESOLUTION ADC_CFGR1_RES_6BIT
```

This is configuration for the ARM-specific ADC implementation in QMK that allows us to define the ADC samples as having 6 bit accuracy, and define the samples to use a 8-bit (1 byte) type instead of the standard 16-bit (2 byte) type.

##### `halconf.h`

```C
#    if !defined(HAL_USE_ADC) || defined(__DOXYGEN__)
#        define HAL_USE_ADC TRUE
#    endif
```

Of course, we need to turn on the ADC, or we won't be able to read any analog pins!

##### `mcuconf.h`

```C
#define STM32_ADC_COMPACT_SAMPLES TRUE
#define STM32_ADC_USE_ADC1 TRUE
```

`STM32_ADC_COMPACT_SAMPLES` is the configuration that actually tells ChibiOS to use those 8-bit samples. We then turn on the ADC that we are going to use. On the STM32F0, this is the only ADC. However, there are 4 ADCs on the STM32F3, so pay attention to which one you are using, and which ADC you need to activate.
