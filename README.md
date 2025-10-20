# Not Enigma

A small Win32 desktop application that demonstrates classical substitution ciphers. The app lets you encrypt or decrypt text using the Caesar, Vigenere, and Atbash ciphers. Choose a cipher, provide the shift (for Caesar) or keyword (for Vigenere) when applicable, pick an alphabet ordering, then click **Encrypt** or **Decrypt** to see the transformed text.

## Building (Windows)

This project uses CMake. From a Visual Studio Developer Command Prompt:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable (`NotEnigma.exe`) will be located in `build/Release/`.

## Notes

- The Caesar cipher accepts positive or negative numeric shifts via the **Shift** field.
- The keyword field is only available when the Vigenere cipher is selected. Characters not present in the active alphabet are ignored, and at least one valid character is required.
- All ciphers use the alphabet field to define their working alphabet. Leave it empty to fall back to the standard English alphabet.
- When Caesar or Atbash is active, use the **Alphabet Preset** drop-down to quickly choose the normal alphabet, its reverse, or generate a random alphabet containing all 26 unique Latin letters.
