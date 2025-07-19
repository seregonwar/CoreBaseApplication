# {{project_name}}

{{description}}

## Requisiti

- Compilatore C++ moderno (C++17 o superiore)
- CMake 3.15 o superiore
- Git

## Struttura del Progetto

```
{{project_name}}/
├── src/                    # Codice sorgente
├── include/                # Header files
├── tests/                  # Test unitari
├── docs/                   # Documentazione
├── CMakeLists.txt         # File di configurazione CMake
└── README.md              # Questo file
```

## Compilazione

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Test

```bash
cd build
ctest
```

## Licenza

MIT License

## Autore

{{author}} 