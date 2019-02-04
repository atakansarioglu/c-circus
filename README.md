# C-Circus
Lightweight Circular Buffer implementation for ARM Cortex-M and other 32-bit architectures.

## Usage
Intended for interrupt driven UART communication. Simply use single-byte pusher and popper in your IRQ and multi-byte versions in the main thread code.
