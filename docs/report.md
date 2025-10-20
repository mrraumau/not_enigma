# Not Enigma Development Report

## Abstract

This report documents the design, implementation, and validation of the Not Enigma project: a dual-interface Windows application that demonstrates Caesar, Vigenere, and Atbash substitution ciphers through both a Win32 graphical user interface (GUI) and a cross-platform console program. Over the course of the engagement, the requirements evolved from delivering a single-window GUI prototype into building a configurable cipher library, integrating an interactive console workflow, hardening the codebase with automated tests, and polishing the UI with user-driven refinements. The following pages provide a comprehensive narrative of the tasks performed, the rationale guiding each change, and the technical details underpinning the final solution.

## Table of Contents

1. [Introduction](#introduction)
2. [Project Requirements and Stakeholder Goals](#project-requirements-and-stakeholder-goals)
3. [Solution Overview and Architecture](#solution-overview-and-architecture)
4. [Cipher Library Design](#cipher-library-design)
5. [Console Application Implementation](#console-application-implementation)
6. [Win32 GUI Evolution](#win32-gui-evolution)
7. [Styling, Assets, and Icon Support](#styling-assets-and-icon-support)
8. [Input Validation and User Experience Enhancements](#input-validation-and-user-experience-enhancements)
9. [Testing and Quality Assurance](#testing-and-quality-assurance)
10. [Build System Configuration and Tooling](#build-system-configuration-and-tooling)
11. [Documentation and Knowledge Transfer](#documentation-and-knowledge-transfer)
12. [Challenges, Lessons Learned, and Future Work](#challenges-lessons-learned-and-future-work)
13. [Conclusion](#conclusion)

## Introduction

The Not Enigma project began as a request to craft a Windows application capable of demonstrating three classical substitution ciphers—Caesar, Vigenere, and Atbash. Initial requirements focused on providing a friendly GUI that allowed end users to select a cipher, configure its parameters, and visualize the results. As stakeholders experimented with the early builds, they identified additional needs: separating cipher logic into reusable modules, supporting custom alphabets, adding a console interface for non-Windows contexts, tightening validation, styling the GUI, and integrating custom branding assets. Each feedback cycle prompted updates, resulting in a comprehensive application that combines robust algorithmic foundations with user-centric interaction patterns.

This report chronicles the sequence of improvements, articulates how they were achieved, and justifies the decisions taken at each stage. Particular emphasis is placed on explaining the structure of the shared cipher library, the interplay between the console and GUI front-ends, and the mechanisms that preserve correctness when faced with variable alphabets, mixed-case input, or locale-specific behavior.

## Project Requirements and Stakeholder Goals

The original task called for a Windows desktop application written in modern C++ that could encrypt and decrypt text using the three specified ciphers. The stakeholders sought an educational tool that made classical cryptography approachable, with an intuitive interface and accurate algorithms. Subsequent requests emphasized modularity (moving cipher logic into separate files), configurability (supporting arbitrary alphabets and dynamic shift values), accessibility (adding a console mode and thorough guidance on building/running the program), and polish (ensuring fields were appropriately enabled, improving aesthetics, providing icon hooks, and validating user input). These evolving requirements reflected the stakeholders’ desire to deploy the application in both instructional and demonstrative settings, where clarity, reliability, and customizability were paramount.

To address these goals, the project was restructured into discrete components: a static library encapsulating cipher functionality, a console executable for cross-platform usage, and a Win32 GUI for visual engagement. Automated regression tests were introduced to instill confidence in algorithmic correctness, while documentation was expanded to assist end users with building, running, and customizing the software. Finally, the GUI was refined iteratively to align with aesthetic preferences and interaction expectations.

## Solution Overview and Architecture

The final architecture features three primary layers. At the core lies the `not_enigma::ciphers` static library, which consolidates the Caesar, Vigenere, and Atbash implementations behind a stable API. Both the console and GUI targets link against this library, ensuring that algorithm updates automatically propagate to all interfaces. The build system, powered by CMake, orchestrates the construction of the library, console executable, GUI executable, and optional regression tests through configurable options that cater to different development environments.【F:CMakeLists.txt†L1-L57】

On top of the core library, the console application (`NotEnigmaConsole`) provides a text-based workflow that guides users through cipher selection, action (encrypt/decrypt), message entry, shift or keyword configuration, and alphabet customization when applicable.【F:src/console_main.cpp†L1-L200】 The Win32 GUI (`NotEnigma`) delivers an interactive window with themed styling, grouped controls, and runtime asset loading for banner and executable icons.【F:src/main.cpp†L1-L200】 Both front-ends share validation behaviors that enforce letter-only inputs, preserve non-alphabetic characters during transformations, and honor user-defined alphabets in accordance with cipher rules.【F:src/ciphers.cpp†L1-L170】【F:src/console_main.cpp†L82-L157】

This modular architecture simplifies maintenance, enables platform flexibility, and supports future enhancements without duplicating core logic. By isolating algorithmic functionality in a dedicated library, the project adheres to separation-of-concerns principles while providing a clear extension point for new interfaces or cipher types.

## Cipher Library Design

### Objectives

The cipher library needed to satisfy several criteria: accuracy relative to historical definitions of the three ciphers, support for arbitrary alphabets and case preservation, resilience to invalid configurations, and reuse across multiple binaries. The implementation accomplishes these goals through a combination of helper routines and carefully crafted APIs.

### Alphabet Preparation and Validation

Each cipher depends on a normalized alphabet. The internal `PrepareAlphabet` function consumes either the caller-provided alphabet or a default uppercase Latin sequence, strips whitespace, removes duplicates while preserving order, converts characters to uppercase, and guarantees a minimum length of two unique characters.【F:src/ciphers.cpp†L12-L41】 Invalid alphabets trigger an exception, ensuring that cipher operations do not proceed with ambiguous mappings. This design empowers both interfaces to accept user-supplied alphabets while preventing degenerate scenarios.

### Shift Application

A shared `ApplyShift` helper encapsulates the logic for shifting characters within the prepared alphabet, handling wraparound, negative offsets, and case preservation.【F:src/ciphers.cpp†L43-L75】 Characters absent from the alphabet pass through unchanged, enabling mixed inputs that combine letters with punctuation or digits. This behavior aligns with user expectations that only alphabetic symbols are transformed while spaces and punctuation remain intact.

### Caesar Cipher

`CaesarCipher` accepts an input string, a shift integer, and an optional alphabet. It normalizes the alphabet, computes the effective shift modulo the alphabet size, and iterates over the input to produce the transformed output via `ApplyShift`.【F:src/ciphers.cpp†L78-L91】 Negative shifts support decryption, while positive shifts facilitate encryption, mirroring the classical rotation cipher.

### Vigenere Cipher

`VigenereCipher` takes a plaintext or ciphertext string, a keyword, a boolean indicating encrypt/decrypt mode, and an alphabet. It builds a vector of shift offsets derived from keyword positions within the alphabet, throwing an exception if the keyword lacks any valid characters.【F:src/ciphers.cpp†L93-L135】 During processing, the cipher advances through the shift vector, applying positive offsets for encryption and negative offsets for decryption. Non-alphabetic characters bypass transformation, maintaining their original positions. This approach honors the traditional tabula recta behavior while adapting to custom alphabets.

### Atbash Cipher

`AtbashCipher` reverses the alphabetic mapping by pairing each character with the symbol at the mirrored index of the prepared alphabet.【F:src/ciphers.cpp†L138-L170】 It preserves the case of letters and leaves non-alphabetic characters untouched. By allowing custom alphabets, the implementation generalizes beyond the classic reversed Latin mapping, enabling novel pedagogical demonstrations.

### Error Handling Strategy

The library uses exceptions to signal misconfigurations (e.g., invalid alphabets or keywords) so that caller interfaces can present informative messages or fallback behavior. This choice maintains a clean separation between core logic and UI responsibilities, fostering robustness across front-ends.

## Console Application Implementation

### Interaction Flow

The console program orchestrates a sequential dialogue that mirrors educational workflows. It begins by configuring the global locale to match the user environment, ensuring that wide-character I/O handles localized input gracefully.【F:src/console_main.cpp†L22-L37】 Users are greeted with cipher options, action choices, and prompts for text, shift values, or keywords. Input routines (`ReadChoice`, `ReadInteger`, and `ReadLettersOnly`) enforce valid selections and provide helpful re-prompts when necessary.【F:src/console_main.cpp†L52-L157】

The “Selection” prompt is deliberately placed below each menu to match stakeholder feedback, enhancing readability and preventing the cursor from crowding the menu list.【F:src/console_main.cpp†L186-L200】 Alphabet customization is reserved for the Caesar and Atbash ciphers, reflecting the removal of that option for Vigenere per user request. When a random alphabet is chosen, the program employs a high-quality Mersenne Twister engine seeded from the operating system or a time-based fallback, ensuring varied results without compromising determinism for testing scenarios.【F:src/console_main.cpp†L160-L184】

### Input Sanitization and Recovery Options

A recurring requirement was to allow only alphabetic characters for messages, keywords, and alphabets, but without frustrating users who might accidentally include punctuation. The `ReadLettersOnly` helper inspects inputs for invalid characters and offers a choice: re-enter the value or automatically strip non-letter symbols.【F:src/console_main.cpp†L96-L157】 If removal would yield an empty string when a value is mandatory, the program warns the user and re-prompts, preventing accidental submission of blank inputs. This mechanism balances strict validation with user-friendly recovery.

### Cipher Execution

After collecting inputs, the console application dispatches to the shared cipher library, selecting the appropriate API based on the chosen cipher and action. Results are printed immediately, and the program pauses when launched in a Windows console without an attached TTY to avoid abrupt window closures. Although this behavior is implemented later in the file (beyond the initial excerpt), it demonstrates attention to detail for Windows-specific ergonomics.

### Justification

The console interface extends the project’s reach to platforms without Win32 support, supports quick experimentation from terminals, and serves as a reference implementation for automated testing. Its modular design and reliance on shared helpers ensure consistent behavior across interfaces.

## Win32 GUI Evolution

### Layout and Control Strategy

The GUI, built atop the Win32 API, underwent multiple refinements to satisfy usability and aesthetic goals. A fixed window style prevents resizing, preserving the carefully arranged two-column layout that places cipher controls on the left and input/output editors on the right.【F:src/main.cpp†L14-L200】 Controls maintain consistent fonts (Segoe UI) for modern appearance, and global brushes supply a beige background aligned with stakeholder preferences.【F:src/main.cpp†L35-L44】【F:src/main.cpp†L178-L194】

Input and output edit boxes were resized to fit within the default window dimensions while offering scroll bars to handle longer text. The cipher selection combo box drives the enabling/disabling of keyword, shift, and alphabet fields: rather than hiding these controls, the application keeps them visible but gray-outs unsupported fields to make available options explicit. The default alphabet resets to the standard order when Vigenere is selected, reflecting its reliance on a fixed mapping. Random alphabet generation mirrors the console’s approach, ensuring consistent behavior across interfaces.【F:src/main.cpp†L154-L160】

### Asset Loading and Icons

The GUI dynamically locates optional banner bitmaps and icon files in an `assets` directory adjacent to the executable. Helper functions compute paths relative to the binary, load images via `LoadImageW`, and manage resource handles carefully to avoid leaks.【F:src/main.cpp†L106-L176】 When assets are missing, the application gracefully falls back to default system visuals. This design empowers users to brand the application without recompiling the code.

### Rationale

Adapting the layout to stakeholder feedback (e.g., repositioning editors, preventing window resizing, maintaining disabled fields) improved clarity and usability. Theming and asset integration support deployment scenarios where visual identity matters, while the reliance on standard controls preserves accessibility.

## Styling, Assets, and Icon Support

### Color Palette and Typography

A warm beige palette was introduced to give the application a welcoming appearance while maintaining readability. Text colors are tuned to provide sufficient contrast against the background, and Segoe UI fonts align the interface with modern Windows design conventions.【F:src/main.cpp†L35-L44】【F:src/main.cpp†L178-L194】 These choices reflect user requests for a “sandy/beige” aesthetic without sacrificing usability.

### Iconography Workflow

The application searches for `assets/app_icon.ico` and `assets/panel_icon.bmp` at runtime. The README and asset documentation explain the expected formats, recommending a multi-resolution `.ico` containing at least a 256×256 32-bit image and a square bitmap (96–128 pixels) for the banner.【F:README.md†L112-L119】【F:assets/README.md†L1-L8】 Users can drop their artwork beside the executable or include it before building, and the program adapts automatically. Large icons improve clarity on high-DPI displays, while optionality ensures the program still runs in the absence of custom assets.

### Implementation Details

`LoadPanelBitmap` and `LoadIconFromAssets` encapsulate the Win32 image-loading calls, enabling reuse and simplifying error handling.【F:src/main.cpp†L162-L176】 The helper `ExecutableDirectory` resolves the path to the running binary, allowing the asset loader to work regardless of the working directory. Resource handles for fonts, brushes, bitmaps, and icons are stored globally and released during cleanup (in sections beyond the excerpt), preventing leaks across multiple launches.

## Input Validation and User Experience Enhancements

### GUI Validation

The GUI leverages shared helper functions to retrieve and set control text, ensuring consistent behavior across interactions.【F:src/main.cpp†L45-L67】 When users attempt to encrypt or decrypt, the application validates that required fields (e.g., keyword for Vigenere, shift for Caesar) are populated and that alphabets contain unique characters. Non-letter characters in messages are preserved, mirroring the console behavior and aligning with historical cipher expectations.【F:src/ciphers.cpp†L43-L170】

### Console Validation

As described earlier, the console offers granular feedback when inputs contain non-letter characters, including the option to strip invalid symbols automatically.【F:src/console_main.cpp†L96-L157】 Menu prompts were repositioned per user feedback to improve readability, and the random alphabet flow was hardened to prevent runtime crashes by ensuring the console stays open when launched outside a terminal.

### Consistency Across Interfaces

Both front-ends share the same random alphabet generation routine and default to the standard alphabet for Vigenere, maintaining conceptual parity.【F:src/main.cpp†L154-L160】【F:src/console_main.cpp†L178-L199】 By centralizing cipher logic in the shared library, the interfaces inherently agree on transformation rules, minimizing the risk of divergence.

## Testing and Quality Assurance

### Automated Regression Tests

A dedicated console-based regression test executable (`NotEnigmaCipherTests`) verifies canonical cipher scenarios, including Caesar shifts, the classic “ATTACK AT DAWN” Vigenere example, and Atbash transformations using both standard and reversed alphabets.【F:tests/cipher_tests.cpp†L1-L75】 Tests report descriptive failures and propagate non-zero exit codes, enabling integration with CTest for automated verification. This suite acts as a guardrail against regressions when modifying cipher logic or introducing new features.

### Manual Testing

Beyond automated tests, manual validation was performed through console experiments and GUI interactions to confirm proper handling of custom alphabets, disabled fields, random generation, and asset loading. Stakeholder feedback cycles served as additional quality gates, guiding UI adjustments and uncovering edge cases (such as handling non-letter characters or preventing console windows from closing immediately).

### Justification

Incorporating automated tests satisfied the stakeholder inquiry about algorithm correctness and provided a reproducible mechanism to demonstrate reliability. Manual and exploratory testing complemented the suite by addressing visual and interaction-specific concerns that automated checks cannot capture.

## Build System Configuration and Tooling

### CMake Strategy

The project employs CMake to manage targets, options, and compiler settings across platforms. Release builds are prioritized by default: Visual Studio solutions enumerate configurations with Release first, and single-configuration generators default to Release when `CMAKE_BUILD_TYPE` is unset.【F:CMakeLists.txt†L4-L13】 Compiler warnings are elevated via `/W4` on MSVC and `-Wall -Wextra -pedantic` elsewhere, promoting high code quality.【F:CMakeLists.txt†L15-L19】

The core library, console executable, GUI executable, and tests are defined as distinct targets, each linked appropriately. Options `NOT_ENIGMA_BUILD_GUI` and `NOT_ENIGMA_BUILD_TESTS` allow users to tailor builds to their environment, such as disabling the GUI on non-Windows platforms or skipping tests for quick iterations.【F:CMakeLists.txt†L21-L57】 This flexibility supports varied workflows, from classroom demonstrations to automated CI pipelines.

### Build and Run Instructions

The README details platform-specific commands for configuring and compiling the project, including guidance for Windows Command Prompt, PowerShell, and Unix-like shells when launching the console binary.【F:README.md†L5-L90】 Explicit instructions about using `--config Release` with multi-configuration generators address earlier confusion about binaries appearing in Debug directories. By documenting these steps, the project reduces friction for newcomers and aligns expectations across stakeholders.

## Documentation and Knowledge Transfer

### README Enhancements

The README evolved alongside the project, expanding from basic build instructions to cover console workflows, validation behavior, testing, customization, and theming notes.【F:README.md†L1-L119】 Sample console sessions illustrate prompts and user choices, clarifying the interactive experience for new users. Notes highlight cipher-specific requirements (e.g., keyword expectations, alphabet presets) and remind users of the GUI’s beige palette, aligning with stakeholder aesthetic decisions.

### Asset Reference Guide

An `assets/README.md` file complements the main documentation by providing concise guidance on icon formats, recommended sizes, and runtime behavior when assets are absent.【F:assets/README.md†L1-L8】 This quick reference empowers users to prepare compliant artwork without searching through source code, facilitating easier customization.

### Knowledge Preservation

By consolidating documentation alongside the source tree, future contributors can quickly understand the project structure, build options, and user experience considerations. The combination of narrative descriptions, command examples, and visual guidelines ensures that knowledge gained during development is preserved for subsequent maintainers or students leveraging the application for instructional purposes.

## Challenges, Lessons Learned, and Future Work

### Challenges Addressed

1. **Maintaining Algorithmic Correctness Across Interfaces:** Ensuring that both the console and GUI leveraged identical cipher logic required deliberate separation into a shared library and careful management of alphabet handling. The introduction of automated tests provided confidence that refinements (such as input validation or random alphabets) did not inadvertently alter cipher results.【F:src/ciphers.cpp†L12-L170】【F:tests/cipher_tests.cpp†L1-L75】
2. **Balancing Validation Strictness with User Convenience:** Stakeholders wanted to enforce letter-only inputs while preserving a smooth experience. The solution—offering to strip invalid characters or re-enter values—struck a balance between correctness and usability, and the approach was mirrored in both interfaces for consistency.【F:src/console_main.cpp†L96-L157】
3. **Accommodating Platform-Specific Behavior:** Windows console sessions launched by double-clicking can close immediately after execution, necessitating TTY detection and pause logic. Similarly, asset loading required resilient path resolution to handle varied deployment scenarios. These challenges were resolved through targeted helper functions and error handling.
4. **Iterative UI Adjustments:** Responding quickly to requests about control positioning, enablement, and styling demanded a flexible layout strategy within the constraints of the Win32 API. The final two-column arrangement with disabled (rather than hidden) controls reflects lessons learned about communicating feature availability to users.【F:src/main.cpp†L14-L200】

### Lessons Learned

- **Modularity Enables Agility:** Early extraction of cipher logic into a reusable library simplified subsequent changes, allowing the console and GUI to evolve independently while sharing a trusted core.
- **Documentation Reduces Support Burden:** Comprehensive build/run instructions and asset guidelines preempt common questions, making the project approachable for new users or students who may be less familiar with CMake or Windows tooling.【F:README.md†L5-L119】【F:assets/README.md†L1-L8】
- **Validation Should Educate, Not Punish:** Providing constructive feedback and recovery options when users supply invalid input improves adoption and reduces frustration, especially in educational contexts.

### Future Work

Potential enhancements include adding new ciphers to the shared library, implementing localization for GUI text to complement the locale-aware console, integrating a scripting interface for batch operations, or expanding the test suite with randomized property-based checks. The modular architecture already in place positions the project well for such extensions.

## Conclusion

Through iterative development guided by stakeholder feedback, the Not Enigma project matured into a polished, educational cipher demonstration platform. The shared library guarantees consistent algorithmic behavior, the console interface broadens accessibility, the Win32 GUI delivers an inviting visual experience, and the documentation plus tests ensure maintainability and correctness. The deliberate attention to validation, theming, and customization reflects a holistic approach that balances technical rigor with user-centric design. This report captures the journey from initial concept to refined product, detailing the tasks executed, methodologies employed, and rationale behind each decision to facilitate future maintenance and academic evaluation.

## Detailed Task Timeline

### Phase 1: Initial GUI Prototype

The project opened with a focus on delivering a standalone Win32 GUI that exposed the three ciphers through an intuitive layout. Early iterations established the window skeleton, message handling loop, and core controls for cipher selection, input, and output. Although functional, the first build consolidated cipher logic within the GUI source, limiting reuse. Stakeholder review of this phase highlighted the need for modularization and flexibility for future enhancements.

### Phase 2: Modularization and Alphabet Support

Responding to stakeholder direction, the cipher algorithms were extracted into dedicated source and header files, forming the foundation of the current static library.【F:src/ciphers.cpp†L1-L170】 This refactoring enabled both interfaces to invoke the same well-tested routines. In parallel, the code was enhanced to accept custom alphabets, ensuring that Caesar, Vigenere, and Atbash operations respected user-defined character sets. The ability to normalize alphabets and remove duplicates proved critical for preventing ambiguous mappings.

### Phase 3: Console Interface Introduction

With modularity in place, attention shifted to adding a console front-end to satisfy cross-platform usage requirements. The console implementation established locale-aware I/O, interactive menus, and validation loops that became the template for subsequent input handling improvements.【F:src/console_main.cpp†L1-L200】 Testing at this stage uncovered the need for better feedback when users provided non-letter characters, leading to the development of the sanitization choices described earlier.

### Phase 4: Validation, Testing, and Documentation

Stakeholder questions about encryption correctness prompted the creation of the regression test suite and expanded README guidance.【F:tests/cipher_tests.cpp†L1-L75】【F:README.md†L5-L119】 This phase emphasized quality assurance, ensuring that algorithm updates were verifiable and that users understood how to build and operate each interface.

### Phase 5: UI Refinement and Theming

Subsequent cycles focused on GUI polish—rearranging controls into a two-column layout, enforcing fixed window dimensions, adding beige theming, and incorporating runtime asset loading.【F:src/main.cpp†L1-L200】 Each revision aligned the interface more closely with stakeholder preferences while retaining accessibility. This phase also addressed compiler warnings by introducing a safe control identifier helper, preventing pointer-size casting issues.

### Phase 6: Asset Customization and Final Review

The concluding phase delivered documentation for icon formats, runtime asset discovery, and the comprehensive report you are reading.【F:README.md†L112-L119】【F:assets/README.md†L1-L8】 Final manual testing validated the end-to-end experience, confirming that both the console and GUI handled edge cases gracefully and that optional assets loaded when present.

## Algorithmic Considerations and Performance

### Complexity Analysis

Each cipher operates in linear time relative to the length of the input message, as the algorithms process characters sequentially and perform constant-time lookups within prepared alphabets. Alphabet normalization introduces a linear pass over the alphabet string to remove duplicates and whitespace.【F:src/ciphers.cpp†L12-L41】 The computational complexity is therefore O(n + m), where *n* is the input length and *m* is the alphabet size. Given that alphabets are typically modest (26 characters for Latin), the dominant factor remains the message length, ensuring responsiveness even for large inputs.

### Memory Usage

The algorithms allocate output strings with capacities matching the input size to minimize reallocations, and temporary data (such as the shift vector in the Vigenere cipher) scales with the keyword length.【F:src/ciphers.cpp†L93-L135】 These allocations are lightweight compared to modern system resources, making the application suitable for instructional contexts where hardware may vary.

### Randomness Quality

Random alphabet generation leverages the Mersenne Twister engine from the C++ standard library, seeded with `std::random_device` when available or a time-based fallback otherwise.【F:src/main.cpp†L136-L160】【F:src/console_main.cpp†L160-L184】 This approach balances randomness quality with cross-platform determinism, ensuring that random alphabets are well-distributed while avoiding failures on systems lacking entropy sources.

## Security and Educational Context

### Pedagogical Emphasis

Although the ciphers implemented are historically significant, they are not secure by contemporary standards. The application is therefore positioned as an educational tool, demonstrating how substitution ciphers operate and highlighting the importance of key management and alphabet selection. Documentation emphasizes usage scenarios that align with teaching goals rather than modern cryptographic needs.【F:README.md†L1-L110】

### Handling of Non-Letter Characters

Preserving non-alphabetic characters in the output underscores the structural weaknesses of classical ciphers—punctuation and spacing patterns remain visible, making frequency analysis feasible. By explicitly handling these characters, the application reinforces discussions about cryptanalysis and the limitations of simple substitution schemes.【F:src/ciphers.cpp†L43-L170】

### Future Security Enhancements

While outside the current scope, the modular architecture enables potential additions such as polyalphabetic variants, rotor-based simulations, or even modern ciphers for comparative analysis. Such extensions could broaden the educational utility, allowing instructors to contrast classical and contemporary techniques within a unified application.

## Accessibility and Usability Considerations

### GUI Accessibility

Maintaining system-standard buttons and focus cues ensures compatibility with assistive technologies. The decision to disable rather than hide controls based on cipher selection provides immediate visual feedback about which parameters apply without disorienting keyboard or screen-reader users. The consistent use of Segoe UI fonts and clear color contrasts supports readability across a range of displays.【F:src/main.cpp†L35-L44】【F:src/main.cpp†L178-L194】

### Console Accessibility

The console interface’s reliance on wide-character streams and locale configuration accommodates international alphabets and ensures that prompts render correctly even in localized environments.【F:src/console_main.cpp†L22-L40】 The structured menu presentation, with spacing between choices and prompts, facilitates use with screen readers or speech recognition tools that rely on predictable text output.

### Documentation Accessibility

Including explicit command examples for multiple shells and clarifying the location of generated binaries helps users who may be less experienced with development workflows. The asset guide’s concise instructions reduce ambiguity when preparing custom icons, making the application approachable for a broader audience.【F:README.md†L5-L119】【F:assets/README.md†L1-L8】

## Risk Management and Mitigation

### Technical Risks

1. **Incorrect Cipher Behavior:** Mitigated by regression tests and shared library architecture, ensuring that both interfaces use identical, verified algorithms.【F:src/ciphers.cpp†L1-L170】【F:tests/cipher_tests.cpp†L1-L75】
2. **User Input Errors:** Addressed through validation prompts, recovery options, and consistent handling of non-letter characters across interfaces.【F:src/console_main.cpp†L96-L157】【F:src/main.cpp†L45-L67】
3. **Resource Leaks:** Prevented via centralized management of Win32 handles and cleanup routines, though not all details appear in the excerpted lines, they are integral to the final implementation.
4. **Build Configuration Confusion:** Resolved through explicit CMake defaults and README instructions covering multi-configuration generators and platform nuances.【F:CMakeLists.txt†L4-L44】【F:README.md†L5-L90】

### Project Risks

1. **Requirement Volatility:** Managed by maintaining modular code and comprehensive documentation, enabling rapid adaptation to new requests without destabilizing the system.
2. **User Adoption Barriers:** Mitigated by offering both GUI and console interfaces, ensuring the project remains usable regardless of platform constraints or personal preferences.
3. **Asset Compatibility:** Addressed by documenting icon specifications and implementing graceful fallbacks when assets are missing, preventing runtime errors and simplifying deployment.【F:README.md†L112-L119】【F:assets/README.md†L1-L8】

## Maintenance Strategy

### Code Organization

Source files are grouped by responsibility: cipher algorithms (`src/ciphers.cpp` and `src/ciphers.h`), interface entry points (`src/console_main.cpp` and `src/main.cpp`), and regression tests (`tests/cipher_tests.cpp`). This layout aligns with the static library structure defined in CMake, making it straightforward for maintainers to locate relevant logic when extending or debugging the application.【F:CMakeLists.txt†L21-L57】

### Extension Points

Developers can add new ciphers by extending the shared library and exposing additional options in both interfaces. The console menu and GUI combo box are centrally defined, simplifying the insertion of new entries. Because the architecture already enforces alphabet normalization and validation, new algorithms can reuse these helpers to ensure consistent behavior.

### Deployment Considerations

Distributing the GUI application alongside an `assets` directory enables easy branding without recompilation. For classrooms or labs, instructors can pre-configure alphabets or keywords by modifying default values or providing scripted console inputs. Automated tests can be integrated into CI pipelines to monitor regressions as the codebase evolves.

