# Not Enigma

A small Win32 desktop application that demonstrates classical substitution ciphers. The app lets you encrypt or decrypt text using the Caesar, Vigenere, and Atbash ciphers. Choose a cipher, provide the shift (for Caesar) or keyword (for Vigenere) when applicable, pick an alphabet ordering, then click **Encrypt** or **Decrypt** to see the transformed text. A cross-platform console companion is also available for environments without the Win32 GUI.

## Building (Windows)

This project uses CMake. From a Visual Studio Developer Command Prompt:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable (`NotEnigma.exe`) will be located in `build/Release/`.

To build only the GUI application, ensure the `NOT_ENIGMA_BUILD_GUI` option remains enabled (it is `ON` by default on Windows).

## Console build

The console interface can be built on any platform with a C++17 toolchain:

```bash
cmake -S . -B build -DNOT_ENIGMA_BUILD_GUI=OFF
cmake --build build
```

On Windows you can omit `-DNOT_ENIGMA_BUILD_GUI=OFF` to build both the GUI and console targets simultaneously.

## Running the console application

After building, run the console interface from the build directory:

```bash
./NotEnigmaConsole
```

Follow the prompts to choose a cipher, provide the relevant keyword or shift, and select the alphabet preset (normal, reverse, random, or a custom ordering). The result is printed directly to the terminal. For example:

```
$ ./NotEnigmaConsole
Not Enigma Console
===================

Choose cipher:
  1) Caesar
  2) Vigenere
  3) Atbash
Selection: 1

Choose action:
  1) Encrypt
  2) Decrypt
Selection: 1

Enter text: Hello World
Enter shift value: 3

Choose alphabet preset:
  1) Normal (ABCDEFGHIJKLMNOPQRSTUVWXYZ)
  2) Reverse (ZYXWVUTSRQPONMLKJIHGFEDCBA)
  3) Random
  4) Custom
Selection: 1

Result:
Khoor Zruog
```

To decrypt, choose option **2** when prompted for the action or supply a negative shift value.

## Testing cipher correctness

Automated regression tests verify that Caesar, Vigenere, and Atbash encryption and decryption behave as expected. To build and run the test suite:

```bash
cmake -S . -B build -DNOT_ENIGMA_BUILD_GUI=OFF
cmake --build build
ctest --test-dir build
```

The tests exercise well-known cipher examples (such as the classic "ATTACKATDAWN" Vigenere case) to confirm the implementations remain correct.

## Notes

- The Caesar cipher accepts positive or negative numeric shifts via the **Shift** field.
- The keyword field is only available when the Vigenere cipher is selected. Characters not present in the active alphabet are ignored, and at least one valid character is required.
- All ciphers use the alphabet field to define their working alphabet. Leave it empty to fall back to the standard English alphabet.
- When Caesar or Atbash is active, use the **Alphabet Preset** drop-down to quickly choose the normal alphabet, its reverse, or generate a random alphabet containing all 26 unique Latin letters.
