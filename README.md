# Not Enigma

A small Win32 desktop application that demonstrates classical substitution ciphers. The app lets you encrypt or decrypt text using the Caesar, Vigenère, and Atbash ciphers. Choose a cipher, enter the message and key (when applicable), pick an alphabet ordering, then click **Encrypt** or **Decrypt** to see the transformed text.

## Building (Windows)

This project uses CMake. From a Visual Studio Developer Command Prompt:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable (`NotEnigma.exe`) will be located in `build/Release/`.

## Notes

- The Caesar cipher accepts positive or negative numeric shifts.
- All ciphers use the alphabet field to define their working alphabet. Leave it empty to fall back to the standard English alphabet.
- Vigenère keys ignore characters that are not present in the active alphabet. At least one valid character is required.
- The Atbash cipher does not require a key; the key field is disabled when selected.
