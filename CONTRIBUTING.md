# Contributing

Thanks for your interest in contributing to this operating system project!

This project is a small 32-bit x86 operating system being developed from scratch. Contributions are welcome, especially improvements to the kernel, memory management, processes, drivers, system calls, and development tooling.

## Getting Started

1. Fork the repository.
2. Clone your fork locally.
3. Create a new branch for your changes.
4. Build the operating system before making changes to verify that your environment works.
5. Make your changes.
6. Build and test the OS.
7. Commit your changes with a clear commit message.
8. Open a pull request.

## Development Environment

The project currently targets:

* 32-bit x86
* GCC with `-m32`
* GNU `ld`
* GNU assembler
* QEMU or another x86 emulator
* Make

The kernel is freestanding and does not rely on a host operating system's C runtime.

## Building

Build the project with:

```bash
make
```

If the build succeeds, run the generated kernel using the project's normal emulator/run command.

Do not commit generated files from the `build/` directory unless the project explicitly requires them.

## Making Changes

Keep changes focused.

For example:

* A keyboard-driver change should not also rewrite the process scheduler.
* A paging change should include appropriate paging tests.
* A syscall change should update the corresponding user/kernel interfaces.
* A driver change should remain isolated from unrelated kernel components.

Avoid large rewrites unless they have been discussed beforehand.

## Kernel Safety

This is kernel code, so small mistakes can cause crashes, page faults, general protection faults, triple faults, or boot loops.

When modifying low-level code:

* Be careful with pointers and physical addresses.
* Check page alignment where required.
* Check privilege levels when working with Ring 0 and Ring 3.
* Be careful when modifying the GDT or IDT.
* Preserve registers correctly in assembly.
* Make sure interrupt handlers correctly acknowledge the PIC.
* Do not enable interrupts before the interrupt infrastructure is ready.
* Avoid using uninitialized memory.
* Avoid silently ignoring allocation failures.

If a change affects interrupts, paging, processes, or privilege transitions, test it independently before combining it with other major changes.

## Assembly

Assembly changes should be kept as small and readable as possible.

When adding an interrupt or syscall entry point, document:

* Which registers are pushed.
* What the stack layout is.
* Which registers are modified.
* What arguments are passed to C.
* How the return path restores the CPU state.

Be especially careful with `iret`, privilege transitions, and interrupt stack frames.

## System Calls

System calls are part of the user/kernel boundary.

When adding or modifying a syscall:

1. Define the syscall number.
2. Define its arguments and return value.
3. Update the kernel-side handler.
4. Update the user-side interface if necessary.
5. Test it from Ring 3.
6. Verify invalid arguments do not crash the kernel.

Do not trust pointers supplied by user processes.

## Memory Management

Changes to paging or physical memory management should clearly distinguish between:

* Physical addresses
* Virtual addresses
* Page-directory addresses
* Page-table addresses

Page mappings should use the correct privilege and permission bits.

Changes involving user pages should be tested from Ring 3.

## Processes

Process-related changes should not assume that only one process exists.

Keep process state explicit and avoid relying on global state when process-specific state is required.

When modifying process creation, scheduling, or termination, test:

* Process creation
* Process execution
* System calls
* Process exit
* Returning to the kernel
* Multiple processes when supported

## Drivers

Drivers should avoid unnecessarily depending on unrelated kernel subsystems.

Hardware access should be isolated where practical.

For example, port I/O should remain in appropriate low-level driver or I/O code rather than being scattered throughout unrelated kernel code.

## Testing

At minimum, verify that:

```text
make
```

completes successfully.

For kernel changes, also boot the OS and verify the affected functionality.

Useful tests include:

* Kernel boot
* GDT initialization
* IDT initialization
* Interrupt handling
* Timer interrupts
* Keyboard input
* Paging
* Page faults
* Ring 3 execution
* System calls
* Process creation
* Process termination

If your change causes a previously working feature to stop working, investigate it before opening a pull request.

## Debugging

When reporting a kernel bug, include:

* What you expected to happen
* What actually happened
* The last output printed before the failure
* Any exception number
* Page-fault error code, if applicable
* Relevant register values, if available
* The commit or branch being tested
* The emulator being used

For example:

```text
Expected:
Process exits normally.

Actual:
Page fault.

Error code:
0x00000002

Last output:
Running PID 2
```

This makes low-level bugs significantly easier to reproduce.

## Code Style

Follow the existing style of the surrounding code.

Prefer:

* Clear names
* Small functions
* Explicit types
* Minimal global state
* Comments explaining why something unusual is necessary

Avoid:

* Unnecessary abstractions
* Huge functions
* Copying large amounts of unrelated code
* Magic values without explanation
* Debug prints left in production paths

For hardware and architecture-specific constants, prefer named constants when practical.

## Commit Messages

Write concise commit messages describing the change.

Good:

```text
Add PIT timer initialization
Fix Ring 3 syscall stack frame
Add process termination
Fix page directory user permissions
```

Avoid vague messages such as:

```text
stuff
fix
update
changes
```

## Pull Requests

Pull requests should include:

* A clear description of the change
* Why the change is needed
* How it was tested
* Any known limitations
* Any relevant screenshots or emulator output

If the change is unfinished, clearly state what remains to be implemented.

Keep pull requests focused and reasonably sized.

## Issues

Before opening an issue, check whether the problem has already been reported.

Bug reports should include enough information to reproduce the problem.

Feature requests should explain the intended behavior rather than only describing an implementation.

## Generated Files

Do not commit build artifacts, object files, or generated binaries unless the repository specifically requires them.

Typical generated files include:

```text
build/*.o
build/*.bin
```

Check the repository's `.gitignore` before committing.

## Security

If you discover a security issue that could affect the host system, emulator, build environment, or project infrastructure, do not immediately publish exploit details in a public issue.

Contact the project maintainers privately first.

## License

By contributing to this project, you agree that your contributions may be distributed under the project's existing license.

If you are unsure about the licensing requirements for a contribution, ask before submitting the pull request.

## Final Checklist

Before submitting a pull request:

* [ ] The project builds successfully.
* [ ] The OS boots successfully.
* [ ] The affected functionality was tested.
* [ ] No unrelated generated files were added.
* [ ] Debug-only code was removed unless intentionally retained.
* [ ] Assembly changes preserve the expected CPU state.
* [ ] Interrupt and privilege-level changes were tested carefully.
* [ ] Commit messages clearly describe the changes.
* [ ] The pull request explains what was changed and how it was tested.

Thanks for contributing!
