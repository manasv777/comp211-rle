# References

- [Microsoft Open Specifications: Run-Length Encoding (RLE) Compression](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/b05fc2f2-ad74-41f0-88ee-55ed5c36aad2) describes count/value encoding and expansion by repeating the stored value.
- [Microsoft Learn: Bitmap Compression](https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-compression) documents the BI_RLE8 count/color-index representation and the additional controls used in complete bitmap files.

The byte counts in this project come from the included C program and its saved
results. The codec implements a simple two-byte record format, not a complete
BMP encoder or decoder.
