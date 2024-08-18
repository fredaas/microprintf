DESCRIPTION
-------------------------------------------------------------------------------

Microprintf is a tiny printf implementation for embedded systems with a very
small memory footprint. Microprintf uses only 256 bytes of internal static
memory by default and can be configured to use even less if need be.

Tested on the following architectures:

- RISCV.
- ARM32.
- ARM64.
- X86_64.

USAGE
-------------------------------------------------------------------------------

Drop printf.c and printf.h into you project and include printf.h. Then define
your custom printf handler, e.g.

```
void printf_handler(char c)
{
    putchar(c);
}
```

FEATURES
-------------------------------------------------------------------------------

Types:

- `%c` Character.
- `%d` Signed decimal integer.
- `%u` Unsigned decimal integer.
- `%s` String of characters.
- `%b` Unsigned binary.
- `%x` Unsigned hex (lowercase).
- `%X` Unsigned hex (uppercase).
- `%p` Unsigned pointer address.

Flags:

- `0` Zero padding
- `-` Left align
- `+` Right align.
