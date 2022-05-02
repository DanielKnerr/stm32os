# Kompilieren
- `make compile`

# Debugging
- in den Ordner `tools` gehen und dort mit `sudo openocd` OpenOCD starten (der Ordner ist wichtig, damit OpenOCD die Konfigurationsdatei findet)

## GDB
- aus dem Hauptverzeichnis mit `tools/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gdb` GDB starten
- in GDB dann mit `target extended-remote *:3333` eine Verbindung zu OpenOCD aufbauen
- mit `load out/os.elf` und `file out/os.elf` die ELF-Datei laden und mit `run` ausführen
- wenn `os.elf` neu kompiliert wurde muss nur mit `load` die Datei aktualisiert werden

## VSCode
- mit VSCode folgende Launch-Konfiguration anlegen
- in der Sidebar "Run and Debug" auswählen und auf Start klicken

```json
{
    "name": "GDB",
    "type": "gdb",
    "request": "launch",
    "cwd": "${workspaceRoot}",
    "target": "${workspaceRoot}/out/os.elf",
    "gdbpath": "<absoluter Pfad zu tools/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gdb>",
    "autorun": [
        "target extended-remote localhost:3333",
        "load ./out/os.elf",
        "symbol-file ./out/os.elf",
    ]
}
```

# Links
- C Runtime: https://microchipdeveloper.com/tls2101:c-runtime-environment