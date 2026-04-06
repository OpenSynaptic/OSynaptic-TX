# Security Policy

## Supported Versions

| Version | Supported          | End of Support |
| ------- | ------------------ | -------------- |
| 1.0.x   | :white_check_mark: | Current        |
| < 1.0.0 | :x:                | Not supported  |

## Security Considerations

### For Embedded Systems Integrators

OSynaptic-TX is a **TX-only encoder** — it does not accept or parse any external input. The attack surface is therefore minimal. However, integrators should be aware of:

- **Value scaling**: the `scaled` parameter is a signed 32-bit integer. Ensure sensor readings are range-checked before passing to `ostx_sensor_pack()` / `ostx_static_pack()` / `ostx_stream_pack()`.
- **Agent ID confidentiality**: the `aid` field is transmitted in plaintext. Do not use cryptographic keys or sensitive identifiers as agent IDs.
- **Transport security**: OSynaptic-TX produces unencrypted frames. If confidentiality is required, encrypt at the transport layer (e.g., TLS over TCP, DTLS over UDP).
- **Replay protection**: frames include a transaction ID (`tid`) and timestamp (`ts`), but the library does not enforce monotonicity at the sender side — that responsibility lies with the receiving server.

### Common Integration Issues

- **Timestamp source**: use a reliable time source for `ts`. Bogus timestamps can confuse the server-side deduplication logic.
- **Buffer sizes**: `OSTX_PACKET_MAX` (default 96) must be large enough for your longest sensor ID + unit + encoded value. Adjust in `ostx_config.h` if needed.
- **Thread safety**: the library has no internal locking. If used from multiple tasks (e.g., FreeRTOS), guard calls with a mutex at the application level.

## Reporting Security Vulnerabilities

**Please do not publicly disclose security vulnerabilities.** Instead:

1. **Open a GitHub Security Advisory** in the repository (preferred), or  
   **email the maintainers** with:
   - Description of the vulnerability
   - Affected components and versions
   - Proof of concept (without exposing a full working exploit)
   - Suggested remediation if available
   - Your name and contact information (for credit)

2. **Response timeline:**
   - Acknowledgment: within 48 hours
   - Initial assessment: within 1 week
   - Fix preparation: within 2 weeks (severity dependent)

3. We will coordinate a disclosure timeline with you and credit responsible reporters in the release notes.

## Security Advisories

Published via:
- GitHub Security Advisories (when a CVE is assigned)
- Release notes and version tags on GitHub

Subscribe to release notifications to stay informed of security updates.
