# Icon Assets

Place your custom artwork in this directory so the GUI build can load it at runtime:

- `app_icon.ico` – used for the executable and taskbar icon. Provide a Windows `.ico` file that includes at least a 256×256 32‑bit image to ensure it looks crisp on high DPI displays. Smaller variants (128×128, 64×64, 32×32) can be embedded in the same `.ico` file for best results.
- `panel_icon.bmp` – displayed in the top-left corner of the application window. Supply a square bitmap (for example 96×96 or 128×128) using the full Latin alphabet theme you prefer. Any dimensions up to 256×256 are accepted; the image is rendered at its native size inside the layout.

When you build or run the GUI executable, it will look for these files relative to the executable location (e.g., `NotEnigma.exe` next to an `assets` folder). If either file is missing the application falls back to the default system icon or simply hides the image.
