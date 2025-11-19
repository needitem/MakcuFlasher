from setuptools import setup, find_packages

setup(
    name="makcu_flasher",
    version="1.0.0",
    description="Firmware uploader for ESP32-based Makcu devices",
    author="Makcu Team",
    py_modules=["makcu_flash"],
    install_requires=[
        "esptool>=4.0",
        "pyserial>=3.0",
    ],
    entry_points={
        "console_scripts": [
            "makcu-flash=makcu_flash:main",
        ],
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.6",
)
