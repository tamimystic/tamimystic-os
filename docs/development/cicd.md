# Continuous Integration and Deployment

Tamimystic OS enforces strict CI/CD pipelines to guarantee software stability. All code pushed to the repository is subject to automated verification.

## GitHub Actions Workflow

The primary pipeline is defined in `.github/workflows/build.yml`.

1.  **Trigger**: The workflow executes on every push to the `main` branch or any Pull Request.
2.  **Environment Provisioning**: A fresh Ubuntu environment is provisioned, and the official `espressif/esp-idf-ci-action` container is initialized.
3.  **Compilation Verification**: The entire operating system is cross-compiled for the ESP32-S3 target. Any compilation errors, syntax faults, or strict warning violations (`-Werror`) will immediately fail the build.
4.  **Artifact Generation**: Upon successful compilation, the workflow extracts the critical `.bin` files (`bootloader.bin`, `partition-table.bin`, `tamimystic_os.bin`) and archives them.
5.  **Distribution**: The generated zip artifact is securely uploaded to the GitHub Actions run page, making it immediately available for end-users to download and deploy.

This automated pipeline ensures that the `main` branch always represents a verified, functional state of the operating system.
